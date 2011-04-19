using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System.Text;
using Microsoft.Xna.Framework.Content;

namespace DeferredRenderer
{
    public abstract class Quad : IHasContent
    {
        protected struct QuadVertex : IVertexType
        {
            public Vector3 Position;
            public Vector3 TexCoordAndCornerIndex;

            public static readonly VertexDeclaration VertexDeclaration = new VertexDeclaration
            (
                new VertexElement(0, VertexElementFormat.Vector3,
                    VertexElementUsage.Position, 0),
                new VertexElement(sizeof(float) * 3, VertexElementFormat.Vector3,
                    VertexElementUsage.TextureCoordinate, 0)
            );

            VertexDeclaration IVertexType.VertexDeclaration { get { return VertexDeclaration; } }
        }

        private VertexBuffer _vertexBuffer;
        public VertexBuffer VertexBuffer
        {
            get { return _vertexBuffer; }
        }

        public virtual void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _vertexBuffer = new VertexBuffer(gd, QuadVertex.VertexDeclaration, 4, BufferUsage.None);
            fillVertexBuffer(_vertexBuffer);
        }
        
        public virtual void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _vertexBuffer.Dispose();
            _vertexBuffer = null;
        }

        protected abstract void fillVertexBuffer(VertexBuffer vb);

        /// <summary>
        /// Draws the full screen quad
        /// </summary>
        /// <param name="graphicsDevice">The GraphicsDevice to use for rendering</param>
        public void Draw(GraphicsDevice graphicsDevice)
        {
            graphicsDevice.SetVertexBuffer(_vertexBuffer);
            graphicsDevice.DrawPrimitives(PrimitiveType.TriangleStrip, 0, 2);
        }
    }
}
