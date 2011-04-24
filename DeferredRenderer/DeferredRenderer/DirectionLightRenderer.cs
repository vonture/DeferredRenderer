using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class DirectionLightRenderer : LightRenderer<DirectionLight>
    {
        private Effect _directionLightEffect;
        private FullScreenQuad _fsQuad;

        public DirectionLightRenderer(GraphicsDevice gd) 
            : base(gd)
        {
            _fsQuad = new FullScreenQuad();
        }

        public override void LoadContent(GraphicsDevice gd, ContentManager cm) 
        {
            _directionLightEffect = cm.Load<Effect>("DirectionLight");
            _fsQuad.LoadContent(gd, cm);

            base.LoadContent(gd, cm);
        }

        public override void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _directionLightEffect.Dispose();
            _directionLightEffect = null;

            _fsQuad.UnloadContent(gd, cm);

            base.UnloadContent(gd, cm);
        }

        public override void RenderUnshadowed(IList<DirectionLight> lights, Camera camera,
            GBuffer gBuffer)
        {
            if (lights.Count == 0)
            {
                return;
            }

            BlendState prevBlend = GraphicsDevice.BlendState;
            DepthStencilState prevStencil = GraphicsDevice.DepthStencilState;

            GraphicsDevice.BlendState = LightBlendState;
            GraphicsDevice.DepthStencilState = LightDepthStencilState;

            _directionLightEffect.CurrentTechnique = 
                _directionLightEffect.Techniques["DirectionLightUnshadowed"];

            gBuffer.SetEffectParameters(_directionLightEffect);           

            Matrix invViewProj = Matrix.Invert(camera.View * camera.Projection);
            _directionLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
            _directionLightEffect.Parameters["CameraPosition"].SetValue(camera.Position);
            _directionLightEffect.Parameters["ScreenSpaceOffset"].SetValue(HalfPixelOffset);

            for (int i = 0; i < lights.Count; i++)
            {
                _directionLightEffect.Parameters["LightDirection"].SetValue(lights[i].Direction);
                _directionLightEffect.Parameters["LightColor"].SetValue(lights[i].Color);
                _directionLightEffect.Parameters["LightIntensity"].SetValue(lights[i].Intensity);

                for (int j = 0; j < _directionLightEffect.CurrentTechnique.Passes.Count; j++)
                {
                    _directionLightEffect.CurrentTechnique.Passes[j].Apply();

                    _fsQuad.Draw(GraphicsDevice);
                }
            }

            GraphicsDevice.BlendState = prevBlend;
            GraphicsDevice.DepthStencilState = prevStencil;
        }

        public override void RenderShadowed(IList<DirectionLight> lights, IList<ModelInstance> models,
            Camera camera, GBuffer gBuffer)
        {
            if (lights.Count == 0)
            {
                return;
            }
            
            BlendState prevBlend = GraphicsDevice.BlendState;
            DepthStencilState prevStencil = GraphicsDevice.DepthStencilState;

            RenderTargetBinding[] rtBinds = GraphicsDevice.GetRenderTargets();

            Vector2[][] clipPlanes;
            Matrix[][] lightViewProjs;
            renderShadows(lights, models, camera, out clipPlanes, out lightViewProjs);

            GraphicsDevice.SetRenderTargets(rtBinds);
            GraphicsDevice.Clear(Color.Transparent);

            GraphicsDevice.BlendState = LightBlendState;
            GraphicsDevice.DepthStencilState = LightDepthStencilState;

            _directionLightEffect.CurrentTechnique =
                _directionLightEffect.Techniques["DirectionLightShadowed"];

            gBuffer.SetEffectParameters(_directionLightEffect);

            Matrix invViewProj = Matrix.Invert(camera.View * camera.Projection);
            _directionLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
            _directionLightEffect.Parameters["CameraPosition"].SetValue(camera.Position);
            _directionLightEffect.Parameters["ScreenSpaceOffset"].SetValue(HalfPixelOffset);

            _directionLightEffect.Parameters["NearClip"].SetValue(camera.NearClip);
            _directionLightEffect.Parameters["FarClip"].SetValue(camera.FarClip);

            _directionLightEffect.Parameters["SqrtCascadeCount"].SetValue((int)Math.Sqrt(CascadeCount));
            
            for (int i = 0; i < lights.Count && i < ShadowRTCount; i++)
            { 
                _directionLightEffect.Parameters["LightViewProjection"].SetValue(lightViewProjs[i]);
                _directionLightEffect.Parameters["ClipPlanes"].SetValue(clipPlanes[i]);

                _directionLightEffect.Parameters["LightDirection"].SetValue(lights[i].Direction);
                _directionLightEffect.Parameters["LightColor"].SetValue(lights[i].Color);
                _directionLightEffect.Parameters["LightIntensity"].SetValue(lights[i].Intensity);

                _directionLightEffect.Parameters["ShadowMap"].SetValue(GetDepthRT(i));
                _directionLightEffect.Parameters["ShadowMapSize"].SetValue(ShadowMapSize);

                for (int j = 0; j < _directionLightEffect.CurrentTechnique.Passes.Count; j++)
                {
                    _directionLightEffect.CurrentTechnique.Passes[j].Apply();

                    _fsQuad.Draw(GraphicsDevice);
                }
            }

            GraphicsDevice.BlendState = prevBlend;
            GraphicsDevice.DepthStencilState = prevStencil;
        }

        private void renderShadows(IList<DirectionLight> lights, IList<ModelInstance> models,
            Camera camera, out Vector2[][] clipPlanes, out Matrix[][] lightViewProjs)
        {
            clipPlanes = new Vector2[Math.Max(lights.Count, ShadowRTCount)][];
            lightViewProjs = new Matrix[Math.Max(lights.Count, ShadowRTCount)][];

            BlendState prevBlend = GraphicsDevice.BlendState;
            DepthStencilState prevStencil = GraphicsDevice.DepthStencilState;

            GraphicsDevice.BlendState = BlendState.Opaque;
            GraphicsDevice.DepthStencilState = DepthStencilState.Default;

            for (int i = 0; i < lights.Count && i < ShadowRTCount; i++)
            {
                renderShadowDepth(lights[i], models, camera, GetDepthRT(i),
                    out clipPlanes[i], out lightViewProjs[i]);
            }

            GraphicsDevice.BlendState = prevBlend;
            GraphicsDevice.DepthStencilState = prevStencil;
        }

        private void renderShadowDepth(DirectionLight light, IList<ModelInstance> models, Camera camera,
            RenderTarget2D depthRT, out Vector2[] clipPlanes, out Matrix[] lightViewProjs)
        {
            float near = camera.NearClip;
            float far = camera.FarClip;

            float[] splitDepths = getSplitDepths(camera.NearClip, camera.FarClip);

            GraphicsDevice.SetRenderTarget(depthRT);
            GraphicsDevice.Clear(Color.Black);

            clipPlanes = new Vector2[CascadeCount];
            lightViewProjs = new Matrix[CascadeCount];

            for (int i = 0; i < CascadeCount; i++)
            {
                float minZ = splitDepths[i];
                float maxZ = splitDepths[i + 1];
                
                Matrix lightView, lightProj;
                createLightViewProjectionMatrix(light, camera, minZ, maxZ, out lightView, out lightProj);

                renderShadowMap(models, i, lightView, lightProj);

                clipPlanes[i] = new Vector2(minZ, maxZ);
                lightViewProjs[i] = lightView * lightProj;
            }
        }

        private float[] getSplitDepths(float near, float far)
        {
            float[] splitDepths = new float[CascadeCount + 1];

            splitDepths[0] = near;
            splitDepths[CascadeCount] = far;
            float fCascadeCount = CascadeCount;
            for (int i = 1; i < splitDepths.Length - 1; i++)
            {
                //float perc = i / fCascadeCount;
                //float percSq = perc * perc;
                //splitDepths[i] = near + ((far - near) * percSq);

                const float splitConstant = 0.95f;
                splitDepths[i] = splitConstant * near * (float)Math.Pow(far / near, i / fCascadeCount) +
                    (1.0f - splitConstant) * ((near + (i / fCascadeCount)) * (far - near));
            }

            return splitDepths;
        }

        private void renderShadowMap(IList<ModelInstance> models, int splitIdx, Matrix lightView, Matrix lightProj)
        {
            int numRows = (int)Math.Sqrt(CascadeCount);
            int cascadeSize = ShadowMapSize / numRows;

            // Set the viewport for the current split            
            Viewport splitViewport = new Viewport();
            splitViewport.MinDepth = 0;
            splitViewport.MaxDepth = 1;
            splitViewport.Width = cascadeSize;
            splitViewport.Height = cascadeSize;
            splitViewport.X = (splitIdx % numRows) * cascadeSize;
            splitViewport.Y = (splitIdx / numRows) * cascadeSize;

            GraphicsDevice.Viewport = splitViewport;

            DepthEffect.Parameters["View"].SetValue(lightView);
            DepthEffect.Parameters["Projection"].SetValue(lightProj);

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
                        GraphicsDevice.SetVertexBuffer(part.VertexBuffer);
                        GraphicsDevice.Indices = part.IndexBuffer;

                        DepthEffect.Parameters["World"].SetValue(models[i].GetBoneMatrix(mesh.ParentBone.Index) * world);

                        for (int m = 0; m < DepthEffect.CurrentTechnique.Passes.Count; m++)
                        {
                            DepthEffect.CurrentTechnique.Passes[m].Apply();

                            GraphicsDevice.DrawIndexedPrimitives(PrimitiveType.TriangleList, part.VertexOffset, 0,
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
