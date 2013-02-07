/* mkfilter -- given n, compute recurrence relation
to implement Butterworth, Bessel or Chebyshev filter of order n
A.J. Fisher, University of York   <fisher@minster.york.ac.uk>
September 1992 */

/*
 * original sources and examples at http://www-users.cs.york.ac.uk/~fisher/mkfilter/
 */

#include <cstdio>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include "mkfilter.h"
#include "complex.h"

using namespace mkfilter;

#define opt_be 0x00001	/* -Be		Bessel characteristic	       */
#define opt_bu 0x00002	/* -Bu		Butterworth characteristic     */
#define opt_ch 0x00004	/* -Ch		Chebyshev characteristic       */
#define opt_re 0x00008	/* -Re		Resonator		       */
#define opt_pi 0x00010	/* -Pi		proportional-integral	       */

#define opt_lp 0x00020	/* -Lp		lowpass			       */
#define opt_hp 0x00040	/* -Hp		highpass		       */
#define opt_bp 0x00080	/* -Bp		bandpass		       */
#define opt_bs 0x00100	/* -Bs		bandstop		       */
#define opt_ap 0x00200	/* -Ap		allpass			       */

#define opt_a  0x00400	/* -a		alpha value		       */
#define opt_l  0x00800	/* -l		just list filter parameters    */
#define opt_o  0x01000	/* -o		order of filter		       */
#define opt_p  0x02000	/* -p		specified poles only	       */
#define opt_w  0x04000	/* -w		don't pre-warp		       */
#define opt_z  0x08000	/* -z		use matched z-transform	       */
#define opt_Z  0x10000	/* -Z		additional zero		       */

struct pzrep
{ 
	complex poles[MAXPZ], zeros[MAXPZ];
	int numpoles, numzeros;
};

struct context {
	pzrep splane, zplane;
	int order;
	double raw_alpha1, raw_alpha2, raw_alphaz;
	complex dc_gain, fc_gain, hf_gain;
	uint options;
	double warped_alpha1, warped_alpha2, chebrip, qfactor;
	bool infq;
	uint polemask;
	double xcoeffs[MAXPZ+1], ycoeffs[MAXPZ+1];
	bool optsok;
};

static const c_complex bessel_poles[] =
{ /* table produced by /usr/fisher/bessel --	N.B. only one member of each C.Conj. pair is listed */
	{ -1.00000000000e+00, 0.00000000000e+00}, { -1.10160133059e+00, 6.36009824757e-01},
	{ -1.32267579991e+00, 0.00000000000e+00}, { -1.04740916101e+00, 9.99264436281e-01},
	{ -1.37006783055e+00, 4.10249717494e-01}, { -9.95208764350e-01, 1.25710573945e+00},
	{ -1.50231627145e+00, 0.00000000000e+00}, { -1.38087732586e+00, 7.17909587627e-01},
	{ -9.57676548563e-01, 1.47112432073e+00}, { -1.57149040362e+00, 3.20896374221e-01},
	{ -1.38185809760e+00, 9.71471890712e-01}, { -9.30656522947e-01, 1.66186326894e+00},
	{ -1.68436817927e+00, 0.00000000000e+00}, { -1.61203876622e+00, 5.89244506931e-01},
	{ -1.37890321680e+00, 1.19156677780e+00}, { -9.09867780623e-01, 1.83645135304e+00},
	{ -1.75740840040e+00, 2.72867575103e-01}, { -1.63693941813e+00, 8.22795625139e-01},
	{ -1.37384121764e+00, 1.38835657588e+00}, { -8.92869718847e-01, 1.99832584364e+00},
	{ -1.85660050123e+00, 0.00000000000e+00}, { -1.80717053496e+00, 5.12383730575e-01},
	{ -1.65239648458e+00, 1.03138956698e+00}, { -1.36758830979e+00, 1.56773371224e+00},
	{ -8.78399276161e-01, 2.14980052431e+00}, { -1.92761969145e+00, 2.41623471082e-01},
	{ -1.84219624443e+00, 7.27257597722e-01}, { -1.66181024140e+00, 1.22110021857e+00},
	{ -1.36069227838e+00, 1.73350574267e+00}, { -8.65756901707e-01, 2.29260483098e+00},
};

