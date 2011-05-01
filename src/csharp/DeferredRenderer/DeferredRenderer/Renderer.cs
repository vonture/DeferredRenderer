using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;
using System.Diagnostics;

namespace DeferredRenderer
{
    public enum GBufferCombineChannels
    {
        Final,
        DiffuseColor,
        SpecularIntensity,
        Normals,
        SpecularPower,
        AmbientOcclusion,
        Translucency,
        Depth,
        LightColor,
        LightSpecular,
    }

    public class Renderer : IHasContent
    {
        private GraphicsDevice _gd;

        private SpriteBatch _sb;
        private SpriteFont _debugFont;

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

        private GBufferCombineChannels _displayChannels;
        public GBufferCombineChannels DisplayChannels
        {
            get { return _displayChannels; }
            set { _displayChannels = value; }
        }

        private PointLightRenderer _pointLightRenderer;
        private DirectionLightRenderer _directionLightRenderer;

        private bool _begun;
        
        private FullScreenQuad _fsQuad;

        private RenderTarget2D _ppRT1;
        private RenderTarget2D _ppRT2;

        private CombinePostProcess _combinePP;
        private List<PostProcess> _postProcesses;

        private List<ModelInstance> _models;        
        private List<string> _debugStrings;

        private Vector2 _halfPixelOffset;

        public Renderer(GraphicsDevice gd)
        {
            _gd = gd;
            _gBuffer = new GBuffer();
            _lightBuffer = new LightBuffer();
            _fsQuad = new FullScreenQuad();

            _models = new List<ModelInstance>();
            _postProcesses = new List<PostProcess>();
            _debugStrings = new List<string>();
            
            _combinePP = new CombinePostProcess();

            _pointLightRenderer = new PointLightRenderer(gd);
            _directionLightRenderer = new DirectionLightRenderer(gd);

            _halfPixelOffset = Vector2.Zero;
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _sb = new SpriteBatch(gd);

            _debugFont = cm.Load<SpriteFont>("DebugFont");

            _gBuffer.LoadContent(gd, cm);
            _lightBuffer.LoadContent(gd, cm);

            _combinePP.LoadContent(gd, cm);

            _ppRT1 = new RenderTarget2D(_gd, _gd.Viewport.Width, _gd.Viewport.Height, false,
                SurfaceFormat.HalfVector4, DepthFormat.None);
            _ppRT2 = new RenderTarget2D(_gd, _gd.Viewport.Width, _gd.Viewport.Height, false,
                SurfaceFormat.HalfVector4, DepthFormat.None);

            _pointLightRenderer.LoadContent(gd, cm);
            _directionLightRenderer.LoadContent(gd, cm);

            _fsQuad.LoadContent(gd, cm);

            _halfPixelOffset = new Vector2(0.5f / gd.Viewport.Width, 0.5f / gd.Viewport.Height);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _sb.Dispose();
            _sb = null;

            _debugFont = null;

            _gBuffer.UnloadContent(gd, cm);
            _lightBuffer.UnloadContent(gd, cm);

            _combinePP.UnloadContent(gd, cm);

            _pointLightRenderer.UnloadContent(gd, cm);
            _directionLightRenderer.UnloadContent(gd, cm);
                        
            _fsQuad.UnloadContent(gd, cm);

            _halfPixelOffset = Vector2.Zero;
        }

        public void DrawModel(ModelInstance model)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _models.Add(model);
        }

