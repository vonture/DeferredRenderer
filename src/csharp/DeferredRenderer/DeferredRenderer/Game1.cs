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
            _dLight2 = new DirectionLight(new Vector3(0.8f, 1f, 0.8f), 1.0f, new Vector3(-0.5f, 0.5f, 0.1f));
            _pLight1 = new PointLight(new Vector3(1.0f, 0.5f, 0.5f), 1.0f, new Vector3(0, 200, 0), 500f);
            _pLight2 = new PointLight(new Vector3(0.5f, 1.5f, 0.5f), 2.0f, new Vector3(0f, 150f, 150f), 300f);           

            _treeModel = new ModelInstance("tree1");
            _treeModel.Position = new Vector3(0.0f, 100.0f, 0.0f);
            _treeModel.Scale = new Vector3(100f);

            _boxModels = new List<ModelInstance>();

            // Floor box
            ModelInstance floor = new ModelInstance("clothbox1");
            floor.Scale = new Vector3(2000f, 100f, 2000f);
            floor.Position = new Vector3(0f, 0f, 0f);
            _boxModels.Add(floor);

            ModelInstance leftWall = new ModelInstance("clothbox1");
            leftWall.Scale = new Vector3(50f, 200f, 400f);
            leftWall.Position = new Vector3(200f, 150f, 0);
            _boxModels.Add(leftWall);

            ModelInstance rightWall = new ModelInstance("clothbox1");
            rightWall.Scale = new Vector3(50f, 200f, 400f);
            rightWall.Position = new Vector3(-200f, 150f, 0);
            _boxModels.Add(rightWall);

            ModelInstance topWall = new ModelInstance("clothbox1");
            topWall.Scale = new Vector3(400f, 200f, 50);
            topWall.Position = new Vector3(0, 150f, -200f);
            _boxModels.Add(topWall);

            ModelInstance bottomWall = new ModelInstance("clothbox1");
            bottomWall.Scale = new Vector3(400f, 200f, 50);
            bottomWall.Position = new Vector3(0, 150f, 200f);
            _boxModels.Add(bottomWall);

            ModelInstance roof = new ModelInstance("clothbox1");
            roof.Scale = new Vector3(400f, 50f, 400);
            roof.Position = new Vector3(0, 250f, 0f);
            _boxModels.Add(roof);

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
            //_renderer.DrawLight(_pLight2, false);

            //_renderer.DrawLight(_dLight1, false);
            _renderer.DrawLight(_dLight2, true);

            _renderer.AddPostProcess(_hdr);

            _renderer.End();

            base.Draw(gameTime);
        }
    }
}