static void readcmdline(char*[]);
static uint decodeoptions(char*), optbit(char);
//static double getfarg(char*);
//static int getiarg(char*);
static bool checkoptions(context& ctx);
static void /* usage(), opterror(char*, int = 0, int = 0), */ setdefaults(context& ctx);
static void compute_s(context& ctx), choosepole(context& ctx, complex), prewarp(context& ctx), normalize(context& ctx), compute_z_blt(context& ctx);
static complex blt(complex);
static void compute_z_mzt(context& ctx);
static void compute_notch(context& ctx), compute_apres(context& ctx);
static complex reflect(complex);
static void compute_bpres(context& ctx), add_extra_zero(context& ctx);
static void expandpoly(context& ctx), expand(complex[], int, complex[]), multin(complex, int, complex[]);
static void printresults(const context& ctx, char*[]), printcmdline(char*[]), printfilter(const context& ctx), printgain(char*, complex);
static void printcoeffs(char*, int, const double[]);
static void printrat_s(const context& ctx), printrat_z(const context& ctx), printpz(const complex*, int), printrecurrence(const context& ctx), prcomplex(complex);

static void design(context& ctx)
{ 
	//readcmdline(argv);
	checkoptions(ctx);
	setdefaults(ctx);
	if (ctx.options & opt_re) 
	{ 
		if (ctx.options & opt_bp) compute_bpres(ctx);	   /* bandpass resonator	 */
		if (ctx.options & opt_bs) compute_notch(ctx);	   /* bandstop resonator (notch) */
		if (ctx.options & opt_ap) compute_apres(ctx);	   /* allpass resonator		 */
	}
	else { 
		if (ctx.options & opt_pi) 
		{ 
			prewarp(ctx);
			ctx.splane.poles[0] = 0.0;
			ctx.splane.zeros[0] = -TWOPI * ctx.warped_alpha1;
			ctx.splane.numpoles = ctx.splane.numzeros = 1;
		}
		else { 
			compute_s(ctx);
			prewarp(ctx);
			normalize(ctx);
		}
		if (ctx.options & opt_z) 
			compute_z_mzt(ctx); 
		else 
			compute_z_blt(ctx);
	}
	if (ctx.options & opt_Z) 
		add_extra_zero(ctx);

	expandpoly(ctx);
	//printresults(ctx, argv);
}

//static void readcmdline(char *argv[])
//  { options = order = polemask = 0;
//    int ap = 0;
//    unless (argv[ap] == NULL) ap++; /* skip program name */
//    until (argv[ap] == NULL)
//      { uint m = decodeoptions(argv[ap++]);
//	if (m & opt_ch) chebrip = getfarg(argv[ap++]);
//	if (m & opt_a)
//	  { raw_alpha1 = getfarg(argv[ap++]);
//	    raw_alpha2 = (argv[ap] != NULL && argv[ap][0] != '-') ? getfarg(argv[ap++]) : raw_alpha1;
//	  }
//	if (m & opt_Z) raw_alphaz = getfarg(argv[ap++]);
//	if (m & opt_o) order = getiarg(argv[ap++]);
//	if (m & opt_p)
//	  { while (argv[ap] != NULL && argv[ap][0] >= '0' && argv[ap][0] <= '9')
//	      { int p = atoi(argv[ap++]);
//		if (p < 0 || p > 31) p = 31; /* out-of-range value will be picked up later */
//		polemask |= (1 << p);
//	      }
//	  }
//	if (m & opt_re)
//	  { char *s = argv[ap++];
//	    if (s != NULL && seq(s,"Inf")) infq = true;
//	    else { qfactor = getfarg(s); infq = false; }
//	  }
//	options |= m;
//    }
//}

