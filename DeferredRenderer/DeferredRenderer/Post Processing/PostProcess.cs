using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public abstract class PostProcess : IHasContent
    {
        private Vector2 _halfPixelOffset;
        public Vector2 HalfPixelOffset
        {
            get { return _halfPixelOffset; }
        }

        public virtual void LoadContent(GraphicsDevice gd, ContentManager cm) 
        {
            _halfPixelOffset = new Vector2(0.5f / gd.Viewport.Width, 0.5f / gd.Viewport.Height);
        }

        public virtual void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _halfPixelOffset = Vector2.Zero;
        }

        public abstract void Render(GraphicsDevice gd, RenderTarget2D source,
            RenderTarget2D destination, GBuffer gBuffer, LightBuffer lightBuffer);
    }
}
