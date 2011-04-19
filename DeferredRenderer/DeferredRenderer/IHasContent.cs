using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using System.Text;

namespace DeferredRenderer
{
    public interface IHasContent
    {
        void LoadContent(GraphicsDevice gd, ContentManager cm);
        void UnloadContent(GraphicsDevice gd, ContentManager cm);
    }
}
