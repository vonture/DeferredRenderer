using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;

namespace DeferredRenderer
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Game1 : Game
    {
        private GraphicsDeviceManager _graphics;
        private Renderer _renderer;
        private FirstPersonCamera _camera;
        
        private ModelInstance _treeModel;
        private List<ModelInstance> _boxModels;
        private ModelInstance _lizardModel;

        private DirectionLight _dLight1;
        private DirectionLight _dLight2;
        private PointLight _pLight1;
        private PointLight _pLight2;

        private HDRPostProcess _hdr;

        private Keys[] _rtKeys = new Keys[] 
        {
            Keys.D1,
            Keys.D2,
            Keys.D3,
            Keys.D4,
            Keys.D5,
            Keys.D6,
            Keys.D7,
            Keys.D8,
            Keys.D9,
            Keys.D0,
        };

        float _time;

        public Game1()
        {
            IsFixedTimeStep = false;

            _graphics = new GraphicsDeviceManager(this);
            _graphics.IsFullScreen = false;
            _graphics.SynchronizeWithVerticalRetrace = false;
            _graphics.PreferredBackBufferWidth = 1360;
            _graphics.PreferredBackBufferHeight = 768;
            _graphics.PreferMultiSampling = false;

            Content.RootDirectory = "Content";
        }

        protected override void Initialize()
        {
            _renderer = new Renderer(GraphicsDevice);

            _camera = new FirstPersonCamera(0.1f, 3000f, 1f, GraphicsDevice.Viewport.AspectRatio);
            _camera.Position = new Vector3(0.0f, 100.0f, 400.0f);

            _dLight1 = new DirectionLight(new Vector3(1, 1, 1f), 1.0f, new Vector3(0.5f, 0.5f, 0.5f));
            _dLight2 = new DirectionLight(new Vector3(0.5f, 1f, 0.5f), 3.0f, new Vector3(-0.5f, 0.5f, 0.1f));
            _pLight1 = new PointLight(new Vector3(1.0f, 0.5f, 0.5f), 10.0f, new Vector3(-150f, 110f, -150), 800f);
            _pLight2 = new PointLight(new Vector3(0.5f, 1.5f, 0.5f), 5.0f, new Vector3(0f, 150f, 150f), 300f);           

            _treeModel = new ModelInstance("tree1");
            _treeModel.Position = new Vector3(0.0f, 100.0f, 0.0f);
            _treeModel.Scale = new Vector3(100f);

            _boxModels = new List<ModelInstance>();
            for (float x = -10; x <= 10; x += 1.5f)
            {
                for (float z = -10; z <= 10; z += 1.5f)
                {
                    float y = (float)Math.Sin(x / 2.0f) * (float)Math.Cos(z / 2.0f) * 200.0f;

                    ModelInstance instance = new ModelInstance("clothbox1");
                    instance.Position = new Vector3(x * 100.0f, y, z * 100.0f);
                    instance.Scale = new Vector3(100f);

                    _boxModels.Add(instance);
                }
            }

            _lizardModel = new ModelInstance("lizard");
            _lizardModel.Scale = new Vector3(5f);
            _lizardModel.Position = new Vector3(-200f, 200f, 300f);

            _hdr = new HDRPostProcess();

            base.Initialize();
        }

        protected override void LoadContent()
        {
            _renderer.LoadContent(GraphicsDevice, Content);

            _treeModel.LoadContent(GraphicsDevice, Content);            
            _lizardModel.LoadContent(GraphicsDevice, Content);
            for (int i = 0; i < _boxModels.Count; i++)
            {
                _boxModels[i].LoadContent(GraphicsDevice, Content);
            }

            _hdr.LoadContent(GraphicsDevice, Content);

            base.LoadContent();
        }

        protected override void UnloadContent()
        {
            _renderer.UnloadContent(GraphicsDevice, Content);

            _treeModel.UnloadContent(GraphicsDevice, Content);
            _lizardModel.UnloadContent(GraphicsDevice, Content);
            for (int i = 0; i < _boxModels.Count; i++)
            {
                _boxModels[i].UnloadContent(GraphicsDevice, Content);
            }

            _hdr.UnloadContent(GraphicsDevice, Content);

            base.UnloadContent();
        }

        protected override void Update(GameTime gameTime)
        {
            float dt = (float)gameTime.ElapsedGameTime.TotalSeconds;

            _camera.Update(dt);
            _time += dt * 0.5f;

            for (int i = 0; i < _boxModels.Count; i++)
            {
                float y = (float)Math.Sin(_boxModels[i].Position.X + _time) *
                    (float)Math.Cos(_boxModels[i].Position.Z + _time) * 100f;

                //_boxModels[i].Position = new Vector3(_boxModels[i].Position.X, y, _boxModels[i].Position.Z);
            }

            KeyboardState kb = Keyboard.GetState();
            for (int i = 0; i < _rtKeys.Length; i++)
            {
                if (kb.IsKeyDown(_rtKeys[i]))
                {
                    _renderer.DisplayChannels = (GBufferCombineChannels)i;
                }
            }

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            _renderer.Begin(_camera);

            //_renderer.DrawModel(_treeModel);

            for (int i = 0; i < _boxModels.Count; i++)
            {
                _renderer.DrawModel(_boxModels[i]);
            }

            _renderer.DrawModel(_lizardModel);

            _renderer.DrawLight(_pLight1, false);
            _renderer.DrawLight(_pLight2, false);

            _renderer.DrawLight(_dLight1, false);
            _renderer.DrawLight(_dLight2, true);

            _renderer.AddPostProcess(_hdr);

            _renderer.End();

            base.Draw(gameTime);
        }
    }
}
