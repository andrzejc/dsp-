/*!
 * @file resample_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include "resample_test.h"
#include <dsp++/resample.h>
#include <dsp++/snd/reader.h>
#include <dsp++/snd/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/float.h>

void dsp::test::resample_test::test_interpolator()
{
	using namespace dsp::snd;

	const size_t factor = 4;
	dsp::snd::file_format ff;
	reader r;
	r.open("data/coil_mono.wav", &ff);

	ff.set_sample_rate(ff.sample_rate() * factor);

	writer w;
	w.open("data/coil_mono_up4.wav", &ff);

	dsp::interpolator<float> interp(factor, 47, .2);

	std::vector<float> in, out;
	const size_t len = 1024;
	in.resize(len);
	out.resize(len * factor);

	while (true) {
		float* x = &in[0];
		float* y = &out[0];
		size_t read = r.read_frames(x, len);
		for (size_t i = 0; i < read; ++i, ++x, y += factor) {
			interp(*x);
			std::copy(interp.begin(), interp.end(), y);
		}
		w.write_frames(&out[0], read * factor);
		if (read != len)
			break;
	}
}

