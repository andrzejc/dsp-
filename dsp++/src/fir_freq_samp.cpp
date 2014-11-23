#include <dsp++/filter_design.h>

#include <complex>
#include <vector>

#include <dsp++/const.h>
#include <dsp++/fftw/dft.h>
#include <dsp++/pow2.h>
#include <dsp++/fft.h>
#include <dsp++/window.h>

using namespace dsp;



size_t dsp::fir_freq_samp(
		size_t order,				//!< [in] filter order, number of coefficients will be order + 1.
		std::complex<double> H[], 	//!< [out] filter response designed in the spectrum domain [order + 1].
		size_t point_count, 		//!< [in] number of points in the filter specification
		double freqs[], 			//!< [in] frequency points in [0, 0.5] range
		double amps[]				//!< [in] amplitude characteristic at each frequency point
)
{
	const char* msg = NULL;
	if (point_count < 2)
		msg = "point_count < 2";
	else if (freqs[0] != 0.)
		msg = "freqs[0] != 0.";
	else if (freqs[point_count - 1] != .5)
		msg = "freqs[point_count - 1] != .5";
	else if (!std::is_sorted(freqs, freqs + point_count))
		msg = "freqs not monotonically increasing";
	if (NULL != msg)
		throw std::invalid_argument(msg);

	size_t N = order + 1; 
	if (0. == amps[0] && (0 == (N % 2))) {
		--N;
		H[N] = 0.;
	}

	size_t Nfft = N / 2 + 1; 
	H[0] = amps[0];

	size_t seg = 0;
	double f0 = freqs[0], f1 = freqs[1], df = f1 - f0, a0 = amps[0], a1 = amps[1], da = a1 - a0;
	double dt = DSP_M_PI * (N - 1);
	for (size_t i = 1; i < Nfft - 1; ++i) {
		double f = i * .5 / (Nfft - 1);
		while (f >= f1) {
			++seg;
			f0 = f1;
			f1 = freqs[seg + 1];
			df = f1 - f0;
			a0 = a1;
			a1 = amps[seg + 1];
			da = a1 - a0;
		}
		double a = a0 + (f - f0) / df * da;
		double ph = f * dt;
		H[i] = std::polar(a, -ph);
	}
	H[Nfft] = std::polar(amps[point_count - 1], -.5 * dt);
	for (size_t i = Nfft + 1; i < N; ++i) 
		H[i] = std::conj(H[N - i]);
	return N;
}

size_t dsp::fir_freq_samp(
		size_t order,				//!< [in] filter order, number of coefficients will be order + 1.
		double h[], 				//!< [out] designed filter impulse response [order + 1].
		size_t point_count, 		//!< [in] number of points in the filter specification
		double freqs[], 			//!< [in] frequency points in [0, 0.5] range
		double amps[],				//!< [in] amplitude characteristic at each frequency point
		const double window[]		//!< [in] 
)
{
	std::vector<std::complex<double>, dsp::fftw::allocator<std::complex<double> > > H(order + 1);
	size_t n = fir_freq_samp(order, &H[0], point_count, freqs, amps);
	if (ispow2(n)) {
		dsp::fft<std::complex<double>, double> fft(n, &H[0], h);
		fft();
	}
	else {
		dsp::fftw::dft<std::complex<double>, double> fft(n, &H[0], h);
		fft();
	}
	std::transform(h, h + n, h, std::bind2nd(std::multiplies<double>(), 1./n));
	std::fill(h + n, h + order + 1, 0.);
	if (NULL != window) 
		std::transform(h, h + n, window, h, std::multiplies<double>());
	else 
		dsp::wnd::apply<dsp::wnd::hamming>(h, h + n); 
	return n;
}


#if 1
namespace {

	static bool test_freq_samp1() {
		size_t order = 30;
		std::complex<double> H[31];
		double f[] = {0, 0.3, 0.3, 0.5};
		double a[] = {1, 1, 0, 0};
		size_t n = dsp::fir_freq_samp(order, H, 4, f, a);
		return (n == 31);
	}

	static bool test_freq_samp2() {
		size_t order = 31;
		double h[32];
		double f[] = {0, 0.3, 0.3, 0.5};
		double a[] = {1, 1, 0, 0};
		size_t n = dsp::fir_freq_samp(order, h, 4, f, a);
		return (n == 32);
	}

	static const bool test_fs = test_freq_samp2();
}
#endif
