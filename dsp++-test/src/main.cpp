/*!
 * @file main.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <cppunit/ui/text/TestRunner.h>
#include "window_test.h"
#include "fft_test.h"
#include "pow2_test.h"
#include "overlap_add_test.h"
#include "filter_test.h"
#include "lattice_test.h"
#include "levinson_test.h"
#include "lpc_test.h"
#include "xcorr_test.h"
#include "filter_design_test.h"
#include "mean_test.h"
#include "simd_test.h"
#include "intmath_test.h"
#include "adaptfilt_test.h"

int main(int argc, char* argv[])
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(dsp::test::simd_test::suite());
	runner.addTest(dsp::test::fft_test::suite());
	runner.addTest(dsp::test::window_test::suite());
	runner.addTest(dsp::test::pow2_test::suite());
	runner.addTest(dsp::test::overlap_add_test::suite());
	runner.addTest(dsp::test::filter_test::suite());
	runner.addTest(dsp::test::lattice_test::suite());
	runner.addTest(dsp::test::levinson_test::suite());
	runner.addTest(dsp::test::lpc_test::suite());
	runner.addTest(dsp::test::xcorr_test::suite());
	runner.addTest(dsp::test::filter_design_test::suite());
	runner.addTest(dsp::test::mean_test::suite());
	runner.addTest(dsp::test::intmath_test::suite());
	runner.addTest(dsp::test::adaptfilt_test::suite());
	runner.run();
	return 0;
}




