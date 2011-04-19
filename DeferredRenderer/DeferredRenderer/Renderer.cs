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
        private RenderTarget2D _lightRT;        

        private Camera _camera;

        private RenderTargetType _displayRT;
        public RenderTargetType DisplayRenderTarget
        {
            get { return _displayRT; }
            set { _displayRT = value; }
        }

        private ShadowRenderer _shadowRenderer;

        private bool _begun;
        
        private FullScreenQuad _fsQuad;

        private Effect _directionLightEffect;
        private Effect _pointLightEffect;
        private Effect _combineEffect;

        private Model _pointLightModel;
        
        private List<ModelInstance> _models;
        private List<PointLight> _pointLights;

        private DirectionLight _mainDirectionLight;
        private List<DirectionLight> _directionLights;

        public Renderer(GraphicsDevice gd)
        {
            _gd = gd;
            _gBuffer = new GBuffer();
            _fsQuad = new FullScreenQuad();

            _models = new List<ModelInstance>();
            _pointLights = new List<PointLight>();
            _directionLights = new List<DirectionLight>();

            _shadowRenderer = new ShadowRenderer(gd);
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _gBuffer.LoadContent(gd, cm);

            Point size = new Point(gd.Viewport.Width, gd.Viewport.Height);

            _lightRT = new RenderTarget2D(gd, size.X, size.Y, false, SurfaceFormat.HdrBlendable,
                DepthFormat.None, 0, RenderTargetUsage.DiscardContents);

            _shadowRenderer.LoadContent(gd, cm);

            _fsQuad.LoadContent(gd, cm);

            _directionLightEffect = cm.Load<Effect>("DirectionLight");
            _pointLightEffect = cm.Load<Effect>("PointLight");
            _combineEffect = cm.Load<Effect>("GBufferCombine");

            _pointLightModel = cm.Load<Model>("Sphere");
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _gBuffer.UnloadContent(gd, cm);
            _shadowRenderer.UnloadContent(gd, cm);

            _lightRT.Dispose();
            _lightRT = null;
                        
            _fsQuad.UnloadContent(gd, cm);

            _directionLightEffect.Dispose();
            _directionLightEffect = null;

            _pointLightEffect.Dispose();
            _pointLightEffect = null;

            _combineEffect.Dispose();
            _combineEffect = null;

            _pointLightModel = null;
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

        public void Begin(Camera cam, DirectionLight mainLight)
        {
            if (_begun)
            {
                throw new InvalidOperationException("Already begun.");
            }
            _begun = true;

            _camera = cam;
            _mainDirectionLight = mainLight;

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
            _gd.SetRenderTarget(_lightRT);
            _gd.Clear(Color.Transparent);

            BlendState blend = new BlendState();
            blend.AlphaBlendFunction = BlendFunction.Add;
            blend.AlphaDestinationBlend = Blend.One;
            blend.AlphaSourceBlend = Blend.One;
            
            blend.ColorBlendFunction = BlendFunction.Add;
            blend.ColorDestinationBlend = Blend.One;
            blend.ColorSourceBlend = Blend.One;

            _gd.BlendState = blend;
            _gd.DepthStencilState = DepthStencilState.None;

            Matrix invViewProj = Matrix.Invert(_camera.View * _camera.Projection);
            Vector2 screenSpaceOffset = new Vector2(0.5f / _gd.Viewport.Width, 0.5f / _gd.Viewport.Height);

            _directionLightEffect.Parameters["RT0"].SetValue(_gBuffer[0]);
            _directionLightEffect.Parameters["RT1"].SetValue(_gBuffer[1]);
            _directionLightEffect.Parameters["RT2"].SetValue(_gBuffer[2]);
            _directionLightEffect.Parameters["RT3"].SetValue(_gBuffer[3]);
            
            _directionLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
            _directionLightEffect.Parameters["CameraPosition"].SetValue(_camera.Position);
            _directionLightEffect.Parameters["ScreenSpaceOffset"].SetValue(screenSpaceOffset);

            for (int i = 0; i < _directionLights.Count; i++)
            {
                _directionLightEffect.Parameters["LightDirection"].SetValue(_directionLights[i].Direction);
                _directionLightEffect.Parameters["LightColor"].SetValue(_directionLights[i].Color);

                for (int j = 0; j < _directionLightEffect.CurrentTechnique.Passes.Count; j++)
                {
                    _directionLightEffect.CurrentTechnique.Passes[j].Apply();

                    _fsQuad.Draw(_gd);
                }
            }            

            if (_pointLights.Count > 0)
            {
                Matrix[] bones = new Matrix[_pointLightModel.Bones.Count];
                _pointLightModel.CopyAbsoluteBoneTransformsTo(bones);

                _pointLightEffect.Parameters["RT0"].SetValue(_gBuffer[0]);
                _pointLightEffect.Parameters["RT1"].SetValue(_gBuffer[1]);
                _pointLightEffect.Parameters["RT2"].SetValue(_gBuffer[2]);
                _pointLightEffect.Parameters["RT3"].SetValue(_gBuffer[3]);
                
                _pointLightEffect.Parameters["InverseViewProjection"].SetValue(invViewProj);
                _pointLightEffect.Parameters["View"].SetValue(_camera.View);
                _pointLightEffect.Parameters["Projection"].SetValue(_camera.Projection);
                _pointLightEffect.Parameters["CameraPosition"].SetValue(_camera.Position);
                _pointLightEffect.Parameters["ScreenSpaceOffset"].SetValue(screenSpaceOffset);

                for (int i = 0; i < _pointLights.Count; i++)
                {
                    _pointLightEffect.Parameters["LightPosition"].SetValue(_pointLights[i].Position);
                    _pointLightEffect.Parameters["LightColor"].SetValue(_pointLights[i].Color);
                    _pointLightEffect.Parameters["LightRadius"].SetValue(_pointLights[i].Range);
                    _pointLightEffect.Parameters["LightIntensity"].SetValue(1.0f);

                    Matrix world = Matrix.CreateScale(_pointLights[i].Range) * 
                        Matrix.CreateTranslation(_pointLights[i].Position);

                    float cameraToCenterSq = Vector3.DistanceSquared(_camera.Position, _pointLights[i].Position);
                    if (cameraToCenterSq < _pointLights[i].Range * _pointLights[i].Range)
                    {
                        _gd.RasterizerState = RasterizerState.CullClockwise;                        
                    }
                    else
                    {
                        _gd.RasterizerState = RasterizerState.CullCounterClockwise;                        
                    }

                    for (int j = 0; j < _pointLightModel.Meshes.Count; j++)
                    {
                        ModelMesh mesh = _pointLightModel.Meshes[j];

                        for (int k = 0; k < mesh.MeshParts.Count; k++)
                        {
                            ModelMeshPart part = mesh.MeshParts[k];
                            _gd.SetVertexBuffer(part.VertexBuffer);
                            _gd.Indices = part.IndexBuffer;

                            _pointLightEffect.Parameters["World"].SetValue(bones[mesh.ParentBone.Index] * world);

                            for (int m = 0; m < _pointLightEffect.CurrentTechnique.Passes.Count; m++)
                            {
                                _pointLightEffect.CurrentTechnique.Passes[m].Apply();

                                _gd.DrawIndexedPrimitives(PrimitiveType.TriangleList, part.VertexOffset, 0,
                                                          part.NumVertices, part.StartIndex,
                                                          part.PrimitiveCount);
                            }
                        }
                    }
                }
            }
            
            _gd.BlendState = BlendState.Opaque;
            _gd.DepthStencilState = DepthStencilState.Default;
            _gd.RasterizerState = RasterizerState.CullCounterClockwise;

            
            // Combine everything
            _gd.SetRenderTarget(null);
            _gd.Clear(Color.Black);

            _combineEffect.Parameters["RT0"].SetValue(_gBuffer[0]);
            _combineEffect.Parameters["RT1"].SetValue(_gBuffer[1]);
            _combineEffect.Parameters["RT2"].SetValue(_gBuffer[2]);
            _combineEffect.Parameters["RT3"].SetValue(_gBuffer[3]);
            _combineEffect.Parameters["LightMap"].SetValue(_lightRT);
            _combineEffect.Parameters["ScreenSpaceOffset"].SetValue(new Vector2(
                0.5f / _gd.Viewport.Width, 0.5f / _gd.Viewport.Height));
            for (int i = 0; i < _combineEffect.CurrentTechnique.Passes.Count; i++)
            {
                _combineEffect.CurrentTechnique.Passes[i].Apply();

                _fsQuad.Draw(_gd);
            }
        }
    }
}
