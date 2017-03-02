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
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#include <sound/soc.h>

static int gpio_mute_pin = 17;
struct gpio_desc *mute = NULL;
struct max98357a_priv *max98357a;


struct max98357a_priv {
        struct delayed_work enable_sdmode_work;
        struct gpio_desc *sdmode;
        unsigned int sdmode_delay;
};

static void max98357a_enable_sdmode_work(struct work_struct *work)
{
        struct max98357a_priv *max98357a =
        container_of(work, struct max98357a_priv,
                        enable_sdmode_work.work);

        gpiod_set_value(max98357a->sdmode, 0);
}




static int toxcodec_daiops_trigger(struct snd_pcm_substream *substream,
		int cmd, struct snd_soc_dai *dai)
{

	printk(KERN_ERR "toxcodec_daiops_trigger\n");


	if (!max98357a->sdmode)
		return 0;



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
                queue_delayed_work(system_power_efficient_wq,
                                &max98357a->enable_sdmode_work,
                                msecs_to_jiffies(max98357a->sdmode_delay));
		//gpiod_set_value(mute, 0);
		printk(KERN_ERR "Switching SDMODE 0\n");
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
                cancel_delayed_work(&max98357a->enable_sdmode_work);
                gpiod_set_value(max98357a->sdmode, 1);
		printk(KERN_ERR "Switching SDMODE 1\n");
		//gpiod_set_value(mute, 1);
		break;
	}

	return 0;
}








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
        max98357a = devm_kzalloc(&pdev->dev, sizeof(*max98357a), GFP_KERNEL);

        if (!max98357a)
                return -ENOMEM;

         max98357a->sdmode = devm_gpiod_get_optional(&pdev->dev, "mute",GPIOD_OUT_HIGH);
         max98357a->sdmode_delay = 10;
         if (IS_ERR(max98357a->sdmode))
               return PTR_ERR(max98357a->sdmode);


         INIT_DELAYED_WORK(&max98357a->enable_sdmode_work,max98357a_enable_sdmode_work);




	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_toxcodec,
			&toxcodec_dai, 1);
}




static int toxcodec_remove(struct platform_device *pdev)
{
	cancel_delayed_work_sync(&max98357a->enable_sdmode_work);
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
