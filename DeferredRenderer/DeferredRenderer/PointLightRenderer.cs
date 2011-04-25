using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    class PointLightRenderer : LightRenderer<PointLight>
    {
        private Effect _pointLightEffect;
        private ModelInstance _lightModel;

        public PointLightRenderer(GraphicsDevice gd)
            : base(gd)
        {
            _lightModel = new ModelInstance("sphere");
        }

        public override void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _pointLightEffect = cm.Load<Effect>("PointLight");
            _lightModel.LoadContent(gd, cm);

            base.LoadContent(gd, cm);
        }

        public override void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _pointLightEffect.Dispose();
            _pointLightEffect = null;

            _lightModel.UnloadContent(gd, cm);

            base.UnloadContent(gd, cm);
        }

        protected override void renderShadowMaps(IList<ModelInstance> models, Camera camera,
            BoundingBox sceneBounds)
        {
        }
        
        protected override void renderLights(Camera camera, GBuffer gBuffer)
        {
            RasterizerState prevRaster = GraphicsDevice.RasterizerState;

            // prep effect parameters
            Matrix invViewProj = Matrix.Invert(camera.View * camera.Projection);
            _pointLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
            _pointLightEffect.Parameters["View"].SetValue(camera.View);
            _pointLightEffect.Parameters["Projection"].SetValue(camera.Projection);
            _pointLightEffect.Parameters["CameraPosition"].SetValue(camera.Position);
            _pointLightEffect.Parameters["ScreenSpaceOffset"].SetValue(HalfPixelOffset);

            gBuffer.SetEffectParameters(_pointLightEffect);
            
            // render unshadowed
            for (int i = 0; i < Count(false); i++)
            {
                PointLight light = GetLight(i, false);

                _lightModel.Position = light.Position;
                _lightModel.Scale = new Vector3(light.Range);
                Matrix world = _lightModel.World;

                ContainmentType containType = camera.BoundingFrustum.Contains(_lightModel.BoundingBox);
                if (containType == ContainmentType.Disjoint)
                {
                    continue;
                }

                _pointLightEffect.Parameters["LightPosition"].SetValue(light.Position);
                _pointLightEffect.Parameters["LightColor"].SetValue(light.Color);
                _pointLightEffect.Parameters["LightRadius"].SetValue(light.Range);
                _pointLightEffect.Parameters["LightIntensity"].SetValue(light.Intensity);
                
                // Depending on if the camera is inside or outside of the light, flip cull direction
                float cameraToCenterSq = Vector3.DistanceSquared(camera.Position, light.Position);
                GraphicsDevice.RasterizerState = (cameraToCenterSq < light.Range * light.Range) ?
                        RasterizerState.CullClockwise : RasterizerState.CullCounterClockwise;

                for (int j = 0; j < _lightModel.Model.Meshes.Count; j++)
                {
                    ModelMesh mesh = _lightModel.Model.Meshes[j];

                    for (int k = 0; k < mesh.MeshParts.Count; k++)
                    {
                        ModelMeshPart part = mesh.MeshParts[k];
                        GraphicsDevice.SetVertexBuffer(part.VertexBuffer);
                        GraphicsDevice.Indices = part.IndexBuffer;

                        Matrix boneMatrix = _lightModel.GetBoneMatrix(mesh.ParentBone.Index);
                        _pointLightEffect.Parameters["World"].SetValue(boneMatrix * world);

                        for (int m = 0; m < _pointLightEffect.CurrentTechnique.Passes.Count; m++)
                        {
                            _pointLightEffect.CurrentTechnique.Passes[m].Apply();

                            GraphicsDevice.DrawIndexedPrimitives(PrimitiveType.TriangleList,
                                part.VertexOffset, 0, part.NumVertices, part.StartIndex,
                                part.PrimitiveCount);
                        }
                    }
                }
            }

            // render shadowed (same as unshadowed right now)
            for (int i = 0; i < Count(true); i++)
            {
                PointLight light = GetLight(i, false);

                _lightModel.Position = light.Position;
                _lightModel.Scale = new Vector3(light.Range);
                Matrix world = _lightModel.World;

                ContainmentType containType = camera.BoundingFrustum.Contains(_lightModel.BoundingBox);
                if (containType == ContainmentType.Disjoint)
                {
                    continue;
                }

                _pointLightEffect.Parameters["LightPosition"].SetValue(light.Position);
                _pointLightEffect.Parameters["LightColor"].SetValue(light.Color);
                _pointLightEffect.Parameters["LightRadius"].SetValue(light.Range);
                _pointLightEffect.Parameters["LightIntensity"].SetValue(light.Intensity);

                // Depending on if the camera is inside or outside of the light, flip cull direction
                float cameraToCenterSq = Vector3.DistanceSquared(camera.Position, light.Position);
                GraphicsDevice.RasterizerState = (cameraToCenterSq < light.Range * light.Range) ?
                        RasterizerState.CullClockwise : RasterizerState.CullCounterClockwise;

                for (int j = 0; j < _lightModel.Model.Meshes.Count; j++)
                {
                    ModelMesh mesh = _lightModel.Model.Meshes[j];

                    for (int k = 0; k < mesh.MeshParts.Count; k++)
                    {
                        ModelMeshPart part = mesh.MeshParts[k];
                        GraphicsDevice.SetVertexBuffer(part.VertexBuffer);
                        GraphicsDevice.Indices = part.IndexBuffer;

                        Matrix boneMatrix = _lightModel.GetBoneMatrix(mesh.ParentBone.Index);
                        _pointLightEffect.Parameters["World"].SetValue(boneMatrix * world);

                        for (int m = 0; m < _pointLightEffect.CurrentTechnique.Passes.Count; m++)
                        {
                            _pointLightEffect.CurrentTechnique.Passes[m].Apply();

                            GraphicsDevice.DrawIndexedPrimitives(PrimitiveType.TriangleList,
                                part.VertexOffset, 0, part.NumVertices, part.StartIndex,
                                part.PrimitiveCount);
                        }
                    }
                }
            }


            GraphicsDevice.RasterizerState = prevRaster;
        }
    }
}
