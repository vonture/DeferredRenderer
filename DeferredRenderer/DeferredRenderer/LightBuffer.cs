using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    class LightBuffer : IHasContent
    {
        // LightRT =   Light.r  | Light.g   | Light.b   | Specular Highlight
        private RenderTarget2D _lightRT;
        public RenderTarget2D RenderTarget
        {
            get { return _lightRT; }
        }

        public LightBuffer()
        {
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _lightRT = new RenderTarget2D(gd, gd.Viewport.Width, gd.Viewport.Height, false,
                SurfaceFormat.Color, DepthFormat.None, 0, RenderTargetUsage.DiscardContents);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _lightRT.Dispose();
            _lightRT = null;
        }

        /// <summary>
        /// Sets the render targets ont he graphics card.
        /// </summary>
        /// <param name="gd"></param>
        public void Set(GraphicsDevice gd)
        {
            gd.SetRenderTarget(_lightRT);
        }

        /// <summary>
        /// Assigns the render targets to the specified shader under the parameters RT0-RTX.
        /// </summary>
        /// <param name="effect">Effect to apply to.</param>
        public void SetEffectParameters(Effect effect)
        {
            effect.Parameters["LightMap"].SetValue(_lightRT);
        }

        /// <summary>
        /// Clears all of the rendertargets to their default values.  Assumes the render targets
        /// are already set.
        /// </summary>
        public void Clear(GraphicsDevice gd)
        {
            gd.Clear(Color.Transparent);
        }
    }
}
