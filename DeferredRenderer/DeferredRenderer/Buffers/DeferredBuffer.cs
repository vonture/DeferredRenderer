using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace DeferredRenderer
{
    public abstract class DeferredBuffer : IHasContent
    {
        /// <summary>
        /// Get the buffer at this index.
        /// </summary>
        public abstract RenderTarget2D this[int index] { get; }

        /// <summary>
        /// The number of render targets in this buffer.
        /// </summary>
        public abstract int Count { get; }

        public virtual void LoadContent(GraphicsDevice gd, ContentManager cm) { }
        public virtual void UnloadContent(GraphicsDevice gd, ContentManager cm) { }

        /// <summary>
        /// Sets the render targets ont he graphics card.
        /// </summary>
        public abstract void Set(GraphicsDevice gd);

        /// <summary>
        /// Assigns the render targets to the specified shader under the parameters RT0-RTX.
        /// </summary>
        /// <param name="effect">Effect to apply to.</param>
        public abstract void SetEffectParameters(Effect effect);

        /// <summary>
        /// Clears all of the rendertargets to their default values.  Assumes the render targets
        /// are already set.
        /// </summary>
        public abstract void Clear(GraphicsDevice gd);
    }
}
