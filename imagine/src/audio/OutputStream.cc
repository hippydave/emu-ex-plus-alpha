/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#define LOGTAG "Audio"
#include <imagine/audio/defs.hh>
#include <imagine/audio/AudioManager.hh>

#if defined __ANDROID__
#include <imagine/audio/opensl/OpenSLESOutputStream.hh>
#include <imagine/audio/android/AAudioOutputStream.hh>
#elif defined __APPLE__
#include <imagine/audio/coreaudio/CAOutputStream.hh>
#else
	#ifdef CONFIG_AUDIO_PULSEAUDIO
	#include <imagine/audio/pulseaudio/PAOutputStream.hh>
	#endif
	#ifdef CONFIG_AUDIO_ALSA
	#include <imagine/audio/alsa/ALSAOutputStream.hh>
	#endif
#endif

namespace IG::Audio
{

std::unique_ptr<OutputStream> makeOutputStream(Api api)
{
	api = makeValidAPI(api);
	switch(api)
	{
		#ifdef CONFIG_AUDIO_PULSEAUDIO
		case Api::PULSEAUDIO: return std::make_unique<PAOutputStream>();
		#endif
		#ifdef CONFIG_AUDIO_ALSA
		case Api::ALSA: return std::make_unique<ALSAOutputStream>();
		#endif
		#ifdef __ANDROID__
		case Api::OPENSL_ES: return std::make_unique<OpenSLESOutputStream>();
		case Api::AAUDIO: return std::make_unique<AAudioOutputStream>();
		#endif
		#ifdef __APPLE__
		case Api::COREAUDIO: return std::make_unique<CAOutputStream>();
		#endif
		default:
			bug_unreachable("audio API should always be valid");
			return nullptr;
	}
}

PcmFormat OutputStreamConfig::format() const
{
	if(format_)
		return format_;
	else
		return AudioManager::nativeFormat();
}

OutputStream::~OutputStream() {}

}
