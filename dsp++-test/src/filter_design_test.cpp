/*!
 * @file filter_design_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <dsp++/float.h>
#include <dsp++/filter_design.h>
#include "filter_design_test.h"
#include <stdexcept>
#include <cppunit/TestAssert.h>

using namespace dsp::test;

double f[] = {0, .15, .2, .3, .35, .5};
const double a[] = {0, 0, 1, 1, 0, 0};
const double w[] = {1, 1, 1};
const double b[] = {-9.14740606267554e-07, -0.0132815745135427, 4.50815794327054e-06, -0.0227391901997639, 5.80567543484568e-06, 0.0447649068234034, 1.53225273717582e-06, -0.0380715616942447, 1.48505518070342e-06, -0.0270869931233585, 2.71708112189860e-06, 0.141932483495935, -4.87930542623759e-06, -0.254376414183629, -1.02541763855492e-05, 0.301292819502550, -1.02541763855492e-05, -0.254376414183629, -4.87930542623759e-06, 0.141932483495935, 2.71708112189860e-06, -0.0270869931233585, 1.48505518070342e-06, -0.0380715616942447, 1.53225273717582e-06, 0.0447649068234034, 5.80567543484568e-06, -0.0227391901997639, 4.50815794327054e-06, -0.0132815745135427, -9.14740606267554e-07};

double hf[] = {0.05, 0.45};
const double ha[] = {1, 1};
const double hw[] =	{1000};
const double hb[] = {-0.00419563589034877, 1.51424873732622e-15, -0.00928210154880479, 6.18177841192320e-16, -0.0188358069977063, 1.21094131347115e-15, -0.0344010080193255, 9.61041302648788e-16, -0.0595515755697025,
			1.25589949896924e-15, -0.103037636419894, 8.54044024472374e-16, -0.196831535623640, 3.67836436250229e-16, -0.631353640882195, 0, 0.631353640882195, -3.67836436250229e-16, 0.196831535623640, -8.54044024472374e-16,
			0.103037636419894, -1.25589949896924e-15, 0.0595515755697025, -9.61041302648788e-16, 0.0344010080193255, -1.21094131347115e-15, 0.0188358069977063, -6.18177841192320e-16, 0.00928210154880479, -1.51424873732622e-15,
			0.00419563589034877};

double df[] = {0, 0.25, 0.275, 0.5 };
const double da[] = {0, 1, 0, 0};
const double dw[] =	{1, 1};

void filter_design_test::test_bp()
{
	const size_t N = 30;
	double h[N + 1];
	bool res = dsp::firpm(N, h, 3, f, a, w);
	CPPUNIT_ASSERT(std::equal(h, h + N + 1, b, dsp::within_range<double >(0.000001)));
	CPPUNIT_ASSERT(res);
}

void filter_design_test::test_hilbert()
{
	const size_t N = 30;
	double h[N + 1];
	bool res = dsp::firpm(N, h, 1, hf, ha, hw, dsp::filter_type_hilbert);
	CPPUNIT_ASSERT(std::equal(h, h + N + 1, hb, dsp::within_range<double >(0.003)));
	CPPUNIT_ASSERT(res);
}

void filter_design_test::test_differentiator()
{
	const size_t N = 60;
	double h[N + 1];
	bool res = dsp::firpm(N, h, 2, df, da, dw, dsp::filter_type_differentiator);
//	CPPUNIT_ASSERT(std::equal(h, h + N + 1, hb, dsp::within_range<double >(0.003)));
	CPPUNIT_ASSERT(res);
}

void filter_design_test::test_butter_bp()
{
	const size_t N = 6;
	double b[2 * N + 1], a[2 * N + 1];
	const double fc[] = {0.1, 0.2};
	const double a_ref[] = {1,-5.92041629838217,18.3064556556883,-37.7423139762241,57.2188065131169,-66.6720827750306,61.0355354353409,
		-44.1451196293111,25.0682015130099,-10.9271218325737,3.49829082250256,-0.746469481246891,0.0837564796186786};
	const double b_ref[] = {0.000340537652719436, 0,-0.00204322591631662,0,0.00510806479079154,0,-0.00681075305438872,
		0,0.00510806479079154,0, -0.00204322591631662, 0, 0.000340537652719436};

	dsp::iir::design(N, b, a, dsp::iir::resp::bandpass, fc);

	CPPUNIT_ASSERT(std::equal(b, b + 2 * N + 1, b_ref, dsp::within_range<double >(0.000000001)));
	CPPUNIT_ASSERT(std::equal(a, a + 2 * N + 1, a_ref, dsp::within_range<double >(0.000000001)));
}