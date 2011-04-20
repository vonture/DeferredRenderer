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

        public override void RenderUnshadowed(IList<PointLight> lights, Camera camera, GBuffer gBuffer)
        {
            if (lights.Count == 0)
            {
                return;
            }

            BlendState prevBlend = GraphicsDevice.BlendState;
            DepthStencilState prevStencil = GraphicsDevice.DepthStencilState;
            RasterizerState prevRaster = GraphicsDevice.RasterizerState;

            GraphicsDevice.BlendState = LightBlendState;
            GraphicsDevice.DepthStencilState = LightDepthStencilState;

            gBuffer.SetEffectParameters(_pointLightEffect);

            Matrix invViewProj = Matrix.Invert(camera.View * camera.Projection);
            _pointLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
            _pointLightEffect.Parameters["View"].SetValue(camera.View);
            _pointLightEffect.Parameters["Projection"].SetValue(camera.Projection);
            _pointLightEffect.Parameters["CameraPosition"].SetValue(camera.Position);
            _pointLightEffect.Parameters["ScreenSpaceOffset"].SetValue(HalfPixelOffset);

            for (int i = 0; i < lights.Count; i++)
            {
                _pointLightEffect.Parameters["LightPosition"].SetValue(lights[i].Position);
                _pointLightEffect.Parameters["LightColor"].SetValue(lights[i].Color);
                _pointLightEffect.Parameters["LightRadius"].SetValue(lights[i].Range);
                _pointLightEffect.Parameters["LightIntensity"].SetValue(lights[i].Intensity);

                _lightModel.Position = lights[i].Position;
                _lightModel.Scale = new Vector3(lights[i].Range);
                Matrix world = _lightModel.GetWorldMatrix();

                // Depending on if the camera is inside or outside of the light, flip cull direction
                float cameraToCenterSq = Vector3.DistanceSquared(camera.Position, lights[i].Position);
                GraphicsDevice.RasterizerState = (cameraToCenterSq < lights[i].Range * lights[i].Range) ?
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
            
            GraphicsDevice.BlendState = prevBlend;
            GraphicsDevice.DepthStencilState = prevStencil;
            GraphicsDevice.RasterizerState = prevRaster;
        }

        public override void RenderShadowed(IList<PointLight> lights, IList<ModelInstance> models,
            Camera camera, GBuffer gbuffer)
        {
            if (lights.Count == 0)
            {
                return;
            }

            // TODO
            RenderUnshadowed(lights, camera, gbuffer);
        }
    }
}