//static uint decodeoptions(char *s)
//  { unless (*(s++) == '-') usage();
//    uint m = 0;
//    if (seq(s,"Be")) m |= opt_be;
//    else if (seq(s,"Bu")) m |= opt_bu;
//    else if (seq(s, "Ch")) m |= opt_ch;
//    else if (seq(s, "Re")) m |= opt_re;
//    else if (seq(s, "Pi")) m |= opt_pi;
//    else if (seq(s, "Lp")) m |= opt_lp;
//    else if (seq(s, "Hp")) m |= opt_hp;
//    else if (seq(s, "Bp")) m |= opt_bp;
//    else if (seq(s, "Bs")) m |= opt_bs;
//    else if (seq(s, "Ap")) m |= opt_ap;
//    else
//      { until (*s == '\0')
//	  { uint bit = optbit(*(s++));
//	    if (bit == 0) usage();
//	    m |= bit;
//	  }
//      }
//    return m;
//  }

//static uint optbit(char c)
//  { switch (c)
//      { default:    return 0;
//	case 'a':   return opt_a;
//	case 'l':   return opt_l;
//	case 'o':   return opt_o;
//	case 'p':   return opt_p;
//	case 'w':   return opt_w;
//	case 'z':   return opt_z;
//	case 'Z':   return opt_Z;
//      }
//  }

//static double getfarg(char *s)
//  { if (s == NULL) usage();
//    return atof(s);
//  }

//static int getiarg(char *s)
//  { if (s == NULL) usage();
//    return atoi(s);
//  }

//static void usage()
//  { fprintf(stderr, "Mkfilter V.%s from <fisher@minster.york.ac.uk>\n", VERSION);
//    fprintf(stderr, "Usage: mkfilter [-Be | -Bu | -Ch <r> | -Pi] [-Lp | -Hp | -Bp | -Bs] [-p <n1> <n2> ...] [-{lwz}] "
//				     "[-Z <alphaz>] "
//				     "-o <order> -a <alpha1> [ <alpha2> ]\n");
//    fprintf(stderr, "       mkfilter -Re <q> [-Bp | -Bs | -Ap] [-l] -a <alpha>\n\n");
//    fprintf(stderr, "  -Be, Bu             = Bessel, Butterworth\n");
//    fprintf(stderr, "  -Ch <r>             = Chebyshev (r = dB ripple)\n");
//    fprintf(stderr, "  -Pi                 = Proportional-Integral\n");
//    fprintf(stderr, "  -Re <q>             = 2-pole resonator (q = Q-factor)\n");
//    fprintf(stderr, "  -Lp, Hp, Bp, Bs, Ap = lowpass, highpass, bandpass, bandstop, allpass\n");
//    fprintf(stderr, "  -p                  = use listed poles only (ni = 0 .. order-1)\n");
//    fprintf(stderr, "  -l                  = just list <order> parameters\n");
//    fprintf(stderr, "  -w                  = don't pre-warp frequencies\n");
//    fprintf(stderr, "  -z                  = use matched z-transform\n");
//    fprintf(stderr, "  -Z                  = additional z-plane zero\n");
//    fprintf(stderr, "  order = 1..%d;  alpha = f(corner)/f(sample)\n\n", MAXORDER);
//    exit(1);
//  }

