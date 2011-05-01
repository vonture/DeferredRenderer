using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;

namespace DeferredRenderer
{
    public class HDRPostProcess : PostProcess
    {
        private RenderTarget2D[] _luminanceRTs;
        private RenderTarget2D _brightPassRT;
        private RenderTarget2D _blurredBrightPassRT1;
        private RenderTarget2D _blurredBrightPassRT2;

        private Effect _blurEffect;
        private Effect _brightPassEffect;
        private Effect _luminanceEffect;
        private Effect _downSampleEffect;
        private Effect _combineEffect;

        private FullScreenQuad _fsQuad;

        private float _avgLuminance;
        private float _maxLuminance;

        private float _luminanceThreshold;
        public float LuminanceThreshold
        {
            get { return _luminanceThreshold; }
            set { _luminanceThreshold = Math.Max(value, 0.0f); }
        }
        
        private float _gaussianMultiplier;
        public float GaussianMultiplier
        {
            get { return _gaussianMultiplier; }
            set { _gaussianMultiplier = Math.Max(value, 0.0f); }
        }

        private float _gaussianMean;
        public float GaussianMean
        {
            get { return _gaussianMean; }
            set { _gaussianMean = MathHelper.Clamp(value, -1.0f, 1.0f); }
        }
        
        private float _gaussianStdDev;
        public float GaussianStdDev
        {
            get { return _gaussianStdDev; }
            set { _gaussianStdDev = Math.Max(value, 1.0f); }
        }

        private float _exposure;
        public float Exposure
        {
            get { return _exposure; }
            set { _exposure = Math.Max(value, 0.0f); }
        }

        public HDRPostProcess()
        {
            _fsQuad = new FullScreenQuad();
            LuminanceThreshold = 0.7f;
            Exposure = 0.5f;
            GaussianMean = 0.0f;
            GaussianStdDev = 0.8f;
            GaussianMultiplier = 0.4f;
        }

        public override void LoadContent(GraphicsDevice gd, ContentManager cm) 
        {
            _blurEffect = cm.Load<Effect>("GaussianBlur");
            _luminanceEffect = cm.Load<Effect>("Luminance");
            _downSampleEffect = cm.Load<Effect>("DownSample");
            _brightPassEffect = cm.Load<Effect>("BrightPass");
            _combineEffect = cm.Load<Effect>("HDRCombine");

            List<RenderTarget2D> lumTex = new List<RenderTarget2D>();
            int texSize = 1;
            while (texSize < Math.Min(gd.Viewport.Width, gd.Viewport.Height))
            {
                lumTex.Add(new RenderTarget2D(gd, texSize, texSize, false, SurfaceFormat.Vector2, 
                    DepthFormat.None));
                texSize *= 3;
            }

            //lumTex.RemoveRange(0, lumTex.Count - 2);

            lumTex.Reverse();
            _luminanceRTs = lumTex.ToArray();

            _brightPassRT = new RenderTarget2D(gd, gd.Viewport.Width / 2, gd.Viewport.Height / 2, false,
                SurfaceFormat.HalfVector4, DepthFormat.None);
            _blurredBrightPassRT1 = new RenderTarget2D(gd, gd.Viewport.Width / 8, gd.Viewport.Height / 8, false,
                SurfaceFormat.HalfVector4, DepthFormat.None);
            _blurredBrightPassRT2 = new RenderTarget2D(gd, gd.Viewport.Width / 8, gd.Viewport.Height / 8, false,
                SurfaceFormat.HalfVector4, DepthFormat.None);

            _fsQuad.LoadContent(gd, cm);

            base.LoadContent(gd, cm);
        }

        public override void UnloadContent(GraphicsDevice gd, ContentManager cm)
        {
            _blurEffect.Dispose();
            _blurEffect = null;

            _luminanceEffect.Dispose();
            _luminanceEffect = null;

            _downSampleEffect.Dispose();
            _downSampleEffect = null;

            _brightPassEffect.Dispose();
            _brightPassEffect = null;

            _combineEffect.Dispose();
            _combineEffect = null;

            _blurredBrightPassRT1.Dispose();
            _blurredBrightPassRT1 = null;

            _blurredBrightPassRT2.Dispose();
            _blurredBrightPassRT2 = null;
            
            for (int i = 0; i < _luminanceRTs.Length; i++)
            {
                _luminanceRTs[i].Dispose();
                _luminanceRTs[i] = null;
            }

            _brightPassRT.Dispose();
            _brightPassRT = null;

            _fsQuad.UnloadContent(gd, cm);

            base.UnloadContent(gd, cm);
        }

        public override void Render(GraphicsDevice gd, RenderTarget2D source,
            RenderTarget2D destination, GBuffer gBuffer, LightBuffer lightBuffer)
        {
            measureLuminance(gd, source);
            brightPass(gd, source);
            combine(gd, source, destination);
        }

