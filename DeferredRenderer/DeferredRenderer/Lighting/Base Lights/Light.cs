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
            set { _color = Vector3.Clamp(value, Vector3.Zero, Vector3.One); }
        }

        private float _intensity;
        public float Intensity
        {
            get { return _intensity; }
            set { _intensity = Math.Max(value, 0.0f); }
        }

        public Light(Vector3 color, float intensity)
        {
            Color = color;
            Intensity = intensity;
        }
    }
}
