using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public abstract class LightRenderer<T> : IHasContent 
        where T : Light
    {
        private const int SHADOWMAP_SIZE_MIN = 2;
        private const int SHADOWMAP_SIZE_MAX = 4096;

        private int _shadowMapSize;
        public int ShadowMapSize
        {
            get { return _shadowMapSize; }
            set
            {
                if (value < SHADOWMAP_SIZE_MIN || value > SHADOWMAP_SIZE_MAX)
                {
                    throw new ArgumentException("Shadow map size must be between " + SHADOWMAP_SIZE_MIN +
                        " and " + SHADOWMAP_SIZE_MAX + ".");
                }

                if ((value & (value - 1)) != 0)
                {
                    throw new ArgumentException("Shadow map size must be a power of 2.");
                }

                if (_shadowMapSize != value)
                {
                    _shadowMapSize = value;
                    destroyDeptyRT();
                    buildDepthRT(_gd);
                    OnShadowParameterChange();
                }
            }
        }

        private const int CASCADE_COUNT_MIN = 1;
        private const int CASCADE_COUNT_MAX = 9;

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

                if ((float)Math.Sqrt(value) % 1 != 0)
                {
                    throw new ArgumentException("Cascade count must be a perfect square.");
                }

                if (_cascadeCount != value)
                {
                    _cascadeCount = value;
                    destroyDeptyRT();
                    buildDepthRT(_gd);
                    OnShadowParameterChange();
                }
            }
        }

        private const int DEPTHRT_COUNT_MIN = 1;
        private const int DEPTHRT_COUNT_MAX = 8;

        private List<RenderTarget2D> _depthRTs;

        private int _depthRTCount;
        public int ShadowRTCount
        {
            get
            {
                return _depthRTCount;
            }
            set
            {
                if (value < DEPTHRT_COUNT_MIN || value > DEPTHRT_COUNT_MAX)
                {
                    throw new ArgumentException("Shadow RT count must be between " + DEPTHRT_COUNT_MIN +
                        " and " + DEPTHRT_COUNT_MAX + ".");
                }

                if (_depthRTCount != value)
                {
                    buildDepthRT(_gd);
                }
            }
        }

        private Effect _depthEffect;
        protected Effect DepthEffect
        {
            get { return _depthEffect; }
        }

        private BlendState _lightBlendState;
        protected BlendState LightBlendState
        {
            get { return _lightBlendState; }
        }

        private DepthStencilState _lightDepthStencilState;
        protected DepthStencilState LightDepthStencilState
        {
            get { return _lightDepthStencilState; }
        }

        private Vector2 _halfPixelOffset;
        public Vector2 HalfPixelOffset
        {
            get { return _halfPixelOffset; }
        }

        private GraphicsDevice _gd;
        protected GraphicsDevice GraphicsDevice
        {
            get { return _gd; }
        }

        public LightRenderer(GraphicsDevice gd)
        {
            _gd = gd;

            _lightBlendState = new BlendState();
            _lightBlendState.ColorBlendFunction = BlendFunction.Add;
            _lightBlendState.ColorDestinationBlend = Blend.One;
            _lightBlendState.ColorSourceBlend = Blend.One;
            _lightBlendState.AlphaBlendFunction = BlendFunction.Add;
            _lightBlendState.AlphaDestinationBlend = Blend.One;
            _lightBlendState.AlphaSourceBlend = Blend.One;

            _lightDepthStencilState = DepthStencilState.None;

            _depthRTs = new List<RenderTarget2D>();

            _halfPixelOffset = Vector2.Zero;

            _shadowMapSize = SHADOWMAP_SIZE_MAX;
            _cascadeCount = 9;
            _depthRTCount = (DEPTHRT_COUNT_MIN + DEPTHRT_COUNT_MAX) / 2;
        }

        public virtual void LoadContent(GraphicsDevice gd, ContentManager cm) 
        {
            _depthEffect = cm.Load<Effect>("DepthMapEffect");
            _halfPixelOffset = new Vector2(0.5f / _gd.Viewport.Width, 0.5f / _gd.Viewport.Height);

            buildDepthRT(gd);
        }

        public virtual void UnloadContent(GraphicsDevice gd, ContentManager cm) 
        {
            _depthEffect.Dispose();
            _depthEffect = null;

            _halfPixelOffset = Vector2.Zero;

            destroyDeptyRT();
        }

        private void destroyDeptyRT()
        {
            for (int i = 0; i < _depthRTs.Count; i++)
            {
                _depthRTs[i].Dispose();
            }
            _depthRTs.Clear();
        }

        private void buildDepthRT(GraphicsDevice gd)
        {
            for (int i = 0; i < _depthRTCount; i++)
            {
                _depthRTs.Add(new RenderTarget2D(gd, _shadowMapSize, _shadowMapSize, false,
                    SurfaceFormat.Vector2, DepthFormat.Depth24Stencil8, 0, RenderTargetUsage.DiscardContents));
            }
        }

        protected RenderTarget2D GetDepthRT(int idx)
        {
            return _depthRTs[idx];
        }
        
        protected virtual void OnShadowParameterChange() { }

        /// <summary>
        /// Render unshadowed lights.  Assumes that the light render target is already set.
        /// </summary>
        /// <param name="lights">Lights to render</param>
        /// <param name="camera">Main view camera.</param>
        /// <param name="gbuffer">Gbuffer to pull material information from.</param>
        public abstract void RenderUnshadowed(IList<T> lights, Camera camera, GBuffer gbuffer);

        /// <summary>
        /// Render shadowed lights.  Assumes that the light render target is already set.
        /// </summary>
        /// <param name="lights">Lights to render</param>
        /// <param name="camera">Main view camera.</param>
        /// <param name="gbuffer">Gbuffer to pull material information from.</param>
        public abstract void RenderShadowed(IList<T> lights, IList<ModelInstance> models, Camera camera,
            GBuffer gbuffer);
    }
}
