using System;
using System.Collections.Generic;
using System.Linq;
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
        private ModelInstance _boxModel;
        private ModelInstance _lizardModel;

        private DirectionLight _dLight1;
        private DirectionLight _dLight2;
        private PointLight _pLight1;
        private PointLight _pLight2;

        public Game1()
        {
            IsFixedTimeStep = false;

            _graphics = new GraphicsDeviceManager(this);
            _graphics.IsFullScreen = false;
            _graphics.SynchronizeWithVerticalRetrace = false;
            _graphics.PreferredBackBufferWidth = 1280;
            _graphics.PreferredBackBufferHeight = 720;
            _graphics.PreferMultiSampling = false;

            Content.RootDirectory = "Content";
        }

        protected override void Initialize()
        {
            _renderer = new Renderer(GraphicsDevice);

            _camera = new FirstPersonCamera(0.1f, 1500f, 1f, GraphicsDevice.Viewport.AspectRatio);
            _camera.Position = new Vector3(0.0f, 100.0f, 400.0f);

            _dLight1 = new DirectionLight(new Vector3(1, 1, 0.3f), 1.0f, new Vector3(0.5f, 0.5f, 0.5f));
            _dLight2 = new DirectionLight(new Vector3(1, 1f, 1f), 1.0f, new Vector3(-0.5f, 0.5f, 0.1f));
            _pLight1 = new PointLight(Vector3.UnitX, 1.0f, new Vector3(-150f, 110f, -150), 350f);
            _pLight2 = new PointLight(Vector3.UnitY, 1.0f, new Vector3(0f, 150f, 150f), 300f);           

            _treeModel = new ModelInstance("tree1");
            _treeModel.Position = new Vector3(0.0f, 100.0f, 0.0f);
            
            _boxModel = new ModelInstance("clothbox1");

            _lizardModel = new ModelInstance("lizard");
            _lizardModel.Scale = new Vector3(5f);
            _lizardModel.Position = new Vector3(100f, 0f, 100f);

            base.Initialize();
        }

        protected override void LoadContent()
        {
            _renderer.LoadContent(GraphicsDevice, Content);

            _treeModel.LoadContent(GraphicsDevice, Content);
            _boxModel.LoadContent(GraphicsDevice, Content);
            _lizardModel.LoadContent(GraphicsDevice, Content);
        }

        protected override void UnloadContent()
        {
            _renderer.UnloadContent(GraphicsDevice, Content);

            _treeModel.UnloadContent(GraphicsDevice, Content);
            _boxModel.UnloadContent(GraphicsDevice, Content);
            _lizardModel.UnloadContent(GraphicsDevice, Content);
        }

        protected override void Update(GameTime gameTime)
        {
            float dt = (float)gameTime.ElapsedGameTime.TotalSeconds;

            _camera.Update(dt);

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            _renderer.Begin(_camera);

            _renderer.DrawModel(_treeModel);
            _renderer.DrawModel(_boxModel);
            _renderer.DrawModel(_lizardModel);

            _renderer.DrawLight(_pLight1);
            _renderer.DrawLight(_pLight2);

            _renderer.DrawLight(_dLight1);
            _renderer.DrawLight(_dLight2);

            _renderer.End();

            base.Draw(gameTime);
        }
    }
}
