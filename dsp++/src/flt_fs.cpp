#include <dsp++/flt/fir_design.h>

#include <complex>
#include <stdexcept>

#include <dsp++/const.h>
#include <dsp++/pow2.h>
#include <dsp++/fft.h>
#include <dsp++/window.h>
#include <dsp++/trivial_array.h>
#include <dsp++/debug.h>

#if !DSP_FFTW_DISABLED
#include <dsp++/fftw/dft.h>
#endif // !DSP_FFTW_DISABLED

namespace {

static inline unsigned fir_fs_length(unsigned order, const double amps[]) {
	unsigned N = order + 1; 
	if (0. == amps[0] && (0 == (N % 2)))
		--N;
	return N;
}

static inline void fir_fs_check_preconditions(size_t point_count, const double freqs[]) {
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
}

}

unsigned dsp::fir::fs::design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],			//!< [in] amplitude characteristic at each frequency point
		std::complex<double> H[] 	//!< [out] filter response designed in the spectrum domain [order + 1].
)
{
	fir_fs_check_preconditions(point_count, freqs);
	unsigned N = fir_fs_length(order, amps);
	if (N == order)
		H[N] = 0.;

	unsigned Nfft = N / 2 + 1; 
	H[0] = amps[0];

	unsigned seg = 0;
	double f0 = freqs[0], f1 = freqs[1], df = f1 - f0, a0 = amps[0], a1 = amps[1], da = a1 - a0;
	double dt = DSP_M_PI * (N - 1);
	for (unsigned i = 1; i < Nfft - 1; ++i) {
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
	for (unsigned i = Nfft + 1; i < N; ++i) 
		H[i] = conj(H[N - i]);
	return N;
}

namespace {

unsigned fir_fs_impl(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		const double win[],			//!< [in] 
		double h[] 					//!< [out] designed filter impulse response [order + 1].
)
{
#if DSP_FFTW_DISABLED
	fir_freq_samp_check_preconditions(point_count, freqs);
	if (!ispow2(fir_freq_samp_length(order, amps)))
		throw std::invalid_argument("dsp::fir::fs::design(): with fftw disabled only filters of power-of-2 length allowed");
	dsp::trivial_array<std::complex<double>> H(order + 1);
#else // !DSP_FFTW_DISABLED
	dsp::trivial_array<std::complex<double>, dsp::fftw::allocator<std::complex<double> > > H(order + 1);
#endif // !DSP_FFTW_DISABLED

	unsigned n = dsp::fir::fs::design(order, point_count, freqs, amps, H.begin());
	if (dsp::ispow2(n)) {
		dsp::fft<std::complex<double>, double> fft(n, H.begin(), h);
		fft();
	}
#if !DSP_FFTW_DISABLED
	else {
		dsp::fftw::dft<std::complex<double>, double> fft(static_cast<size_t>(n), H.begin(), h);
		fft();
	}
#endif // !DSP_FFTW_DISABLED

	std::transform(h, h + n, h, std::bind2nd(std::multiplies<double>(), 1./n));
	std::fill(h + n, h + order + 1, 0.);
	if (NULL != win) 
		std::transform(h, h + n, win, h, std::multiplies<double>());
	return n;
}

}

unsigned dsp::fir::fs::design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		const double win[],			//!< [in] 
		double h[]					//!< [out] designed filter impulse response [order + 1].
)
{
	return fir_fs_impl(order, point_count, freqs, amps, win, h);
}

unsigned dsp::fir::fs::design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		double h[] 					//!< [out] designed filter impulse response [order + 1].
)
{
	unsigned n = fir_fs_impl(order, point_count, freqs, amps, NULL, h);
	dsp::wnd::apply<dsp::wnd::hamming>(h, h + n);
	return n;
}

#if 0
namespace {

	static bool test_freq_samp1() {
		size_t order = 30;
		std::complex<double> H[31];
		double f[] = {0, 0.3, 0.34, 0.5};
		double a[] = {1, 1, 0, 0};
		size_t n = dsp::fir_freq_samp(order, H, 4, f, a);
		return (n == 31);
	}

	static bool test_freq_samp2() {
		size_t order = 31;
		double h[32];
		double f[] = {0, 0.3, 0.34, 0.5};
		double a[] = {1, 1, 0, 0};
		size_t n = dsp::fir_freq_samp(order, h, 4, f, a);
		dsp::dbg::clipbrd_copy(h, n);
		return (n == 32);
	}

	static const bool test_fs = test_freq_samp2();
}
#endif
