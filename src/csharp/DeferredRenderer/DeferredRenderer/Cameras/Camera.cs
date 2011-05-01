using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System.Text;

namespace DeferredRenderer
{
    abstract public class Camera : IUpdateable
    {
        private Matrix _world;
        public Matrix World
        {
            get { return _world; }
            set { _world = value; _dirty = true; }
        }

        public Vector3 Position
        {
            get { return _world.Translation; }
            set { _world.Translation = value; _dirty = true; }
        }

        public Quaternion Orientation
        {
            get { return Quaternion.CreateFromRotationMatrix(_world); }
            set
            {                
                Vector3 oldPos = _world.Translation;
                _world = Matrix.CreateFromQuaternion(value);
                _world.Translation = oldPos;
                _dirty = true;
            }
        }

        private Matrix _view;
        public Matrix View
        {
            get
            {
                if (_dirty)
                {
                    updateValues();
                    _dirty = false;
                }
                return _view;
            }
            private set
            {
                _view = value;
            }
        }

        private Matrix _projection;
        public Matrix Projection
        {
            get
            {
                if (_dirty)
                {
                    updateValues();
                    _dirty = false;
                }
                return _projection;
            }
            private set
            {
                _projection = value;
            }
        }

        private BoundingFrustum _boundingFrustum;
        public BoundingFrustum BoundingFrustum
        {
            get
            {
                if (_dirty)
                {
                    updateValues();
                    _dirty = false;
                }
                return _boundingFrustum;
            }
            private set 
            { 
                _boundingFrustum = value;
            }
        }

        private float _nearClip;
        public float NearClip
        {
            get { return _nearClip; }
            set { _dirty = true;  _nearClip = value; }
        }

        private float _farClip;
        public float FarClip
        {
            get { return _farClip; }
            set { _dirty = true;  _farClip = value; }
        }

        private bool _dirty;
        protected bool Dirty
        {
            get { return _dirty; }
            set { _dirty = value; }
        }

        public Camera(float near, float far)
        {
            World = Matrix.Identity;
            View = Matrix.Identity;
            Projection = Matrix.Identity;
            BoundingFrustum = new BoundingFrustum(Matrix.Identity);

            NearClip = near;
            FarClip = far;

            _dirty = true;
        }

        private void updateValues()
        {
            Matrix.Invert(ref _world, out _view);
            BuildProjection(NearClip, FarClip, ref _projection);

            Matrix vp;
            Matrix.Multiply(ref _view, ref _projection, out vp);
            _boundingFrustum.Matrix = vp;
        }

        protected abstract void BuildProjection(float near, float far, ref Matrix projection);
        
        public Matrix BuildProjection(float near, float far)
        {
            Matrix proj = new Matrix();
            BuildProjection(near, far, ref proj);
            return proj;
        }

        public virtual void Update(float dt)
        {
        }
    }
}
