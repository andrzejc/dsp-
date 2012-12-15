#include <dsp++/snd/reader.h>
#include <dsp++/snd/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/dynamics.h>

#include <cstdio>
#include <iostream>

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		std::cerr << "usage: compressor_demo <input_file> <output_file>\n";
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

	//dsp::snd::writer d;
	//d.open("debug.wav", &of);

	const float rms_period_ms = 40; 
	const float attack_ms = 15; 
	const float release_ms = 60;
	const float threshold_dB = -20.f;
	const float gain_dB = 7.f;
	const float ratio = 3.f;

	dsp::compressor<float> comp(inf.time_ms_to_samples(rms_period_ms));
	comp.set_attack(inf.time_ms_to_samples(attack_ms));
	comp.set_release(inf.time_ms_to_samples(release_ms));
	comp.set_threshold_dB(threshold_dB);
	comp.set_gain_dB(gain_dB);
	comp.set_ratio(ratio);
	comp.set_limiter(true);

	const size_t buf_size = 512;
	float buffer[buf_size];
	//float dbuf[buf_size];
	while (true) {
		size_t read = r.read_samples(buffer, buf_size);
		//float comp_dB;
		for (size_t i = 0; i < read; ++i) {
			buffer[i] = comp(buffer[i]/*, &comp_dB*/);
			//dbuf[i] = 0.5f * std::pow(10.f, comp_dB / 20.f);
		}

		w.write_samples(buffer, read);
		//d.write_samples(dbuf, read);
		if (read != buf_size)
			break;
	}
	w.close();
	//d.close();
	return EXIT_SUCCESS;
}