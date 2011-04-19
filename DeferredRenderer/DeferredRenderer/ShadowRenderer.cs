using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class ShadowRenderer : IHasContent
    {
        private const int SHADOWMAP_SIZE_MIN = 2;
        private const int SHADOWMAP_SIZE_MAX = 1024;

        private int _shadowMapSize;
        public int ShadowMapSize
        {
            get { return _shadowMapSize; }
            set 
            {
                if (value < SHADOWMAP_SIZE_MIN || value > SHADOWMAP_SIZE_MAX || (value & (value - 1)) != 0)
                {
                    throw new ArgumentException("Shadow map size must be between " + SHADOWMAP_SIZE_MIN +
                        " and " + SHADOWMAP_SIZE_MAX + " and be a power of 2.");
                }

                if (_shadowMapSize != value)
                {
                    _shadowMapSize = value;
                    buildDepthRT(_gd);
                }
            }
        }

        private const int CASCADE_COUNT_MIN = 1;
        private const int CASCADE_COUNT_MAX = 8;

        private int _cascadeCount;
        public int CascadeCount
        {
            get { return _cascadeCount; }
            set
            {
                if (value < CASCADE_COUNT_MIN || value > CASCADE_COUNT_MAX)
                {
                    throw new ArgumentException("Cascade count must be between " + CASCADE_COUNT_MIN + 
                        " and " + CASCADE_COUNT_MAX + ".");
                }

                if (_cascadeCount != value)
                {
                    _cascadeCount = value;
                    buildDepthRT(_gd);
                }
            }
        }

        private RenderTarget2D _depthRT;

        private Effect _depthEffect;

        private GraphicsDevice _gd;

        public ShadowRenderer(GraphicsDevice gd)
        {
            _gd = gd;
            _shadowMapSize = SHADOWMAP_SIZE_MAX;
            _cascadeCount = (CASCADE_COUNT_MIN + CASCADE_COUNT_MAX) / 2;
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            buildDepthRT(gd);

            _depthEffect = cm.Load<Effect>("DepthMapEffect");
        }

        private void buildDepthRT(GraphicsDevice gd)
        {
            _depthRT = new RenderTarget2D(gd, _shadowMapSize * _cascadeCount, _shadowMapSize, false, SurfaceFormat.Vector2,
                DepthFormat.Depth24Stencil8, 0, RenderTargetUsage.DiscardContents);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _depthRT.Dispose();
            _depthRT = null;

            _depthEffect.Dispose();
            _depthEffect = null;
        }

        public void Render(IList<ModelInstance> models, DirectionLight light, Camera camera)
        {
            float near = camera.NearClip;
            float far = camera.FarClip;

            float[] splitDepths = new float[_cascadeCount + 1];

            splitDepths[0] = near;
            splitDepths[CascadeCount] = far;
            float fCascadeCount = _cascadeCount;
            const float splitConstant = 0.95f;
            for (int i = 1; i < splitDepths.Length - 1; i++)
            {
                splitDepths[i] = splitConstant * near * (float)Math.Pow(far / near, i / fCascadeCount) +
                    (1.0f - splitConstant) * ((near + (i / fCascadeCount)) * (far - near));
            }

            _gd.SetRenderTarget(null);
            _gd.Clear(ClearOptions.Target, Color.White, 1.0f, 0);
            _gd.Clear(ClearOptions.DepthBuffer, Color.Black, 1.0f, 0); 

            for (int i = 0; i < CascadeCount; i++)
            {
                float minZ = splitDepths[i];
                float maxZ = splitDepths[i + 1];

                Matrix lightView, lightProj;
                createLightViewProjectionMatrix(light, camera, minZ, maxZ, out lightView, out lightProj);

                renderShadowMap(models, i, lightView, lightProj);
            }
        }
   
        private void renderShadowMap(IList<ModelInstance> models, int splitIdx, Matrix lightView, Matrix lightProj)
        {
            // Set the viewport for the current split            
            Viewport splitViewport = new Viewport();
            splitViewport.MinDepth = 0;
            splitViewport.MaxDepth = 1;
            splitViewport.Width = _shadowMapSize;
            splitViewport.Height = _shadowMapSize;
            splitViewport.X = splitIdx * _shadowMapSize;
            splitViewport.Y = 0;

            _gd.Viewport = splitViewport;

            _depthEffect.Parameters["View"].SetValue(lightView);
            _depthEffect.Parameters["Projection"].SetValue(lightProj);

            for (int i = 0; i < models.Count; i++)
            {
                Model model = models[i].Model;
                Matrix world = models[i].GetWorldMatrix();

                for (int j = 0; j < model.Meshes.Count; j++)
                {
                    ModelMesh mesh = model.Meshes[j];

                    for (int k = 0; k < mesh.MeshParts.Count; k++)
                    {
                        ModelMeshPart part = mesh.MeshParts[k];
                        _gd.SetVertexBuffer(part.VertexBuffer);
                        _gd.Indices = part.IndexBuffer;
                        
                        _depthEffect.Parameters["World"].SetValue(models[i].GetBoneMatrix(mesh.ParentBone.Index) * world);

                        for (int m = 0; m < _depthEffect.CurrentTechnique.Passes.Count; m++)
                        {
                            _depthEffect.CurrentTechnique.Passes[m].Apply();

                            _gd.DrawIndexedPrimitives(PrimitiveType.TriangleList, part.VertexOffset, 0,
                                                      part.NumVertices, part.StartIndex,
                                                      part.PrimitiveCount);
                        }
                    }
                }
            }
        }

        private void createLightViewProjectionMatrix(DirectionLight light, Camera cam, float minZ, float maxZ,
            out Matrix lightView, out Matrix lightProj)
        {
            // Matrix with that will rotate in points the direction of the light
            Matrix lightRotation = Matrix.CreateLookAt(Vector3.Zero, -light.Direction, Vector3.Up);
            
            // Get the corners of the frustum
            Matrix clipedProjection = cam.BuildProjection(minZ, maxZ);
            Matrix camView = cam.View;

            BoundingFrustum camFrustum = new BoundingFrustum(camView * clipedProjection);
            Vector3[] frustumCorners = camFrustum.GetCorners();            

            // Transform the positions of the corners into the direction of the light
            for (int i = 0; i < frustumCorners.Length; i++)
            {
                frustumCorners[i] = Vector3.Transform(frustumCorners[i], lightRotation);
            }

            // Find the smallest box around the points
            BoundingBox lightBox = BoundingBox.CreateFromPoints(frustumCorners);

            Vector3 boxSize = lightBox.Max - lightBox.Min;
            Vector3 halfBoxSize = boxSize * 0.5f;

            // The position of the light should be in the center of the back
            // pannel of the box. 
            Vector3 lightPosition = lightBox.Min + halfBoxSize;
            lightPosition.Z = lightBox.Min.Z;

            // We need the position back in world coordinates so we transform 
            // the light position by the inverse of the lights rotation
            lightPosition = Vector3.Transform(lightPosition, Matrix.Invert(lightRotation));

            // Create the view matrix for the light
            lightView = Matrix.CreateLookAt(lightPosition, lightPosition - light.Direction, Vector3.Up);

            // Create the projection matrix for the light
            // The projection is orthographic since we are using a directional light
            lightProj = Matrix.CreateOrthographic(boxSize.X, boxSize.Y, -boxSize.Z * 1.5f, boxSize.Z * 1.5f);
        }
    }
}
