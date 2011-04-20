using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    /// <summary>
    /// A spotlight
    /// </summary>
    public class SpotLight : PointLight
    {
        protected float _innerCone;
        public float InnerCone
        {
            get { return _innerCone; }
            set { _innerCone = Math.Max(value, 0.0f); }
        }

        protected float _outerCone;
        public float OuterCone
        {
            get { return _outerCone; }
            set { _outerCone = Math.Max(value, 0.0f); }
        }

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

        public SpotLight(Vector3 color, float intensity, Vector3 pos, Vector3 dir, float range,
            float innerCone, float outerCone)
            : base(color, intensity, pos, range)
        {
            Direction = dir;
            _innerCone = innerCone;
            _outerCone = outerCone;
        }
    }
}
