#define KEYBOARD_CONTROLS

using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace DeferredRenderer
{
    class FirstPersonCamera : PerspectiveCamera
    {
#if KEYBOARD_CONTROLS
        private MouseState _prevMouse;
        private KeyboardState _prevKb;

        private float _moveSpeed;
        public float MoveSpeed
        {
            get { return _moveSpeed; }
            set { _moveSpeed = value; }
        }

        private float _rotateSpeed;
        public float RotateSpeed
        {
            get { return _rotateSpeed; }
            set { _rotateSpeed = value; }
        }
#endif

        private Vector2 _rotation;
        public Vector2 Rotation
        {
            get { return _rotation; }
            set 
            {
                _rotation.X = value.X;
                _rotation.Y = MathHelper.Clamp(value.Y, -MathHelper.PiOver2, MathHelper.PiOver2);

                World = Matrix.CreateFromYawPitchRoll(_rotation.X, _rotation.Y, 0.0f) *
                    Matrix.CreateTranslation(Position);
                Dirty = true;
            }
        }

        public FirstPersonCamera(float near, float far, float fov, float aspect)
            : base(near, far, fov, aspect)
        {
            _rotation = Vector2.Zero;

#if KEYBOARD_CONTROLS
            _moveSpeed = 500f;
            _rotateSpeed = 0.002f;
#endif
        }

        public override void Update(float dt)
        {
#if KEYBOARD_CONTROLS
            KeyboardState kb = Keyboard.GetState();
            MouseState mouse = Mouse.GetState();

            if (mouse.LeftButton == ButtonState.Pressed)
            {
                Vector2 mouseMove = new Vector2(_prevMouse.X - mouse.X, _prevMouse.Y - mouse.Y) * _rotateSpeed;
                Rotation += mouseMove;

                Vector2 moveDir = Vector2.Zero;
                if (kb.IsKeyDown(Keys.W) || kb.IsKeyDown(Keys.Up))
                {
                    moveDir.Y++;
                }
                if (kb.IsKeyDown(Keys.S) || kb.IsKeyDown(Keys.Down))
                {
                    moveDir.Y--;
                }
                if (kb.IsKeyDown(Keys.A) || kb.IsKeyDown(Keys.Left))
                {
                    moveDir.X--;
                }
                if (kb.IsKeyDown(Keys.D) || kb.IsKeyDown(Keys.Right))
                {
                    moveDir.X++;
                }

                Vector3 cameraDP = ((moveDir.Y * World.Forward) + (moveDir.X * World.Right)) *
                    (_moveSpeed * dt);
                Position += cameraDP;

                Mouse.SetPosition(_prevMouse.X, _prevMouse.Y);
                mouse = Mouse.GetState();
            }

            _prevMouse = mouse;
            _prevKb = kb;    
#endif

            base.Update(dt);
        }
    }
}