static bool checkoptions(context& ctx)
{ 
	ctx.optsok = true;
	unless (onebit(ctx.options & (opt_be | opt_bu | opt_ch | opt_re | opt_pi))) 
	{
		throw std::invalid_argument("missing filter design method");
		//opterror("must specify exactly one of -Be, -Bu, -Ch, -Re, -Pi");
	}
	if (ctx.options & opt_re) 
	{ 
		unless (onebit(ctx.options & (opt_bp | opt_bs | opt_ap))) 
			throw std::invalid_argument("missing resonator type");
		//opterror("must specify exactly one of -Bp, -Bs, -Ap with -Re");

		if (ctx.options & (opt_lp | opt_hp | opt_o | opt_p | opt_w | opt_z))
			throw std::invalid_argument("options incompatible with 2-pole resonator filter");
		//opterror("can't use -Lp, -Hp, -o, -p, -w, -z with -Re");
	}
	else if (ctx.options & opt_pi) 
	{ 
		if (ctx.options & (opt_lp | opt_hp | opt_bp | opt_bs | opt_ap))
			throw std::invalid_argument("options incompatible with proportional integrator");
		//opterror("-Lp, -Hp, -Bp, -Bs, -Ap illegal in conjunction with -Pi");
		unless ((ctx.options & opt_o) && (ctx.order == 1)) 
			throw std::out_of_range("proportional integrator allows only order of 1");
		// opterror("-Pi implies -o 1");
	}
	else 
	{ 
		unless (onebit(ctx.options & (opt_lp | opt_hp | opt_bp | opt_bs)))
			throw std::invalid_argument("missing filter type");
		// opterror("must specify exactly one of -Lp, -Hp, -Bp, -Bs");
		if (ctx.options & opt_ap)
			throw std::invalid_argument("allpass characteristic avaialable only with 2-pole resonator");
		// opterror("-Ap implies -Re");
		if (ctx.options & opt_o) 
		{ 
			unless (ctx.order >= 1 && ctx.order <= MAXORDER) 
				throw std::out_of_range("filter order out of range");
			//	opterror("order must be in range 1 .. %d", MAXORDER);
			if (ctx.options & opt_p) 
			{ 
				uint m = (1 << ctx.order) - 1; /* "order" bits set */
				if ((ctx.polemask & ~m) != 0)
					throw std::out_of_range("selected poles out of range");
				//opterror("order=%d, so args to -p must be in range 0 .. %d", ctx.order, ctx.order-1);
			}
		}
		else 
			throw std::invalid_argument("missing filter order");
		//opterror("must specify -o");
	}
	unless (ctx.options & opt_a) 
		throw std::invalid_argument("missing normalized corner frequency");
	//opterror("must specify -a");

	return (ctx.optsok);
}

//static void opterror(context& ctx, char *msg, int p1, int p2)
//  { fprintf(stderr, "mkfilter: "); fprintf(stderr, msg, p1, p2); putc('\n', stderr);
//    ctx.optsok = false;
//  }

static void setdefaults(context& ctx)
{ 
	unless (ctx.options & opt_p) 
		ctx.polemask = ~0; /* use all poles */
	unless (ctx.options & (opt_bp | opt_bs)) 
		ctx.raw_alpha2 = ctx.raw_alpha1;
}

static void compute_s(context& ctx) /* compute S-plane poles for prototype LP filter */
{ 
	ctx.splane.numpoles = 0;
	if (ctx.options & opt_be)
	{ /* Bessel filter */
		int p = (ctx.order*ctx.order)/4; /* ptr into table */
		if (ctx.order & 1) 
			choosepole(ctx, bessel_poles[p++]);
		for (int i = 0; i < ctx.order/2; i++)
		{ 
			choosepole(ctx, bessel_poles[p]);
			choosepole(ctx, cconj(bessel_poles[p]));
			p++;
		}
	}
	if (ctx.options & (opt_bu | opt_ch))
	{ /* Butterworth filter */
		for (int i = 0; i < 2*ctx.order; i++)
		{ 
			double theta = (ctx.order & 1) ? (i*PI) / ctx.order : ((i+0.5)*PI) / ctx.order;
			choosepole(ctx, expj(theta));
		}
	}
	if (ctx.options & opt_ch)
	{ /* modify for Chebyshev (p. 136 DeFatta et al.) */
		if (ctx.chebrip >= 0.0)
		{ 
			throw std::domain_error("Chebyshev ripple must be < 0");
			//fprintf(stderr, "mkfilter: Chebyshev ripple is %g dB; must be .lt. 0.0\n", chebrip);
			//   exit(1);
		}
		double rip = pow(10.0, -ctx.chebrip / 10.0);
		double eps = sqrt(rip - 1.0);
		double y = asinh(1.0 / eps) / (double) ctx.order;
		if (y <= 0.0)
		{ 
			throw std::domain_error("Chebyshev y must be > 0");
			//fprintf(stderr, "mkfilter: bug: Chebyshev y=%g; must be .gt. 0.0\n", y);
			//   exit(1);
		}
		for (int i = 0; i < ctx.splane.numpoles; i++)
		{ 
			ctx.splane.poles[i].re *= sinh(y);
			ctx.splane.poles[i].im *= cosh(y);
		}
	}
}

