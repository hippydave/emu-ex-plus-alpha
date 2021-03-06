/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/OptionView.hh>
#include <emuframework/EmuApp.hh>
#include "EmuOptions.hh"
#include "private.hh"

static void setAudioRate(uint32_t rate)
{
	if(rate > optionSoundRate.defaultVal)
		return;
	optionSoundRate = rate;
	EmuSystem::configAudioPlayback(rate);
}

static void setSoundBuffers(int val)
{
	optionSoundBuffers = val;
}

AudioOptionView::AudioOptionView(ViewAttachParams attach, bool customMenu):
	TableView{"Audio Options", attach, item},
	snd
	{
		"Sound",
		(bool)soundIsEnabled(),
		[this](BoolMenuItem &item, Input::Event e)
		{
			setSoundEnabled(item.flipBoolValue(*this));
			if(item.boolValue())
				emuAudio.open(audioOutputAPI());
			else
				emuAudio.close();
		}
	},
	soundDuringFastForward
	{
		"Sound During Fast Forward",
		(bool)soundDuringFastForwardIsEnabled(),
		[this](BoolMenuItem &item, Input::Event e)
		{
			setSoundDuringFastForwardEnabled(item.flipBoolValue(*this));
			emuAudio.setSoundDuringFastForward(item.boolValue());
		}
	},
	soundBuffersItem
	{
		{"2", [this]() { setSoundBuffers(2); }},
		{"3", [this]() { setSoundBuffers(3); }},
		{"4", [this]() { setSoundBuffers(4); }},
		{"5", [this]() { setSoundBuffers(5); }},
		{"6", [this]() { setSoundBuffers(6); }},
		{"7", [this]() { setSoundBuffers(7); }},
		{"8", [this]() { setSoundBuffers(8); }},
	},
	soundBuffers
	{
		"Buffer Size In Frames",
		(int)optionSoundBuffers - 2,
		[this](const MultiChoiceMenuItem &) -> int
		{
			return std::size(soundBuffersItem);
		},
		[this](const MultiChoiceMenuItem &, uint idx) -> TextMenuItem&
		{
			return soundBuffersItem[idx];
		}
	},
	addSoundBuffersOnUnderrun
	{
		"Auto-increase Buffer Size",
		(bool)optionAddSoundBuffersOnUnderrun,
		[this](BoolMenuItem &item, Input::Event e)
		{
			optionAddSoundBuffersOnUnderrun = item.flipBoolValue(*this);
			emuAudio.setAddSoundBuffersOnUnderrun(optionAddSoundBuffersOnUnderrun);
		}
	},
	audioRate
	{
		"Sound Rate",
		0,
		audioRateItem
	}
	#ifdef CONFIG_AUDIO_MANAGER_SOLO_MIX
	,audioSoloMix
	{
		"Mix With Other Apps",
		!optionAudioSoloMix,
		[this](BoolMenuItem &item, Input::Event e)
		{
			optionAudioSoloMix = !item.flipBoolValue(*this);
		}
	}
	#endif
	#ifdef CONFIG_AUDIO_MULTIPLE_SYSTEM_APIS
	,api
	{
		"Audio Driver",
		0,
		apiItem
	}
	#endif
{
	#ifdef CONFIG_AUDIO_MULTIPLE_SYSTEM_APIS
	apiItem.emplace_back("Auto",
		[this](TextMenuItem &, View &parent, Input::Event)
		{
			optionAudioAPI = 0;
			auto defaultApi = IG::Audio::makeValidAPI();
			emuAudio.open(defaultApi);
			api.setSelected(idxOfAPI(defaultApi));
			parent.dismiss();
			return false;
		});
	for(auto desc: IG::Audio::audioAPIs())
	{
		apiItem.emplace_back(desc.name,
			[this, api = desc.api]()
			{
				optionAudioAPI = (uint8_t)api;
				emuAudio.open(api);
			});
	}
	#endif
	if(!customMenu)
	{
		loadStockItems();
	}
}

void AudioOptionView::loadStockItems()
{
	item.emplace_back(&snd);
	item.emplace_back(&soundDuringFastForward);
	if(!optionSoundRate.isConst)
	{
		audioRateItem.clear();
		audioRateItem.emplace_back("Device Native",
			[this](TextMenuItem &, View &parent, Input::Event)
			{
				setAudioRate(optionSoundRate.defaultVal);
				updateAudioRateItem();
				parent.dismiss();
				return false;
			});
		audioRateItem.emplace_back("22KHz", [this]() { setAudioRate(22050); });
		audioRateItem.emplace_back("32KHz", [this]() { setAudioRate(32000); });
		audioRateItem.emplace_back("44KHz", [this]() { setAudioRate(44100); });
		if(optionSoundRate.defaultVal >= 48000)
			audioRateItem.emplace_back("48KHz", [this]() { setAudioRate(48000); });
		item.emplace_back(&audioRate);
		updateAudioRateItem();
	}
	item.emplace_back(&soundBuffers);
	item.emplace_back(&addSoundBuffersOnUnderrun);
	#ifdef CONFIG_AUDIO_MANAGER_SOLO_MIX
	item.emplace_back(&audioSoloMix);
	#endif
	#ifdef CONFIG_AUDIO_MULTIPLE_SYSTEM_APIS
	item.emplace_back(&api);
	api.setSelected(idxOfAPI(IG::Audio::makeValidAPI(audioOutputAPI())));
	#endif
}

void AudioOptionView::updateAudioRateItem()
{
	switch(optionSoundRate)
	{
		bcase 22050: audioRate.setSelected(1);
		bcase 32000: audioRate.setSelected(2);
		bdefault: audioRate.setSelected(3); // 44100
		bcase 48000: audioRate.setSelected(4);
	}
}

#ifdef CONFIG_AUDIO_MULTIPLE_SYSTEM_APIS
unsigned AudioOptionView::idxOfAPI(IG::Audio::Api api)
{
	for(unsigned idx = 0; auto desc: IG::Audio::audioAPIs())
	{
		if(desc.api == api)
		{
			assert(idx + 1 < std::size(apiItem));
			return idx + 1;
		}
		idx++;
	}
	return 0;
}
#endif
