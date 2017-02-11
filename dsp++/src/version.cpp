/*!
 * @file version.cpp
 * @brief Implementation of library version checking.
 */
#include <dsp++/version.h>

#define STR(v) #v

const char* dsp::ver::cstr()
{
	return STR(DSP_VERSION_MAJOR) "." STR(DSP_VERSION_MINOR) "." STR(DSP_VERSION_PATCH);
}

unsigned dsp::ver::major() {return DSPXX_VERSION_MAJOR;}
unsigned dsp::ver::minor() {return DSPXX_VERSION_MINOR;}
unsigned dsp::ver::patch() {return DSPXX_VERSION_PATCH;}
unsigned dsp::ver::number() {return DSPXX_VERSION;}
