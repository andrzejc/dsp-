/*!
 * @file loudness_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <dsp++/snd/loudness.h>
#include <dsp++/snd/reader.h>
#include <dsp++/float.h>

#include <gtest/gtest.h>

#include <fstream>

static void test_loudness_file(const char* path, float exp_level, float peak)
{
	using namespace dsp::snd;
	reader r;
	r.open(path);

	std::string dump(path);
	dump += ".raw";
	std::ofstream d(dump, std::ios_base::binary | std::ios_base::out);

	loudness_ebu<float> met(r.sample_rate(), r.channel_count());
	loudness_peak<float> metp(static_cast<unsigned>(r.sample_rate() * .1 +.5), 5);

	std::vector<float> buf;
	const size_t len = 9600;
	buf.resize(r.channel_count() * len);
	float vm = 0, vs = 0, vi = 0, vp = 0;
	while (true) {
		float* x = &buf[0];
		size_t read = r.read_frames(x, len);

		for (size_t i = 0; i < read; ++i, x += r.channel_count()) {
			if (met.next_frame(x)) {
				vm = met.value_m();
				vs = met.value_s();
				vi = met.value_i();
				d.write((char*)(&vm), 4);
				d.write((char*)(&vs), 4);
				d.write((char*)(&vi), 4);
			}
			metp(*x);
		}
		if (read != len)
			break;
	}

	vi = met.value_i();
	vp = metp.value_db();
	EXPECT_TRUE(dsp::within_range<float>(.1f)(vi,exp_level));
	EXPECT_TRUE(dsp::within_range<float>(.1f)(vp,peak));
}

TEST(loudness, ebu1)
{
	test_loudness_file("data/coil.wav", -11.6f, -.3f);
	test_loudness_file("data/ebu_testcase1_-23dBFS.wav", -23.f, -22.8f);
	test_loudness_file("data/ebu_testcase2_-33dBFS.wav", -33.f, -32.8f);
	test_loudness_file("data/ebu_testcase5_-23dBFS.wav", -23.f, -19.8f);
}

TEST(loudness, peak)
{
}