        public void DrawLight(PointLight plight, bool shadowed)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _pointLightRenderer.Add(plight, shadowed);
        }

        public void DrawLight(DirectionLight dlight, bool shadowed)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _directionLightRenderer.Add(dlight, shadowed);
        }

        public void AddPostProcess(PostProcess pp)
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }

            _postProcesses.Add(pp);
        }

        public void DrawDebugString(string text)
        {
            _debugStrings.Add(text);
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
            _debugStrings.Clear();
            _postProcesses.Clear();
            _directionLightRenderer.Clear();
            _pointLightRenderer.Clear();
        }

        public void End()
        {
            if (!_begun)
            {
                throw new InvalidOperationException("Not begun.");
            }
            _begun = false;
            
            // calculate the scene bounding box
            BoundingBox sceneBB = (_models.Count > 0) ? _models[0].BoundingBox : new BoundingBox();
            for (int i = 1; i < _models.Count; i++)
            {
                sceneBB = BoundingBox.CreateMerged(sceneBB, _models[i].BoundingBox);
            }

            //DrawDebugString("Scene bounds: " + sceneBB + ".");
            
            // Render the shadow maps to the appropriate depth buffers
            _directionLightRenderer.RenderShadowMaps(_models, _camera, new BoundingBox());
            _pointLightRenderer.RenderShadowMaps(_models, _camera, new BoundingBox());            

            // Set and clear the g-buffer for a new set of draw calls
            _gBuffer.Set(_gd);
            _gBuffer.Clear(_gd);

            // Render all the models
            int modelsDrawn = 0;
            int numTriangles = 0;
            BoundingFrustum cameraFrust = _camera.BoundingFrustum;
            for (int i = 0; i < _models.Count; i++)
            {
                Model model = _models[i].Model;
                Matrix world = _models[i].World;
                BoundingBox bb = _models[i].BoundingBox;

                ContainmentType containType = cameraFrust.Contains(bb);
                if (containType == ContainmentType.Disjoint)
                {
                    continue;
                }
                modelsDrawn++;

                for (int j = 0; j < model.Meshes.Count; j++)
                {
                    ModelMesh mesh = model.Meshes[j];

                    for (int k = 0; k < mesh.MeshParts.Count; k++)
                    {
                        ModelMeshPart part = mesh.MeshParts[k];
                        _gd.SetVertexBuffer(part.VertexBuffer);
                        _gd.Indices = part.IndexBuffer;

                        numTriangles += part.PrimitiveCount;

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

            DrawDebugString("Models: " + _models.Count + " ("  + modelsDrawn + " drawn, " + numTriangles + " triangles)");
            
            // Render lights
            _lightBuffer.Set(_gd);
            _lightBuffer.Clear(_gd);

            _directionLightRenderer.RenderLights(_camera, _gBuffer);
            DrawDebugString("Directional lights: " + (_directionLightRenderer.Count(false) + _directionLightRenderer.Count(true)) +
                " (" + _directionLightRenderer.Count(true) + " shadowed)");

            _pointLightRenderer.RenderLights(_camera, _gBuffer);
            DrawDebugString("Point lights: " + (_pointLightRenderer.Count(false) + _pointLightRenderer.Count(true)) +
                " (" + _pointLightRenderer.Count(true) + " shadowed)");


            // Combine everything
            if (_displayChannels != GBufferCombineChannels.Final)
            {
                _postProcesses.Clear();
            }

            _combinePP.DisplayChannels = _displayChannels;
            RenderTarget2D destTarget = (_postProcesses.Count > 0) ? _ppRT1 : null;
            _combinePP.Render(_gd, null, destTarget, _gBuffer, _lightBuffer);

            // Render post processes
            for (int i = 0; i < _postProcesses.Count; i++)
            {
                RenderTarget2D src =  (i % 2 == 0) ? _ppRT1 : _ppRT2;
                RenderTarget2D dest = (i % 2 != 0) ? _ppRT1 : _ppRT2;

                if (i == _postProcesses.Count - 1)
                {
                    dest = null;
                }

                _postProcesses[i].Render(_gd, src, dest, _gBuffer, _lightBuffer);
            }

            DrawDebugString("Post processes: " + _postProcesses.Count);

            if (_debugStrings.Count > 0)
            {
                int screenWidth = _gd.Viewport.Width;

                _sb.Begin();
                for (int i = 0; i < _debugStrings.Count; i++)
                {
                    string text = _debugStrings[i];
                    Vector2 textSize = _debugFont.MeasureString(text);

                    Vector2 pos = new Vector2(screenWidth - (int)textSize.X - 2, i * _debugFont.LineSpacing);
                    _sb.DrawString(_debugFont, text, pos, Color.White);
                }
                _sb.End();

                _gd.BlendState = BlendState.Opaque;
                _gd.RasterizerState = RasterizerState.CullCounterClockwise;
                _gd.DepthStencilState = DepthStencilState.Default;
            }
        }
    }
}
