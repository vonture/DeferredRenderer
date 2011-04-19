using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace DeferredRenderer
{
    class DirectionLightRenderer : IHasContent
    {
        private GraphicsDevice _gd;

        public DirectionLightRenderer(GraphicsDevice gd)
        {
            _gd = gd;
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
        }
    }
}
