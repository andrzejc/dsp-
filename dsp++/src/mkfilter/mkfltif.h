//! @file external interface to mkfilter module
#pragma once

#define mkfilter_opt_be 0x00001	/* -Be		Bessel characteristic	       */
#define mkfilter_opt_bu 0x00002	/* -Bu		Butterworth characteristic     */
#define mkfilter_opt_ch 0x00004	/* -Ch		Chebyshev characteristic       */
#define mkfilter_opt_re 0x00008	/* -Re		Resonator		       */
#define mkfilter_opt_pi 0x00010	/* -Pi		proportional-integral	       */

#define mkfilter_opt_lp 0x00020	/* -Lp		lowpass			       */
#define mkfilter_opt_hp 0x00040	/* -Hp		highpass		       */
#define mkfilter_opt_bp 0x00080	/* -Bp		bandpass		       */
#define mkfilter_opt_bs 0x00100	/* -Bs		bandstop		       */
#define mkfilter_opt_ap 0x00200	/* -Ap		allpass			       */

#define mkfilter_opt_a  0x00400	/* -a		alpha value		       */
#define mkfilter_opt_l  0x00800	/* -l		just list filter parameters    */
#define mkfilter_opt_o  0x01000	/* -o		order of filter		       */
#define mkfilter_opt_p  0x02000	/* -p		specified poles only	       */
#define mkfilter_opt_w  0x04000	/* -w		don't pre-warp		       */
#define mkfilter_opt_z  0x08000	/* -z		use matched z-transform	       */
#define mkfilter_opt_Z  0x10000	/* -Z		additional zero		       */

namespace mkfilter {

	const unsigned max_pz = 512;
	const unsigned max_order = 10;

struct inout_b 
{
	int order;
	double raw_alpha1, raw_alpha2, raw_alphaz;
	unsigned options;
	unsigned polemask;
	double chebrip, qfactor;
	bool infq;
};

struct inout: public inout_b 
{
	double xcoeffs_r[max_pz+1], ycoeffs_r[max_pz+1];
};

void design(inout& params);

}