static void choosepole(context& ctx, complex z)
{ 
	if (z.re < 0.0)
	{ 
		if (ctx.polemask & 1) ctx.splane.poles[ctx.splane.numpoles++] = z;
		ctx.polemask >>= 1;
	}
}

static void prewarp(context& ctx)
{ /* for bilinear transform, perform pre-warp on alpha values */
	if (ctx.options & (opt_w | opt_z))
	{ 
		ctx.warped_alpha1 = ctx.raw_alpha1;
		ctx.warped_alpha2 = ctx.raw_alpha2;
	}
	else
	{ 
		ctx.warped_alpha1 = tan(PI * ctx.raw_alpha1) / PI;
		ctx.warped_alpha2 = tan(PI * ctx.raw_alpha2) / PI;
	}
}

static void normalize(context& ctx)		/* called for trad, not for -Re or -Pi */
{ 
	double w1 = TWOPI * ctx.warped_alpha1;
	double w2 = TWOPI * ctx.warped_alpha2;
	/* transform prototype into appropriate filter type (lp/hp/bp/bs) */
	switch (ctx.options & (opt_lp | opt_hp | opt_bp| opt_bs)) { 
	case opt_lp: { 
		for (int i = 0; i < ctx.splane.numpoles; i++) 
			ctx.splane.poles[i] = ctx.splane.poles[i] * w1;
		ctx.splane.numzeros = 0;
		break;
				 }

	case opt_hp: { 
		int i;
		for (i=0; i < ctx.splane.numpoles; i++) 
			ctx.splane.poles[i] = w1 / ctx.splane.poles[i];
		for (i=0; i < ctx.splane.numpoles; i++) 
			ctx.splane.zeros[i] = 0.0;	 /* also N zeros at (0,0) */
		ctx.splane.numzeros = ctx.splane.numpoles;
		break;
				 }

	case opt_bp: { 
		double w0 = sqrt(w1*w2), bw = w2-w1; int i;
		for (i=0; i < ctx.splane.numpoles; i++) { 
			complex hba = 0.5 * (ctx.splane.poles[i] * bw);
			complex temp = csqrt(1.0 - sqr(w0 / hba));
			ctx.splane.poles[i] = hba * (1.0 + temp);
			ctx.splane.poles[ctx.splane.numpoles+i] = hba * (1.0 - temp);
		}
		for (i=0; i < ctx.splane.numpoles; i++) 
			ctx.splane.zeros[i] = 0.0;	 /* also N zeros at (0,0) */
		ctx.splane.numzeros = ctx.splane.numpoles;
		ctx.splane.numpoles *= 2;
		break;
				 }

	case opt_bs: { 
		double w0 = sqrt(w1*w2), bw = w2-w1; int i;
		for (i=0; i < ctx.splane.numpoles; i++)	{ 
			complex hba = 0.5 * (bw / ctx.splane.poles[i]);
			complex temp = csqrt(1.0 - sqr(w0 / hba));
			ctx.splane.poles[i] = hba * (1.0 + temp);
			ctx.splane.poles[ctx.splane.numpoles+i] = hba * (1.0 - temp);
		}
		for (i=0; i < ctx.splane.numpoles; i++)	   /* also 2N zeros at (0, +-w0) */ { 
			ctx.splane.zeros[i] = complex(0.0, +w0);
			ctx.splane.zeros[ctx.splane.numpoles+i] = complex(0.0, -w0);
		}
		ctx.splane.numpoles *= 2;
		ctx.splane.numzeros = ctx.splane.numpoles;
		break;
				 }
	}
}

