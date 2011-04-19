using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;

namespace DeferredRenderer
{
    public class ModelInstance : IHasContent
    {
        private string _modelPath;

        private Model _model;
        public Model Model
        {
            get { return _model; }
        }

        private Matrix[] _bones;

        private Vector3 _position;
        public Vector3 Position
        {
            get { return _position; }
            set { _position = value; }
        }

        private Vector3 _scale;
        public Vector3 Scale
        {
            get { return _scale; }
            set { _scale = value; }
        }

        private Quaternion _rotation;
        public Quaternion Rotation
        {
            get { return _rotation; }
            set { _rotation = value; }
        }

        public ModelInstance(string path)
        {
            _modelPath = path;

            _position = Vector3.Zero;
            _scale = Vector3.One;
            _rotation = Quaternion.Identity;
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _model = cm.Load<Model>(_modelPath);

            _bones = new Matrix[_model.Bones.Count];
            _model.CopyAbsoluteBoneTransformsTo(_bones);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _bones = null;
            _model = null;
        }

        public Matrix GetWorldMatrix()
        {
            return Matrix.CreateFromQuaternion(_rotation) *
                Matrix.CreateScale(_scale) *
                Matrix.CreateTranslation(_position);
        }

        public Matrix GetBoneMatrix(int idx)
        {
            if (idx < 0 || idx >= _bones.Length)
            {
                throw new ArgumentException("Index out of range.");
            }

            return _bones[idx];
        }
    }
}
