using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    class BillboardQuad : Quad
    {
        protected override void fillVertexBuffer(VertexBuffer vb)
        {
            QuadVertex[] vbData = new QuadVertex[4];

            // Upper right
            vbData[0].Position = new Vector3(0.5f, 0.5f, 0);
            vbData[0].TexCoordAndCornerIndex = new Vector3(0, 0, 1);

            // Lower right
            vbData[1].Position = new Vector3(-0.5f, 0.5f, 0);
            vbData[1].TexCoordAndCornerIndex = new Vector3(1, 0, 2);

            // Upper left
            vbData[2].Position = new Vector3(0.5f, -0.5f, 0);
            vbData[2].TexCoordAndCornerIndex = new Vector3(0, 1, 0);

            // Lower left
            vbData[3].Position = new Vector3(-0.5f, -0.5f, 0);
            vbData[3].TexCoordAndCornerIndex = new Vector3(1, 1, 3);

            vb.SetData(vbData);
        }
    }
}
