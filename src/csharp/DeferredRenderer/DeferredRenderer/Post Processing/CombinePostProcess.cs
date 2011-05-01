using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class CombinePostProcess : PostProcess
    {
        private Effect _combineEffect;
        private FullScreenQuad _fsQuad;
        private Vector2 _halfPixel;

        private GBufferCombineChannels _displayChannels;
        public GBufferCombineChannels DisplayChannels
        {
            get { return _displayChannels; }
            set { _displayChannels = value; }
        }

        public CombinePostProcess()
        {
            _fsQuad = new FullScreenQuad();
        }

        public override void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _combineEffect = cm.Load<Effect>("GBufferCombine");
            _fsQuad.LoadContent(gd, cm);

            _halfPixel = new Vector2(0.5f / gd.Viewport.Width, 0.5f / gd.Viewport.Height);

            base.LoadContent(gd, cm);
        }

        public override void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _combineEffect.Dispose();
            _combineEffect = null;

            _fsQuad.UnloadContent(gd, cm);

            _halfPixel = Vector2.Zero;

            base.UnloadContent(gd, cm);
        }

        public override void Render(GraphicsDevice gd, RenderTarget2D source,
            RenderTarget2D destination, GBuffer gBuffer, LightBuffer lightBuffer)
        {
            gd.SetRenderTarget(destination);
            gd.Clear(Color.Black);

            gBuffer.SetEffectParameters(_combineEffect);
            lightBuffer.SetEffectParameters(_combineEffect);

            _combineEffect.CurrentTechnique = _combineEffect.Techniques[_displayChannels.ToString()];
            _combineEffect.Parameters["ScreenSpaceOffset"].SetValue(_halfPixel);

            _fsQuad.Draw(gd, _combineEffect);
        }
    }
}
