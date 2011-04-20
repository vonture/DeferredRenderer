using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class GBuffer : IHasContent
    {
        /// <summary>
        /// RT0 =       Diffuse.r           | Diffuse.g         | Diffuse.b     | Specular Intensity
        /// RT1 =       Normal.x            | Normal.y          | Normal.z      | Specular Power
        /// RT2 =                           | Ambient Occlusion | Translucency  | Material ID
        /// RT3 =       Depth               |                   |               |        
        /// </summary>
        private RenderTarget2D[] _renderTargets;

        public RenderTarget2D this[int index]
        {
            get { return _renderTargets[index]; }
        }
        public int Count
        {
            get { return _renderTargets.Length; }
        }

        private Effect _clearEffect;
        private FullScreenQuad _fsQuad;

        public GBuffer()
        {
            _renderTargets = new RenderTarget2D[4];
            _fsQuad = new FullScreenQuad();
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            Point size = new Point(gd.Viewport.Width, gd.Viewport.Height);

            _renderTargets[0] = new RenderTarget2D(gd, size.X, size.Y, false, SurfaceFormat.Color,
                DepthFormat.Depth24Stencil8, 0, RenderTargetUsage.DiscardContents);

            _renderTargets[1] = new RenderTarget2D(gd, size.X, size.Y, false, SurfaceFormat.Color,
                DepthFormat.None, 0, RenderTargetUsage.DiscardContents);

            _renderTargets[2] = new RenderTarget2D(gd, size.X, size.Y, false, SurfaceFormat.Color,
                DepthFormat.None, 0, RenderTargetUsage.DiscardContents);

            _renderTargets[3] = new RenderTarget2D(gd, size.X, size.Y, false, SurfaceFormat.Single,
                DepthFormat.None, 0, RenderTargetUsage.DiscardContents);

            _clearEffect = cm.Load<Effect>("GBufferClear");

            _fsQuad.LoadContent(gd, cm);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            for (int i = 0; i < _renderTargets.Length; i++)
            {
                _renderTargets[i].Dispose();
                _renderTargets[i] = null;
            }

            _clearEffect.Dispose();
            _clearEffect = null;

            _fsQuad.UnloadContent(gd, cm);
        }

        /// <summary>
        /// Sets the render targets ont he graphics card.
        /// </summary>
        /// <param name="gd"></param>
        public void Set(GraphicsDevice gd)
        {
            gd.SetRenderTargets(_renderTargets[0], _renderTargets[1], _renderTargets[2], _renderTargets[3]);
        }

        /// <summary>
        /// Assigns the render targets to the specified shader under the parameters RT0-RTX.
        /// </summary>
        /// <param name="effect">Effect to apply to.</param>
        public void SetEffectParameters(Effect effect)
        {
            effect.Parameters["RT0"].SetValue(_renderTargets[0]);
            effect.Parameters["RT1"].SetValue(_renderTargets[1]);
            effect.Parameters["RT2"].SetValue(_renderTargets[2]);
            effect.Parameters["RT3"].SetValue(_renderTargets[3]);
        }

        /// <summary>
        /// Clears all of the rendertargets to their default values.  Assumes the render targets
        /// are already set.
        /// </summary>
        public void Clear(GraphicsDevice gd)
        {
            for (int i = 0; i < _clearEffect.CurrentTechnique.Passes.Count; i++)
            {
                _clearEffect.CurrentTechnique.Passes[i].Apply();

                _fsQuad.Draw(gd);
            }
        }
    }
}
