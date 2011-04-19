using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System.Text;

namespace DeferredRenderer
{
    /// <summary>
	/// Abstract base class for all lights
	/// </summary>
    public abstract class Light
    {
        private Vector3 _color;
        public Vector3 Color
        {
            get { return _color; }
            set { _color = value; }
        }

        public Light(Vector3 col)
        {
            _color = col;
        }
    }
}
