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
        /// <summary>
        /// Gets or sets the width of the inner cone, in radians
        /// </summary>
        public float InnerCone
        {
            get { return _innerCone; }
            set { _innerCone = value; }
        }

        protected float _outerCone;
        /// <summary>
        /// Gets or sets the width of the outer cone, in radians
        /// </summary>
        public float OuterCone
        {
            get { return _outerCone; }
            set { _outerCone = value; }
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

        public SpotLight(Vector3 c, Vector3 pos, Vector3 dir, float range, float innerCone, float outerCone)
            : base(c, pos, range)
        {
            Direction = dir;
            _innerCone = innerCone;
            _outerCone = outerCone;
        }
    }
}