        private void combine(GraphicsDevice gd, RenderTarget2D source, RenderTarget2D destination)
        {
            gd.SetRenderTarget(destination);

            _combineEffect.Parameters["SceneTexture"].SetValue(source);           
            _combineEffect.Parameters["BrightPassTexture"].SetValue(_blurredBrightPassRT1);
            _combineEffect.Parameters["AverageLuminance"].SetValue(_avgLuminance);
            _combineEffect.Parameters["MaximumLuminance"].SetValue(_maxLuminance);
            _combineEffect.Parameters["InverseBrightPassSize"].SetValue(
                new Vector2(1.0f / _blurredBrightPassRT1.Width, 1.0f / _blurredBrightPassRT1.Height));
            _combineEffect.Parameters["Exposure"].SetValue(Exposure);
            _combineEffect.Parameters["GaussianScalar"].SetValue(GaussianMultiplier);
            _combineEffect.Parameters["TargetSize"].SetValue(new Vector2(gd.Viewport.Width, gd.Viewport.Height));

            _fsQuad.Draw(gd, _combineEffect);
        }

        private void brightPass(GraphicsDevice gd, RenderTarget2D source)
        {
            // Render the bright pass to a 1/2 size RT
            gd.SetRenderTarget(_brightPassRT);

            Vector2[] lumValues = new Vector2[1];
            _luminanceRTs[_luminanceRTs.Length - 1].GetData(lumValues);
            _avgLuminance = lumValues[0].X;
            _maxLuminance = lumValues[0].Y;

            Vector2 pixSrc = new Vector2(1.0f / source.Width, 1.0f / source.Height);
            Vector2 pixBright = new Vector2(1.0f / _brightPassRT.Width, 1.0f / _brightPassRT.Height);
            Vector2 halfPixDS = new Vector2(0.5f / _blurredBrightPassRT1.Width, 0.5f / _blurredBrightPassRT1.Height);

            Vector2[] brightPassOffsets = new Vector2[5];
            brightPassOffsets[0] = new Vector2(0.0f, 0.0f);
            brightPassOffsets[1] = new Vector2(-pixSrc.X, pixSrc.Y);
            brightPassOffsets[2] = new Vector2(pixSrc.X, pixSrc.Y);
            brightPassOffsets[3] = new Vector2(-pixSrc.X, -pixSrc.Y);
            brightPassOffsets[4] = new Vector2(pixSrc.X, -pixSrc.Y);

            _brightPassEffect.CurrentTechnique = _brightPassEffect.Techniques["BrightPass"];
            _brightPassEffect.Parameters["SceneTexture"].SetValue(source);
            _brightPassEffect.Parameters["LuminanceThreshold"].SetValue(LuminanceThreshold);
            _brightPassEffect.Parameters["LuminanceOffsets"].SetValue(brightPassOffsets);
            _brightPassEffect.Parameters["SceneSize"].SetValue(new Vector2(source.Width, source.Height));
            _brightPassEffect.Parameters["TargetSize"].SetValue(new Vector2(_brightPassRT.Width, _brightPassRT.Height));

            _fsQuad.Draw(gd, _brightPassEffect);
            
            // Down sample the bright pass
            Vector2[] dsOffsets = new Vector2[5];
            dsOffsets[0] = new Vector2(0.0f, 0.0f);
            dsOffsets[1] = new Vector2(-pixBright.X, pixBright.Y);
            dsOffsets[2] = new Vector2(pixBright.X, pixBright.Y);
            dsOffsets[3] = new Vector2(-pixBright.X, -pixBright.Y);
            dsOffsets[4] = new Vector2(pixBright.X, -pixBright.Y);

            gd.SetRenderTarget(_blurredBrightPassRT1);

            _downSampleEffect.CurrentTechnique = _downSampleEffect.Techniques["DownSample"];
            _downSampleEffect.Parameters["DownSampleOffsets"].SetValue(dsOffsets);
            _downSampleEffect.Parameters["SceneTexture"].SetValue(_brightPassRT);
            _downSampleEffect.Parameters["SceneSize"].SetValue(new Vector2(_brightPassRT.Width, _brightPassRT.Height));
            _downSampleEffect.Parameters["TargetSize"].SetValue(new Vector2(_blurredBrightPassRT1.Width, _blurredBrightPassRT1.Height));
            _fsQuad.Draw(gd, _downSampleEffect);
            
            // Blur horizontally
            Vector2[] horizontalOffsets = new Vector2[9];
            Vector2[] verticalOffsets = new Vector2[9];
            float[] weights = new float[9];
            for (int i = 0; i < 9; i++)
            {
                // Compute the offsets. We take 9 samples - 4 either side and one in the middle:
                //     i =  0,  1,  2,  3, 4,  5,  6,  7,  8
                //Offset = -4, -3, -2, -1, 0, +1, +2, +3, +4
                horizontalOffsets[i] = new Vector2((i - 4.0f) * (1.0f / _blurredBrightPassRT1.Width), 0.0f);
                verticalOffsets[i] = new Vector2(0.0f, (i - 4.0f) * (1.0f / _blurredBrightPassRT1.Height));

                // 'x' is just a simple alias to map the [0,8] range down to a [-1,+1]
                float x = (i - 4.0f) / 4.0f;

                // Use a gaussian distribution. Changing the standard-deviation
                // (second parameter) as well as the amplitude (multiplier) gives
                // distinctly different results.
                weights[i] = GaussianMultiplier * computeGaussianValue(x, GaussianMean, GaussianStdDev);
            }

            gd.SetRenderTarget(_blurredBrightPassRT2);

            _blurEffect.CurrentTechnique = _blurEffect.Techniques["Blur"];
            _blurEffect.Parameters["SceneTexture"].SetValue(_blurredBrightPassRT1);
            _blurEffect.Parameters["Offsets"].SetValue(horizontalOffsets);
            _blurEffect.Parameters["Weights"].SetValue(weights);
            _blurEffect.Parameters["SceneSize"].SetValue(new Vector2(_blurredBrightPassRT1.Width, _blurredBrightPassRT1.Height));
            _blurEffect.Parameters["TargetSize"].SetValue(new Vector2(_blurredBrightPassRT2.Width, _blurredBrightPassRT2.Height));
            _fsQuad.Draw(gd, _blurEffect);

            // Blur vertically
            gd.SetRenderTarget(_blurredBrightPassRT1);

            _blurEffect.CurrentTechnique = _blurEffect.Techniques["Blur"];
            _blurEffect.Parameters["SceneTexture"].SetValue(_blurredBrightPassRT2);
            _blurEffect.Parameters["Offsets"].SetValue(verticalOffsets);
            _blurEffect.Parameters["Weights"].SetValue(weights);
            _blurEffect.Parameters["SceneSize"].SetValue(new Vector2(_blurredBrightPassRT2.Width, _blurredBrightPassRT2.Height));
            _blurEffect.Parameters["TargetSize"].SetValue(new Vector2(_blurredBrightPassRT1.Width, _blurredBrightPassRT1.Height));
            _fsQuad.Draw(gd, _blurEffect);          
        }

