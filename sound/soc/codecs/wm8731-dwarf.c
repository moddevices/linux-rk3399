/*
 * ASoC Driver for AudioInjector Pi add on soundcard
 *
 *  Created on: 13-May-2016
 *      Author: flatmax@flatmax.org
 *              based on code by  Cliff Cai <Cliff.Cai@analog.com> for the ssm2602 machine blackfin.
 *              with help from Lars-Peter Clausen for simplifying the original code to use the dai_fmt field.
 *		i2s_node code taken from the other sound/soc/bcm machine drivers.
 *
 * Copyright (C) 2016 Flatmax Pty. Ltd.
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

#include <linux/module.h>
#include <linux/types.h>

#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/control.h>

#include "wm8731.h"

static int moddwarf_soundcard_dai_init(struct snd_soc_pcm_runtime *rtd)
{
    // TODO error checking
    // setup clocks
    const int sysclk = 12288000;
    snd_soc_dai_set_sysclk(rtd->cpu_dai, SNDRV_PCM_STREAM_CAPTURE, sysclk, 0);
    snd_soc_dai_set_sysclk(rtd->cpu_dai, SNDRV_PCM_STREAM_PLAYBACK, sysclk, 0);
	snd_soc_dai_set_sysclk(rtd->codec_dai, WM8731_SYSCLK_XTAL, sysclk, SND_SOC_CLOCK_IN);

    // TODO set good defaults
	/* Enable playback and capture */
    // SND_SOC_DAPM_DAC("DAC", "HiFi Playback", WM8731_PWR, 3, 1),
    // SND_SOC_DAPM_ADC("ADC", "HiFi Capture", WM8731_PWR, 2, 1),

    return 0;
}

static struct snd_soc_dai_link moddwarf_soundcard_dai[] = {
	{
		.name = "MOD DWARF",
		.stream_name = "MOD DWARF",
		.cpu_dai_name	= "rockchip-i2s-tdm.0",
		.codec_dai_name = "wm8731-hifi",
		.platform_name	= "rockchip-i2s-tdm.0",
		.codec_name = "wm8731.2-001a",
		.init = moddwarf_soundcard_dai_init,
		.dai_fmt = SND_SOC_DAIFMT_CBS_CFS|SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF,
	},
};

#if 0
static const struct snd_soc_dapm_widget wm8731_dapm_widgets[] = {
// 	SND_SOC_DAPM_HP("Headphone Jack", NULL),
// 	SND_SOC_DAPM_SPK("Ext Spk", NULL),
// 	SND_SOC_DAPM_LINE("Line In Jacks", NULL),
	SND_SOC_DAPM_MIC("Microphone", NULL),
};

static const struct snd_soc_dapm_route audioinjector_audio_map[] = {
	/* headphone connected to LHPOUT, RHPOUT */
// 	{"Headphone Jack", NULL, "LHPOUT"},
// 	{"Headphone Jack", NULL, "RHPOUT"},

	/* speaker connected to LOUT, ROUT */
// 	{"Ext Spk", NULL, "ROUT"},
// 	{"Ext Spk", NULL, "LOUT"},

	/* line inputs */
// 	{"Line In Jacks", NULL, "Line Input"},

	/* mic is connected to Mic Jack, with WM8731 Mic Bias */
	{"Microphone", NULL, "Mic Bias"},
};
#endif

static struct snd_soc_card snd_soc_audioinjector = {
	.name = "MOD DWARF",
	.dai_link = moddwarf_soundcard_dai,
	.num_links = ARRAY_SIZE(moddwarf_soundcard_dai),

#if 0
	.dapm_widgets = wm8731_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(wm8731_dapm_widgets),
	.dapm_routes = audioinjector_audio_map,
	.num_dapm_routes = ARRAY_SIZE(audioinjector_audio_map),
#endif
};

static int moddwarf_soundcard_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_audioinjector;
	int ret;
	
	card->dev = &pdev->dev;

	if (pdev->dev.of_node) {
		struct snd_soc_dai_link *dai = &moddwarf_soundcard_dai[0];
		struct device_node *i2s_node = of_parse_phandle(pdev->dev.of_node,
								"i2s-controller", 0);

		if (i2s_node) {
			dai->cpu_dai_name = NULL;
			dai->cpu_of_node = i2s_node;
			dai->platform_name = NULL;
			dai->platform_of_node = i2s_node;
		} else
			if (!dai->cpu_of_node) {
				dev_err(&pdev->dev, "Property 'i2s-controller' missing or invalid\n");
				return -EINVAL;
			}
	}

	if ((ret = snd_soc_register_card(card))) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);
	}
	return ret;
}

static int moddwarf_soundcard_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	return snd_soc_unregister_card(card);

}

static const struct of_device_id moddwarf_soundcard_of_match[] = {
	{ .compatible = "moddwarf-soundcard", },
	{},
};
MODULE_DEVICE_TABLE(of, moddwarf_soundcard_of_match);

static struct platform_driver moddwarf_soundcard_driver = {
       .driver         = {
		.name   = "moddwarf-soundcard",
		.owner  = THIS_MODULE,
		.of_match_table = moddwarf_soundcard_of_match,
       },
       .probe          = moddwarf_soundcard_probe,
       .remove         = moddwarf_soundcard_remove,
};

module_platform_driver(moddwarf_soundcard_driver);
MODULE_AUTHOR("Matt Flax <flatmax@flatmax.org>");
MODULE_DESCRIPTION("AudioInjector.net Pi Soundcard");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:audioinjector-pi-soundcard");
