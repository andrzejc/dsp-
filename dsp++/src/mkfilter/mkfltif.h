//! @brief external interface to mkfilter module
#pragma once
#include "mkfcplxt.h"
#include <vector>

namespace mkfilter {

static const unsigned opt_be = 0x00001;	/* -Be		Bessel characteristic	       */
static const unsigned opt_bu = 0x00002;	/* -Bu		Butterworth characteristic     */
static const unsigned opt_ch = 0x00004;	/* -Ch		Chebyshev characteristic       */
static const unsigned opt_re = 0x00008;	/* -Re		Resonator		       */
static const unsigned opt_pi = 0x00010;	/* -Pi		proportional-integral	       */

static const unsigned opt_lp = 0x00020;	/* -Lp		lowpass			       */
static const unsigned opt_hp = 0x00040;	/* -Hp		highpass		       */
static const unsigned opt_bp = 0x00080;	/* -Bp		bandpass		       */
static const unsigned opt_bs = 0x00100;	/* -Bs		bandstop		       */
static const unsigned opt_ap = 0x00200;	/* -Ap		allpass			       */

static const unsigned opt_a =  0x00400;	/* -a		alpha value		       */
static const unsigned opt_l =  0x00800;	/* -l		just list filter parameters    */
static const unsigned opt_o =  0x01000;	/* -o		order of filter		       */
static const unsigned opt_p =  0x02000;	/* -p		specified poles only	       */
static const unsigned opt_w =  0x04000;	/* -w		don't pre-warp		       */
static const unsigned opt_z =  0x08000;	/* -z		use matched z-transform	       */
static const unsigned opt_Z =  0x10000;	/* -Z		additional zero		       */

const int max_order = 10;
const int max_pz = max_order * 2;

struct pzrep
{ 
	complex poles[max_pz], zeros[max_pz];
	int numpoles, numzeros;
};

struct context
{
	int order;
	double raw_alpha1, raw_alpha2, raw_alphaz;
	unsigned options;
	unsigned polemask;
	double chebrip, qfactor;
	bool infq;

	pzrep splane, zplane;
	complex dc_gain, fc_gain, hf_gain;
	double warped_alpha1, warped_alpha2;
	bool optsok;

	double* xcoeffs;
	double* ycoeffs;
};


void design(context& params);

double gain(const context& params);

}
