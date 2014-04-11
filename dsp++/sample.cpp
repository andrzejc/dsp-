#include <dsp++/snd/sample.h>
#include <dsp++/snd/convert.h>

#include <limits>
#include <cassert>

using namespace dsp::snd;

namespace {
template<class Int, class Res>
void read_pcm_signed_as_float_(const dsp::snd::sample_layout& sl, Res& res, const void* data) {
	Int i;
	sl.read_pcm(i, data);
	res = i / -static_cast<Res>(std::numeric_limits<Int>::min());
}

template<class Res>
void read_pcm_signed_as_float(const dsp::snd::sample_layout& sl, Res& res, const void* data) {
	if (sl.container_bytes > sizeof(int32_t)) 
		read_pcm_signed_as_float_<int64_t>(sl, res, data);
	else if (sl.container_bytes> sizeof(int16_t))
		read_pcm_signed_as_float_<int32_t>(sl, res, data);
	else if (sl.container_bytes > sizeof(int8_t))
		read_pcm_signed_as_float_<int16_t>(sl, res, data);
	else
		read_pcm_signed_as_float_<int8_t>(sl, res, data);
}

template<class Int, class Res>
void read_pcm_unsigned_as_float_(const dsp::snd::sample_layout& sl, Res& res, const void* data) {
	Int i;
	sl.read_pcm(i, data);
	res = i / static_cast<Res>(std::numeric_limits<Int>::max()) - Res(.5);
}

template<class Res>
void read_pcm_unsigned_as_float(const dsp::snd::sample_layout& sl, Res& res, const void* data) {
	if (sl.container_bytes > sizeof(int32_t)) 
		read_pcm_unsigned_as_float_<int64_t>(sl, res, data);
	else if (sl.container_bytes> sizeof(int16_t))
		read_pcm_unsigned_as_float_<int32_t>(sl, res, data);
	else if (sl.container_bytes > sizeof(int8_t))
		read_pcm_unsigned_as_float_<int16_t>(sl, res, data);
	else
		read_pcm_unsigned_as_float_<int8_t>(sl, res, data);
}

static dsp::snd::byte_order::label platform_test() {
	int16_t i = 1;
	int8_t buf[2];
	memcpy(buf, &i, 2);
	return ((buf[0] != 0) ? dsp::snd::byte_order::little_endian : dsp::snd::byte_order::big_endian);
}

}

#if !defined(DSP_ENDIAN_LITTLE) && !defined(DSP_ENDIAN_BIG)
const dsp::snd::byte_order::label dsp::snd::byte_order::platform = platform_test();
#endif

float dsp::snd::sample_layout::read_as_float(const void* data) const {
	float res;
	switch (type) {
	case sample::type::ieee_float:
		if (container_bytes > sizeof(float)) {
			double d;
			read_ieee_float(d, data);
			res = static_cast<float>(d);
		}
		else
			read_ieee_float(res, data);
		break;
	case sample::type::pcm_signed:
		read_pcm_signed_as_float(*this, res, data);
		break;
	case sample::type::pcm_unsigned:
		read_pcm_unsigned_as_float(*this, res, data);
		break;
	default:
		res = 0.f;
	}
	return res;
}

#if 0

static const char data[] = "\0\1\2\3\4\5\6\7";

static bool test() {
	dsp::snd::sample_layout s8_le(dsp::snd::sample::type::pcm_signed, 1, dsp::snd::byte_order::little_endian);
	long long res;
	s8_le.read_pcm_int_unnormalized(res, &data[0]);
	assert(res == 0);
	s8_le.read_pcm_int_unnormalized(res, &data[1]);
	assert(res == 1);

	dsp::snd::sample_layout s16_le(dsp::snd::sample::type::pcm_signed, 2, dsp::snd::byte_order::little_endian);
	s16_le.read_pcm_int_unnormalized(res, &data[0]);
	assert(res = 256);
	s16_le.read_pcm_int_unnormalized(res, &data[2]);
	assert(res = 3 * 256 + 2);

	dsp::snd::sample_layout s24_le(dsp::snd::sample::type::pcm_signed, 3, dsp::snd::byte_order::little_endian);
	s24_le.read_pcm_int_unnormalized(res, &data[0]);
	assert(res = 1 * 256 + 2 * 65536);
	s24_le.read_pcm_int_unnormalized(res, &data[3]);
	assert(res = 3 + 4 * 256 + 5 * 65536);

	return true;
}


static const bool t = test();

#endif 

void dsp::snd::convert_samples(const dsp::snd::sample_layout& sl_in, unsigned sample_stride_in, const void* in,
	const dsp::snd::sample_layout& sl_out, unsigned sample_stride_out, void* out, unsigned length) 
{
	const uint8_t* bi = static_cast<const uint8_t*>(in);
	uint8_t* bo = static_cast<uint8_t*>(out);
	//sample::type::label fmt;
	//if (sample::type::ieee_float == sl_in.type || sample::type::ieee_float == sl_out.type)
	//	fmt = sample::type::ieee_float;
	//else if (sample::type::pcm_signed == sl_in.type || sample::type::pcm_signed 
	
	for (unsigned i = 0; i < length; ++i, bi += sample_stride_in, bo += sample_stride_out) 
	{





	}
}