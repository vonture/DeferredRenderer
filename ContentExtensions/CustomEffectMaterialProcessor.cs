using System;
using System.IO;
using System.ComponentModel;
using System.Collections.Generic;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Processors;
using Microsoft.Xna.Framework.Content.Pipeline.Graphics;

namespace ContentExtensions
{
    /// <summary>
    /// Custom material processor loads specified effect file to use for the model.
    /// </summary>
    [ContentProcessor]
    public class CustomEffectMaterialProcessor : MaterialProcessor
    {
        private string _effect;
        [DisplayName("Effect")]
        [Description("The custom effect applied to the model.")]
        public string Effect
        {
            get { return _effect; }
            set { _effect = value; }
        }

        /// <summary>
        /// Creates new material with the new effect file.
        /// </summary>
        public override MaterialContent Process(MaterialContent input,
                                                ContentProcessorContext context)
        {
            if (string.IsNullOrEmpty(_effect))
                throw new ArgumentException("Custom Effect not set to an effect file");

            // Create a new effect material.
            EffectMaterialContent customMaterial = new EffectMaterialContent();

            // Point the new material at the custom effect file.
            string effectFile = Path.GetFullPath(_effect);
            customMaterial.Effect = new ExternalReference<EffectContent>(effectFile);

            // Loop over the textures in the current material adding them to 
            // the new material.
            foreach (KeyValuePair<string,
                     ExternalReference<TextureContent>> textureContent in input.Textures)
            {
                customMaterial.Textures.Add(textureContent.Key, textureContent.Value);
            }

            // Loop over the opaque data in the current material adding them to 
            // the new material.
            foreach (KeyValuePair<string, Object> opaqueData in input.OpaqueData)
            {
                if (!customMaterial.OpaqueData.ContainsKey(opaqueData.Key))
                {
                    customMaterial.OpaqueData.Add(opaqueData.Key, opaqueData.Value);
                }                
            }

            // Call the base material processor to continue the rest of the processing.
            return base.Process(customMaterial, context);
        }
    }
}