static complex blt(complex pz) { return (2.0 + pz) / (2.0 - pz);}

static void compute_z_blt(context& ctx) /* given S-plane poles & zeros, compute Z-plane poles & zeros, by bilinear transform */
{ 
	int i;
	ctx.zplane.numpoles = ctx.splane.numpoles;
	ctx.zplane.numzeros = ctx.splane.numzeros;
	for (i=0; i < ctx.zplane.numpoles; i++) 
		ctx.zplane.poles[i] = blt(ctx.splane.poles[i]);
	for (i=0; i < ctx.zplane.numzeros; i++) 
		ctx.zplane.zeros[i] = blt(ctx.splane.zeros[i]);
	while (ctx.zplane.numzeros < ctx.zplane.numpoles) 
		ctx.zplane.zeros[ctx.zplane.numzeros++] = -1.0;
}

static void compute_z_mzt(context& ctx) /* given S-plane poles & zeros, compute Z-plane poles & zeros, by matched z-transform */
{ 
	int i;
	ctx.zplane.numpoles = ctx.splane.numpoles;
	ctx.zplane.numzeros = ctx.splane.numzeros;
	for (i=0; i < ctx.zplane.numpoles; i++) 
		ctx.zplane.poles[i] = cexp(ctx.splane.poles[i]);
	for (i=0; i < ctx.zplane.numzeros; i++) 
		ctx.zplane.zeros[i] = cexp(ctx.splane.zeros[i]);
}

static void compute_notch(context& ctx)
{ /* compute Z-plane pole & zero positions for bandstop resonator (notch filter) */
	compute_bpres(ctx);		/* iterate to place poles */
	double theta = TWOPI * ctx.raw_alpha1;
	complex zz = expj(theta);	/* place zeros exactly */
	ctx.zplane.zeros[0] = zz; ctx.zplane.zeros[1] = cconj(zz);
}

static void compute_apres(context& ctx)
{ /* compute Z-plane pole & zero positions for allpass resonator */
	compute_bpres(ctx);		/* iterate to place poles */
	ctx.zplane.zeros[0] = reflect(ctx.zplane.poles[0]);
	ctx.zplane.zeros[1] = reflect(ctx.zplane.poles[1]);
}

static complex reflect(complex z)
{ 
	double r = hypot(z);
	return z / sqr(r);
}

static void compute_bpres(context& ctx)
{ /* compute Z-plane pole & zero positions for bandpass resonator */
	ctx.zplane.numpoles = ctx.zplane.numzeros = 2;
	ctx.zplane.zeros[0] = 1.0; ctx.zplane.zeros[1] = -1.0;
	double theta = TWOPI * ctx.raw_alpha1; /* where we want the peak to be */
	if (ctx.infq)
	{ /* oscillator */
		complex zp = expj(theta);
		ctx.zplane.poles[0] = zp; ctx.zplane.poles[1] = cconj(zp);
	}
	else
	{ /* must iterate to find exact pole positions */
		complex topcoeffs[MAXPZ+1]; 
		expand(ctx.zplane.zeros, ctx.zplane.numzeros, topcoeffs);
		double r = exp(-theta / (2.0 * ctx.qfactor));
		double thm = theta, th1 = 0.0, th2 = PI;
		bool cvg = false;
		for (int i=0; i < 50 && !cvg; i++)
		{ 
			complex zp = r * expj(thm);
			ctx.zplane.poles[0] = zp; ctx.zplane.poles[1] = cconj(zp);
			complex botcoeffs[MAXPZ+1]; expand(ctx.zplane.poles, ctx.zplane.numpoles, botcoeffs);
			complex g = evaluate(topcoeffs, ctx.zplane.numzeros, botcoeffs, ctx.zplane.numpoles, expj(theta));
			double phi = g.im / g.re; /* approx to atan2 */
			if (phi > 0.0) th2 = thm; else th1 = thm;
			if (fabs(phi) < EPS) cvg = true;
			thm = 0.5 * (th1+th2);
		}
		unless (cvg) 
			throw std::runtime_error("failed to converge");
		// fprintf(stderr, "mkfilter: warning: failed to converge\n");
	}
}

