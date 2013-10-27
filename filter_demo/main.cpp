#include <dsp++/snd/reader.h>
#include <dsp++/snd/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/dynamics.h>

#if FILTER_TIME_DOMAIN
#include <dsp++/filter.h>
#elif FILTER_FREQ_DOMAIN
#include <dsp++/overlap_add.h>
#endif

#include <cstdio>
#include <iostream>

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		std::cerr << "usage: filter_demo <input_file> <output_file>\n";
		return EXIT_FAILURE;
	}

	dsp::snd::file_format inf;
	dsp::snd::reader r;
	try {r.open(argv[1], &inf);} catch (std::exception& ex) {
		std::cerr << "unable to open input file " << argv[1] << " " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (inf.channel_count() != 1) {
		std::cerr << "only mono input files are allowed" << std::endl;
		// TODO add support for multichannel files if needed - will have to deinterleave channels, add multiple filter instances and interleave channels on writing
		return EXIT_FAILURE;
	}

	dsp::snd::file_format of = inf;
	std::string outname = argv[2];
	std::string::size_type pos = outname.rfind('.');
	if (std::string::npos != pos) {
		std::string ext(outname, pos + 1);
		if (const char* ft = dsp::snd::file_type::for_extension(ext.c_str()))
			of.set_type(ft);
	}
	dsp::snd::writer w;
	try {w.open(argv[2], &of);} catch (std::exception& ex) {
		std::cerr << "unable to open output file " << argv[2] << " " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}


	dsp::limiter<float> lim;
	static const size_t frame_len = 512;
	float buffer[frame_len];

	static const size_t fir_len = 128;
	double fir[fir_len] = {0};
	// TODO design the filter here...

#if FILTER_TIME_DOMAIN
	dsp::filter<float> flt(fir, fir_len);
#elif FILTER_FREQ_DOMAIN
	dsp::overlap_add<float> flt(frame_len, fir, fir_len);
#endif

	while (true) {
		size_t read = r.read_samples(buffer, frame_len);
#if FILTER_TIME_DOMAIN
		for (size_t i = 0; i < read; ++i) 
			buffer[i] = lim(flt(buffer[i]));
#elif FILTER_FREQ_DOMAIN
		std::copy_n(buffer, frame_len, flt.begin());
		flt();
		std::transform(flt.begin(), flt.end(), buffer, lim);
#endif

		w.write_samples(buffer, read);
		if (read != frame_len)
			break;
	}

	w.close();
	return EXIT_SUCCESS;
}
