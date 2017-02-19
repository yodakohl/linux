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
};

static struct snd_soc_codec_driver soc_codec_dev_toxcodec;

static int toxcodec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_toxcodec,
			&toxcodec_dai, 1);
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
