using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using System.Text;

namespace DeferredRenderer
{
    public interface IDrawable : IHasContent
    {
        void PreDraw(Renderer r);
        void Draw(Renderer r);
    }
}
