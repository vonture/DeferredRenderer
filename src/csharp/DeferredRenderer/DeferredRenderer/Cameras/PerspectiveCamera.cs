using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using System.Text;

namespace DeferredRenderer
{
    public class PerspectiveCamera : Camera
    {
        private float _fov;
        public float FieldOfView
        {
            get
            {
                return _fov;
            }
            set
            {
                if (value > MathHelper.Pi || value <= 0f)
                {
                    throw new ArgumentOutOfRangeException();
                }
                Dirty = true;
                _fov = value;
            }
        }

        private float _aspectRatio;
        public float AspectRatio
        {
            get
            {
                return _aspectRatio;
            }
            set
            {
                _aspectRatio = value;
                Dirty = true;
            }
        }

        public PerspectiveCamera(float near, float far, float fov, float aspect)
            : base(near, far)
        {
            FieldOfView = fov;
            AspectRatio = aspect;
        }

        protected override void BuildProjection(float near, float far, ref Matrix projection)
        {
            Matrix.CreatePerspectiveFieldOfView(FieldOfView, AspectRatio, near, far, out projection);
        }
    }
}
