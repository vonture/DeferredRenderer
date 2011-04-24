using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;
using System.Diagnostics;

namespace DeferredRenderer
{
    public enum RenderTargetType
    {
        Final,
        RT0,
        RT1,
        RT2,
        RT3,
        LightDepth,
    }

    public class Renderer : IHasContent
    {
        private GraphicsDevice _gd;

        /// <summary>
        /// RT0 =       Diffuse.r           | Diffuse.g         | Diffuse.b     | Specular Intensity
        /// RT1 =       Normal.x            | Normal.y          | Normal.z      | Specular Power
        /// RT2 =                           | Ambient Occlusion | Translucency  | Material ID
        /// RT3 =       Depth               |                   |               |        
        /// </summary>
        private GBuffer _gBuffer;        

        // LightRT =   Light.r             | Light.g           | Light.b       | Specular Highlight
        private LightBuffer _lightBuffer;        

        private Camera _camera;

        private RenderTargetType _displayRT;
        public RenderTargetType DisplayRenderTarget
        {
            get { return _displayRT; }
            set { _displayRT = value; }
        }

        private PointLightRenderer _pointLightRenderer;
        private DirectionLightRenderer _directionLightRenderer;

        private bool _begun;
        
        private FullScreenQuad _fsQuad;

        private Effect _combineEffect;
                
        private List<ModelInstance> _models;

        private List<PointLight> _pointLights;
        private List<DirectionLight> _directionLights;

        private Vector2 _halfPixelOffset;

        public Renderer(GraphicsDevice gd)
        {
            _gd = gd;
            _gBuffer = new GBuffer();
            _lightBuffer = new LightBuffer();
            _fsQuad = new FullScreenQuad();

            _models = new List<ModelInstance>();
            _pointLights = new List<PointLight>();
            _directionLights = new List<DirectionLight>();

            _pointLightRenderer = new PointLightRenderer(gd);
            _directionLightRenderer = new DirectionLightRenderer(gd);

            _halfPixelOffset = Vector2.Zero;
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _gBuffer.LoadContent(gd, cm);
            _lightBuffer.LoadContent(gd, cm);

            _pointLightRenderer.LoadContent(gd, cm);
            _directionLightRenderer.LoadContent(gd, cm);

            _fsQuad.LoadContent(gd, cm);

            _halfPixelOffset = new Vector2(0.5f / gd.Viewport.Width, 0.5f / gd.Viewport.Height);

            _combineEffect = cm.Load<Effect>("GBufferCombine");
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _gBuffer.UnloadContent(gd, cm);
            _lightBuffer.UnloadContent(gd, cm);

            _pointLightRenderer.UnloadContent(gd, cm);
            _directionLightRenderer.UnloadContent(gd, cm);
                        
            _fsQuad.UnloadContent(gd, cm);

            _halfPixelOffset = Vector2.Zero;

            _combineEffect.Dispose();
            _combineEffect = null;
        }

        public void DrawModel(ModelInstance model)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _models.Add(model);
        }

        public void DrawLight(PointLight plight)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _pointLights.Add(plight);
        }

        public void DrawLight(DirectionLight dlight)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _directionLights.Add(dlight);
        }

        public void Begin(Camera cam)
        {
            if (_begun)
            {
                throw new InvalidOperationException("Already begun.");
            }
            _begun = true;

            _camera = cam;

            _models.Clear();
            _pointLights.Clear();
            _directionLights.Clear();
        }

        public void End()
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }
            _begun = false;

            // Set and clear the g-buffer for a new set of draw calls
            _gBuffer.Set(_gd);
            _gBuffer.Clear(_gd);

            // Render all the models
            for (int i = 0; i < _models.Count; i++)
            {
                Model model = _models[i].Model;
                Matrix world = _models[i].GetWorldMatrix();

                for (int j = 0; j < model.Meshes.Count; j++)
                {
                    ModelMesh mesh = model.Meshes[j];

                    for (int k = 0; k < mesh.MeshParts.Count; k++)
                    {
                        ModelMeshPart part = mesh.MeshParts[k];
                        _gd.SetVertexBuffer(part.VertexBuffer);
                        _gd.Indices = part.IndexBuffer;

                        Effect effect = part.Effect;
                        effect.Parameters["World"].SetValue(_models[i].GetBoneMatrix(mesh.ParentBone.Index) * world);
                        effect.Parameters["View"].SetValue(_camera.View);
                        effect.Parameters["Projection"].SetValue(_camera.Projection);

                        for (int m = 0; m < effect.CurrentTechnique.Passes.Count; m++)
                        {
                            effect.CurrentTechnique.Passes[m].Apply();

                            _gd.DrawIndexedPrimitives(PrimitiveType.TriangleList, part.VertexOffset, 0,
                                                      part.NumVertices, part.StartIndex,
                                                      part.PrimitiveCount);
                        }
                    }
                }
            }
            
            // Render lights
            _lightBuffer.Set(_gd);
            _lightBuffer.Clear(_gd);

            _directionLightRenderer.RenderShadowed(_directionLights, _models, _camera, _gBuffer);
            //_pointLightRenderer.RenderShadowed(_pointLights, _models, _camera, _gBuffer);

            //_directionLightRenderer.RenderUnshadowed(_directionLights, _camera, _gBuffer);
            //_pointLightRenderer.RenderUnshadowed(_pointLights, _camera, _gBuffer);

            // Combine everything
            _gd.SetRenderTarget(null);
            _gd.Clear(Color.Black);

            _gBuffer.SetEffectParameters(_combineEffect);
            _lightBuffer.SetEffectParameters(_combineEffect);

            _combineEffect.Parameters["ScreenSpaceOffset"].SetValue(_halfPixelOffset);
            for (int i = 0; i < _combineEffect.CurrentTechnique.Passes.Count; i++)
            {
                _combineEffect.CurrentTechnique.Passes[i].Apply();

                _fsQuad.Draw(_gd);
            }
        }
    }
}