        float computeGaussianValue(float x, float mean, float std_deviation)
        {
            // The gaussian equation is defined as such:
            /*    
              -(x - mean)^2
              -------------
              1.0               2*std_dev^2
              f(x,mean,std_dev) = -------------------- * e^
              sqrt(2*pi*std_dev^2)
      
             */

            return (1.0f / (float)Math.Sqrt(2.0f * MathHelper.Pi * std_deviation * std_deviation)) *
                (float)Math.Exp((-((x - mean) * (x - mean))) / (2.0f * std_deviation * std_deviation));
        }

        private void measureLuminance(GraphicsDevice gd, RenderTarget2D source)
        {
            // Render first pass from color to greyscale
            RenderTarget2D firstPassRT = _luminanceRTs[0];

            gd.SetRenderTarget(firstPassRT);

            Vector2 srcSize = new Vector2(source.Width, source.Height);
            Vector2 dstSize = new Vector2(firstPassRT.Width, firstPassRT.Height);

            Vector2[] dsOffsets = new Vector2[9];
            int idx = 0;
            for (int x = -1; x < 2; x++)
            {
                for (int y = -1; y < 2; y++)
                {
                    dsOffsets[idx] = new Vector2(x / (float)srcSize.X, y / (float)srcSize.Y);
                    idx++;
                }
            }

            _luminanceEffect.CurrentTechnique = _luminanceEffect.Techniques["GreyScaleDownSample"];
            _luminanceEffect.Parameters["SceneTexture"].SetValue(source);
            _luminanceEffect.Parameters["DownSampleOffsets"].SetValue(dsOffsets);
            _luminanceEffect.Parameters["SceneSize"].SetValue(srcSize);
            _luminanceEffect.Parameters["TargetSize"].SetValue(dstSize);
            _fsQuad.Draw(gd, _luminanceEffect);

            // Downscale to the 1x1 texture
            for (int i = 1; i < _luminanceRTs.Length; i++)
            {
                RenderTarget2D src = _luminanceRTs[i - 1];
                RenderTarget2D dst = _luminanceRTs[i];

                srcSize = new Vector2(src.Width, src.Height);
                dstSize = new Vector2(dst.Width, dst.Height);

                idx = 0;
                for (int x = -1; x < 2; x++)
                {
                    for (int y = -1; y < 2; y++)
                    {
                        dsOffsets[idx] = new Vector2(x / (float)src.Width, y / (float)src.Height);
                        idx++;
                    }
                }

                gd.SetRenderTarget(dst);

                _luminanceEffect.CurrentTechnique = _luminanceEffect.Techniques["DownSample"];
                _luminanceEffect.Parameters["SceneTexture"].SetValue(src);
                _luminanceEffect.Parameters["DownSampleOffsets"].SetValue(dsOffsets);
                _luminanceEffect.Parameters["SceneSize"].SetValue(srcSize);
                _luminanceEffect.Parameters["TargetSize"].SetValue(dstSize);
                _fsQuad.Draw(gd, _luminanceEffect);
            }
        }
    }
}
