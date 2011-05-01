using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Processors;
using System.ComponentModel;
using Microsoft.Xna.Framework.Content.Pipeline.Graphics;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace ContentExtensions
{
    [ContentProcessor(DisplayName = "Custom Effect Model Processor")]
    public class CustomEffectModelProcessor : ModelProcessor
    {
        private string _effect = "";
        [DisplayName("Effect")]
        [DefaultValue("")]
        [Description("The shader used to draw this model.")]
        public virtual string Effect
        {
            get { return _effect; }
            set { _effect = value; }
        }

        [DefaultValue(MaterialProcessorDefaultEffect.BasicEffect)]
        public override MaterialProcessorDefaultEffect DefaultEffect
        {
            get { return MaterialProcessorDefaultEffect.BasicEffect; }
            set { }
        }

        public override ModelContent Process(NodeContent input, ContentProcessorContext context)
        {
            ModelContent ret = base.Process(input, context);

            Dictionary<string, object> tag = ret.Tag as Dictionary<string, object>;
            if (tag == null)
            {
                tag = new Dictionary<string, object>();
            }

            ret.Tag = tag;

            return ret;
        }

        protected override MaterialContent ConvertMaterial(MaterialContent input, ContentProcessorContext context)
        {
            OpaqueDataDictionary processorParameters = new OpaqueDataDictionary();
            processorParameters.Add("Effect", Effect);
            processorParameters["ColorKeyColor"] = this.ColorKeyColor;
            processorParameters["ColorKeyEnabled"] = this.ColorKeyEnabled;
            processorParameters["TextureFormat"] = this.TextureFormat;
            processorParameters["GenerateMipmaps"] = this.GenerateMipmaps;
            processorParameters["ResizeTexturesToPowerOfTwo"] = this.ResizeTexturesToPowerOfTwo;

            return context.Convert<MaterialContent, MaterialContent>(input,
                                  "CustomEffectMaterialProcessor", processorParameters);
        }
    }
}
