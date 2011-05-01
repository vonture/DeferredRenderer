using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using System.Text;

namespace DeferredRenderer
{
    public class OrthographicCamera : Camera
    {
        private float _width;
        public float Width
        {
            get
            {
                return _width;
            }
            set
            {
                if (value <= 0f)
                {
                    throw new ArgumentOutOfRangeException();
                }
                Dirty = true;
                _width = value;
            }
        }

        private float _height;
        public float Height
        {
            get
            {
                return _height;
            }
            set
            {
                if (value <= 0f)
                {
                    throw new ArgumentOutOfRangeException();
                }
                Dirty = true;
                _height = value;
            }
        }

        public OrthographicCamera(float near, float far, float width, float height)
            : base(near, far)
        {
            Width = width;
            Height = height;
        }

        protected override void BuildProjection(float near, float far, ref Matrix projection)
        {
            Matrix.CreateOrthographic(Width, Height, near, far, out projection);
        }
    }
}
