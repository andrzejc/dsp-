#include <dsp++/snd/loudness.h>
#include <dsp++/filter_design.h>

void dsp::snd::k_weighting_sos_design(double fs, double sos_num[2][dsp::sos_length], double sos_den[2][dsp::sos_length])
{
	const double s1_g = DSP_SND_K_WEIGHTING_STAGE1_GAIN;
	const double s1_q = DSP_SND_K_WEIGHTING_STAGE1_QUALITY;
	const double s2_q = DSP_SND_K_WEIGHTING_STAGE2_QUALITY;
	biquad_design(sos_num[0], sos_den[0], dsp::biquad_type_high_shelf_eq, DSP_SND_K_WEIGHTING_STAGE1_FC / fs, &s1_g, &s1_q, NULL, NULL);
	std::transform(&sos_num[0][0], &sos_num[0][0] + sos_length, &sos_num[0][0], std::bind2nd(std::divides<double>(), sos_den[0][0]));
	std::transform(&sos_den[0][0], &sos_den[0][0] + sos_length, &sos_den[0][0], std::bind2nd(std::divides<double>(), sos_den[0][0]));

	biquad_design(sos_num[1], sos_den[1], dsp::biquad_type_highpass, DSP_SND_K_WEIGHTING_STAGE2_FC / fs, NULL, &s2_q, NULL, NULL);
	std::transform(&sos_num[1][0], &sos_num[1][0] + sos_length, &sos_num[1][0], std::bind2nd(std::divides<double>(), sos_num[1][0]));
	std::transform(&sos_den[1][0], &sos_den[1][0] + sos_length, &sos_den[1][0], std::bind2nd(std::divides<double>(), sos_den[1][0]));
}