static void add_extra_zero(context& ctx)
{ 
	if (ctx.zplane.numzeros+2 > MAXPZ)
	{ 
		throw std::runtime_error("too many zeros");
		//  fprintf(stderr, "mkfilter: too many zeros; can't do -Z\n");
		//exit(1);
	}
	double theta = TWOPI * ctx.raw_alphaz;
	complex zz = expj(theta);
	ctx.zplane.zeros[ctx.zplane.numzeros++] = zz;
	ctx.zplane.zeros[ctx.zplane.numzeros++] = cconj(zz);
	while (ctx.zplane.numpoles < ctx.zplane.numzeros) 
		ctx.zplane.poles[ctx.zplane.numpoles++] = 0.0;	 /* ensure causality */
}

static void expandpoly(context& ctx) /* given Z-plane poles & zeros, compute top & bot polynomials in Z, and then recurrence relation */
{ 
	complex topcoeffs[MAXPZ+1], botcoeffs[MAXPZ+1]; int i;
	expand(ctx.zplane.zeros, ctx.zplane.numzeros, topcoeffs);
	expand(ctx.zplane.poles, ctx.zplane.numpoles, botcoeffs);
	ctx.dc_gain = evaluate(topcoeffs, ctx.zplane.numzeros, botcoeffs, ctx.zplane.numpoles, 1.0);
	double theta = TWOPI * 0.5 * (ctx.raw_alpha1 + ctx.raw_alpha2); /* "jwT" for centre freq. */
	ctx.fc_gain = evaluate(topcoeffs, ctx.zplane.numzeros, botcoeffs, ctx.zplane.numpoles, expj(theta));
	ctx.hf_gain = evaluate(topcoeffs, ctx.zplane.numzeros, botcoeffs, ctx.zplane.numpoles, -1.0);
	for (i = 0; i <= ctx.zplane.numzeros; i++) 
		ctx.xcoeffs[i] = +(topcoeffs[i].re / botcoeffs[ctx.zplane.numpoles].re);
	for (i = 0; i <= ctx.zplane.numpoles; i++) 
		ctx.ycoeffs[i] = -(botcoeffs[i].re / botcoeffs[ctx.zplane.numpoles].re);
}

static void expand(complex pz[], int npz, complex coeffs[])
{ /* compute product of poles or zeros as a polynomial of z */
	int i;
	coeffs[0] = 1.0;
	for (i=0; i < npz; i++) coeffs[i+1] = 0.0;
	for (i=0; i < npz; i++) multin(pz[i], npz, coeffs);
	/* check computed coeffs of z^k are all real */
	for (i=0; i < npz+1; i++)
	{ 
		if (fabs(coeffs[i].im) > EPS)
		{ 
			throw std::runtime_error("poles/zeros not complex conjugates");
			//fprintf(stderr, "mkfilter: coeff of z^%d is not real; poles/zeros are not complex conjugates\n", i);
			// exit(1);
		}
	}
}

static void multin(complex w, int npz, complex coeffs[])
{ /* multiply factor (z-w) into coeffs */
	complex nw = -w;
	for (int i = npz; i >= 1; i--) 
		coeffs[i] = (nw * coeffs[i]) + coeffs[i-1];
	coeffs[0] = nw * coeffs[0];
}

static void printresults(const context& ctx, char *argv[])
{ 
	if (ctx.options & opt_l)
	{ /* just list parameters */
		printcmdline(argv);
		complex gain = (ctx.options & opt_pi) ? ctx.hf_gain :
			(ctx.options & opt_lp) ? ctx.dc_gain :
			(ctx.options & opt_hp) ? ctx.hf_gain :
			(ctx.options & (opt_bp | opt_ap)) ? ctx.fc_gain :
			(ctx.options & opt_bs) ? csqrt(ctx.dc_gain * ctx.hf_gain) : complex(1.0);
		printf("G  = %.10e\n", hypot(gain));
		printcoeffs("NZ", ctx.zplane.numzeros, ctx.xcoeffs);
		printcoeffs("NP", ctx.zplane.numpoles, ctx.ycoeffs);
	}
	else
	{ 
		printf("Command line: ");
		printcmdline(argv);
		printfilter(ctx);
	}
}

