using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    /// <summary>
    /// A point(omni) light source
    /// </summary>
    public class PointLight : Light
    {
        protected float _range;
        public float Range
        {
            get { return _range; }
            set { _range = value; }
        }

        private Vector3 _position;
        public Vector3 Position
        {
            get { return _position; }
            set { _position = value; }
        }

        public PointLight(Vector3 color, float intensity, Vector3 pos, float range)
            : base(color, intensity)
        {
            Position = pos;
            Range = range;
        }
    }
}
