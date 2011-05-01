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
        private bool _dirty;

        private string _modelPath;

        private Model _model;
        public Model Model
        {
            get { return _model; }
        }

        private BoundingBox _modelBB;
        private Matrix[] _bones;

        private Vector3 _position;
        public Vector3 Position
        {
            get { return _position; }
            set { _position = value; _dirty = true; }
        }

        private Vector3 _scale;
        public Vector3 Scale
        {
            get { return _scale; }
            set { _scale = value; _dirty = true; }
        }

        private Quaternion _rotation;
        public Quaternion Rotation
        {
            get { return _rotation; }
            set { _rotation = value; _dirty = true; }
        }       

        private Matrix _world;
        public Matrix World
        {
            get
            {
                if (_dirty)
                {
                    clean();
                }

                return _world;
            }
        }

        private BoundingBox _boundingBox;
        public BoundingBox BoundingBox
        {
            get
            {
                if (_dirty)
                {
                    clean();
                }

                return _boundingBox;
            }
        }
               
        public ModelInstance(string path)
        {
            _modelPath = path;

            _position = Vector3.Zero;
            _scale = Vector3.One;
            _rotation = Quaternion.Identity;
            _world = Matrix.Identity;
            _boundingBox = new BoundingBox();
        }

        public void LoadContent(GraphicsDevice gd, ContentManager cm)
        {
            _model = cm.Load<Model>(_modelPath);

            _bones = new Matrix[_model.Bones.Count];
            _model.CopyAbsoluteBoneTransformsTo(_bones);

            _modelBB = calculateBoundingBox(_model, _bones);
        } 

        private static BoundingBox calculateBoundingBox(Model model, Matrix[] bones)
        {
            // Create variables to hold min and max xyz values for the model. Initialise them to extremes
            Vector3 modelMax = new Vector3(float.MinValue);
            Vector3 modelMin = new Vector3(float.MaxValue);

            foreach (ModelMesh mesh in model.Meshes)
            {
                //Create variables to hold min and max xyz values for the mesh. Initialise them to extremes
                Vector3 meshMax = new Vector3(float.MinValue);
                Vector3 meshMin = new Vector3(float.MaxValue);

                // There may be multiple parts in a mesh (different materials etc.) so loop through each
                foreach (ModelMeshPart part in mesh.MeshParts)
                {
                    VertexElement[] elements = part.VertexBuffer.VertexDeclaration.GetVertexElements();
                    foreach (VertexElement ve in elements)
                    {
                        if (ve.VertexElementUsage == VertexElementUsage.Position && 
                            ve.VertexElementFormat == VertexElementFormat.Vector3)
                        {
                            // Get verticies
                            Vector3[] verts = new Vector3[part.NumVertices];
                            part.VertexBuffer.GetData<Vector3>(verts);

                            // Transform verticies by the bone transform
                            Vector3.Transform(verts, ref bones[mesh.ParentBone.Index], verts);

                            // Find min and max verticies
                            for (int i = 0; i < verts.Length; i++)
                            {
                                Vector3.Min(ref modelMin, ref verts[i], out modelMin);
                                Vector3.Max(ref modelMax, ref verts[i], out modelMax);
                            }
                        }
                    }                    
                }

                // Expand model extents by the ones from this mesh
                Vector3.Min(ref modelMin, ref meshMin, out modelMin);
                Vector3.Max(ref modelMax, ref meshMax, out modelMax);
            }

            // Create and return the model bounding box
            return new BoundingBox(modelMin, modelMax);
        }

        public void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _bones = null;
            _model = null;
        }

        private void clean()
        {
            _world = Matrix.CreateFromQuaternion(_rotation) *
                     Matrix.CreateScale(_scale) *
                     Matrix.CreateTranslation(_position);

            Vector3 minTransformed = Vector3.Transform(_modelBB.Min, _world);
            Vector3 maxTransformed = Vector3.Transform(_modelBB.Max, _world);

            _boundingBox.Min = Vector3.Min(minTransformed, maxTransformed);
            _boundingBox.Max = Vector3.Max(minTransformed, maxTransformed);
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