static void printcmdline(char *argv[])
{ 
	int k = 0;
	until (argv[k] == NULL)
	{ 
		if (k > 0) putchar(' ');
		fputs(argv[k++], stdout);
	}
	putchar('\n');
}

static void printcoeffs(char *pz, int npz, const double coeffs[])
{ 
	printf("%s = %d\n", pz, npz);
	for (int i = 0; i <= npz; i++) printf("%18.10e\n", coeffs[i]);
}

static void printfilter(const context& ctx)
{ 
	printf("raw alpha1    = %14.10f\n", ctx.raw_alpha1);
	printf("raw alpha2    = %14.10f\n", ctx.raw_alpha2);
	unless (ctx.options & (opt_re | opt_w | opt_z))
	{ 
		printf("warped alpha1 = %14.10f\n", ctx.warped_alpha1);
		printf("warped alpha2 = %14.10f\n", ctx.warped_alpha2);
	}
	printgain("dc    ", ctx.dc_gain);
	printgain("centre", ctx.fc_gain);
	printgain("hf    ", ctx.hf_gain);
	putchar('\n');
	unless (ctx.options & opt_re) printrat_s(ctx);
	printrat_z(ctx);
	printrecurrence(ctx);
}

static void printgain(char *str, complex gain)
{ 
	double r = hypot(gain);
	printf("gain at %s:   mag = %15.9e", str, r);
	if (r > EPS) printf("   phase = %14.10f pi", atan2(gain) / PI);
	putchar('\n');
}

static void printrat_s(const context& ctx)	/* print S-plane poles and zeros */
{ 
	printf("S-plane zeros:\n");
	printpz(ctx.splane.zeros, ctx.splane.numzeros);
	printf("S-plane poles:\n");
	printpz(ctx.splane.poles, ctx.splane.numpoles);
}

static void printrat_z(const context& ctx)	/* print Z-plane poles and zeros */
{ 
	printf("Z-plane zeros:\n");
	printpz(ctx.zplane.zeros, ctx.zplane.numzeros);
	printf("Z-plane poles:\n");
	printpz(ctx.zplane.poles, ctx.zplane.numpoles);
}

static void printpz(const complex *pzvec, int num)
{ 
	int n1 = 0;
	while (n1 < num)
	{ 
		putchar('\t'); prcomplex(pzvec[n1]);
		int n2 = n1+1;
		while (n2 < num && pzvec[n2] == pzvec[n1]) n2++;
		if (n2-n1 > 1) printf("\t%d times", n2-n1);
		putchar('\n');
		n1 = n2;
	}
	putchar('\n');
}

static void printrecurrence(const context& ctx) /* given (real) Z-plane poles & zeros, compute & print recurrence relation */
{ 
	printf("Recurrence relation:\n");
	printf("y[n] = ");
	int i;
	for (i = 0; i < ctx.zplane.numzeros+1; i++)
	{ 
		if (i > 0) printf("     + ");
		double x = ctx.xcoeffs[i];
		double f = fmod(fabs(x), 1.0);
		char *fmt = (f < EPS || f > 1.0-EPS) ? "%3g" : "%14.10f";
		putchar('('); printf(fmt, x); printf(" * x[n-%2d])\n", ctx.zplane.numzeros-i);
	}
	putchar('\n');
	for (i = 0; i < ctx.zplane.numpoles; i++)
	{ 
		printf("     + (%14.10f * y[n-%2d])\n", ctx.ycoeffs[i], ctx.zplane.numpoles-i);
	}
	putchar('\n');
}

static void prcomplex(complex z)
{ 
	printf("%14.10f + j %14.10f", z.re, z.im);
}

