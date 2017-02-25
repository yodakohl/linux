/*
 * Driver for the toxcodec codec
 *
 * Author:	Florian Meier <florian.meier@koalo.de>
 *		Copyright 2013
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/soc.h>


static int toxcodec_daiops_trigger(struct snd_pcm_substream *substream,
		int cmd, struct snd_soc_dai *dai)
{
 
	struct gpio_desc *sdmode = snd_soc_dai_get_drvdata(dai);
	printk(KERN_ERR "toxcodec_daiops_trigger\n");

	if (!sdmode){
		printk(KERN_ERR "no sd mode\n");
		return 0;
	}


	//Only for playback direction
	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK){
		printk(KERN_ERR "Direction is Capture\n");
		return 0;
	} else {
		printk(KERN_ERR "Direction is Playback\n");
	}

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		gpiod_set_value(sdmode, 1);
		printk(KERN_ERR "Switching SDMODE 1\n");
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		printk(KERN_ERR "Switching SDMODE 0\n");
		gpiod_set_value(sdmode, 0);
		break;
	}

	return 0;
}

static struct snd_soc_codec_driver toxcodec_codec_driver = {
	.probe				= toxcodec_codec_probe,
	//.dapm_widgets		= max98357a_dapm_widgets,
	//.num_dapm_widgets	= ARRAY_SIZE(max98357a_dapm_widgets),
	//.dapm_routes		= max98357a_dapm_routes,
	//.num_dapm_routes	= ARRAY_SIZE(max98357a_dapm_routes),
};


static const struct snd_soc_dai_ops toxcodec_dai_ops = {
	.trigger	= toxcodec_daiops_trigger,
};


static struct snd_soc_dai_driver toxcodec_dai = {
	.name = "toxcodec-hifi",
	.playback = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE
	},
	.capture = {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE
	 },

	.ops = &toxcodec_dai_ops,
};

static struct snd_soc_codec_driver soc_codec_dev_toxcodec;

static int toxcodec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_toxcodec,
			&toxcodec_dai, 1);
}


static int toxcodec_codec_probe(struct snd_soc_codec *codec)
{
	struct gpio_desc *sdmode;

	sdmode = devm_gpiod_get_optional(codec->dev, "sdmode", GPIOD_OUT_LOW);
	if (IS_ERR(sdmode))
		return PTR_ERR(sdmode);

	snd_soc_codec_set_drvdata(codec, sdmode);

	return 0;
}



static int toxcodec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static const struct of_device_id toxcodec_of_match[] = {
	{ .compatible = "tox,toxcodec", },
	{ }
};
MODULE_DEVICE_TABLE(of, toxcodec_of_match);

static struct platform_driver toxcodec_codec_driver = {
	.probe 		= toxcodec_probe,
	.remove 	= toxcodec_remove,
	.driver		= {
		.name	= "toxcodec-codec",
		.owner	= THIS_MODULE,
		.of_match_table = toxcodec_of_match,
	},
};

module_platform_driver(toxcodec_codec_driver);

MODULE_DESCRIPTION("ASoC toxcodec codec driver");
MODULE_AUTHOR("Florian Meier <florian.meier@koalo.de>");
MODULE_LICENSE("GPL v2");
