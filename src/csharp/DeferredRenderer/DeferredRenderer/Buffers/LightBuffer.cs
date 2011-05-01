using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class LightBuffer : DeferredBuffer
    {
        // LightRT =   Light.r  | Light.g   | Light.b   | Specular Highlight
        private RenderTarget2D _lightRT;
        public override RenderTarget2D this[int index]
        {
            get
            {
                if (index != 0)
                {
                    throw new ArgumentException("Index out of range.");
                }

                return _lightRT;
            }
        }
        public override int Count
        {
            get { return 1; }
        }

        public LightBuffer()
        {
        }

        public override void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _lightRT = new RenderTarget2D(gd, gd.Viewport.Width, gd.Viewport.Height, false,
                SurfaceFormat.HdrBlendable, DepthFormat.None, 0, RenderTargetUsage.DiscardContents);

            base.LoadContent(gd, cm);
        }

        public override void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _lightRT.Dispose();
            _lightRT = null;

            base.UnloadContent(gd, cm);
        }

        public override void Set(GraphicsDevice gd)
        {
            gd.SetRenderTarget(_lightRT);
        }

        public override void SetEffectParameters(Effect effect)
        {
            effect.Parameters["LightMap"].SetValue(_lightRT);
        }

        public override void Clear(GraphicsDevice gd)
        {
            gd.Clear(Color.Transparent);
        }
    }
}
