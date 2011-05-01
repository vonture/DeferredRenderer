using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    /// <summary>
    /// A directional(infinite) light source
    /// </summary>
    public class DirectionLight : Light
    {
        private Vector3 _direction;
        /// <summary>
        /// Gets or sets the direction of the light
        /// </summary>
        public Vector3 Direction
        {
            get { return _direction; }
            set 
            {
                if (value == Vector3.Zero)
                {
                    value = Vector3.Forward;
                }
                _direction = Vector3.Normalize(value);
            }
        }

        public DirectionLight(Vector3 color, float intensity, Vector3 dir)
            : base(color, intensity)
        {
            Direction = dir;
        }
    }
}
