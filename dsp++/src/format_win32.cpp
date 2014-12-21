#include <dsp++/snd/format.h>
#include <stdexcept>

using namespace dsp::snd;

#ifdef _WIN32
#include <windows.h>
#include <mmreg.h>

void format::render_waveformatex(void* wfx) const {
	WAVEFORMATEX* w = static_cast<WAVEFORMATEX*>(wfx);
	switch (sample_type()) {
	case sample::type::ieee_float:
		w->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		break;
	case sample::type::pcm_unsigned:
		if (sample_bits() != 8) 
			goto noformat;
		w->wFormatTag = WAVE_FORMAT_PCM;
		break;
	case sample::type::pcm_signed:
		if (sample_bits() == 8)
			goto noformat;
		w->wFormatTag = WAVE_FORMAT_PCM;
		break;
	default:
		goto noformat;
	}
	w->nChannels = channel_count();
	w->nSamplesPerSec = sample_rate();
	w->wBitsPerSample = sample_bits();
	w->nBlockAlign = w->nChannels * w->wBitsPerSample / 8;
	w->nAvgBytesPerSec = w->nBlockAlign * w->nSamplesPerSec;
	w->cbSize = 0;
	return;
noformat:
	throw std::runtime_error("dsp::snd::format::render_waveformatex(): no direct WAVEFORMATEX::wFormtTag value for dsp::snd::format::sample_type()");
}

void format::render_waveformatextensible(void* wfx) const {
	WAVEFORMATEXTENSIBLE* w = static_cast<WAVEFORMATEXTENSIBLE*>(wfx);
	render_waveformatex(&w->Format);
	switch (w->Format.wFormatTag) {
	case WAVE_FORMAT_PCM:
		w->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		w->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		break;
	case WAVE_FORMAT_IEEE_FLOAT:
		w->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		w->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		break;
	}
	w->Format.cbSize = 22;
	w->dwChannelMask = channel_config();
	w->Samples.wValidBitsPerSample = w->Format.wBitsPerSample;
}

#endif // _WIN32
