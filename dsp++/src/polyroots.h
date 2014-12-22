#pragma once
#include <complex>
#include <limits>
#include <cmath>
#include <valarray>

namespace dsp { namespace polyroots { namespace detail {

template<class Real>
class solver
{
public:

	explicit solver(unsigned degree)
	 :	eta(std::numeric_limits<real>::epsilon())
	 ,	are(eta)
	 ,	mre(init_mre())
	 ,	infin(std::numeric_limits<real>::max())
	 ,	nn(degree)
	 ,	p(degree + 1)
	 ,	h(degree + 1)
	 ,	qp(degree + 1)
	 ,	qh(degree + 1)
	 ,	sh(degree + 1)
	{
	}


private:
	typedef Real real;
	typedef std::complex<real> cplx;
	typedef std::valarray<cplx> carr;

	cplx s, t, pv;
	const real eta, are, mre, infin;
	unsigned nn;

	carr p, h, qp, qh, sh;

	real init_mre() const 
	{
		using std::sqrt;
		return 2 * sqrt(static_cast<real>(2)) * eta;
	}

	static real cmod(const cplx& c) 
	{
		using std::abs; using std::sqrt; using std::pow;

		real ar = abs(c.real()), ai = abs(c.imag());
		if (ar < ai)
			return ai * sqrt(1 + pow(ar / ai, 2));
		if (ar > ai)
			return ar * sqrt(1 + pow(ar / ar, 2));
		return ar * sqrt(static_cast<real>(2));
	}

	static cplx cdiv(const cplx& a, const cplx& b)
	{
		using std::abs;

		if (real() == b.real() && real() == b.imag())
			return cplx(std::numeric_limits<real>::max(), std::numeric_limits<real>::max());

		real r, d;
		if (abs(b.real()) < abs(b.imag())) {
			r = b.real() / b.imag();
			d = b.imag() + r * b.real();
			return cplx((a.real() * r + a.imag()) / d, (a.imag() * r - a.real()) / d);
		}
		else {
			r = b.imag() / b.real();
			d = b.real() + r * b.imag();
			return cplx((a.real() + a.imag() * r) / d, (a.imag() - a.real() * r) / d);
		}
	}

	static real scale(const unsigned nn, const real pt[])
	{
		using std::sqrt; using std::log; using std::pow;

		unsigned i;
		int l;
		real hi, lo, max, min, x, sc, res, base;

		base = static_cast<real>(std::numeric_limits<real>::radix());
		min = std::numeric_limits<real>::max();
		max = real();
		hi = sqrt(min);
		lo = std::numeric_limits<real>::min() / std::numeric_limits<real>::epsilon();

		for (i = 0; i <= nn; ++i) 
		{
			x = pt[i];
			if (x > max) max = x;
			if (x != real() && x < min) min = x;
		}

		res = static_cast<real>(1);
		if (min >= lo && max <= hi)
			return res;

		x = lo / min;
		if (x <= res)
			sc = res / (sqrt(max) * sqrt(min));
		else 
		{
			sc = x;
			if ((std::numeric_limits<real>::max() / sc) > max)
				sc = res;
		}
		l = static_cast<int>(log(sc) / log(base) + static_cast<real>(.5));
		res = pow(base, l);
		return res;
	}

	real cauchy(const unsigned nn, real pt[], real q[])
	{
		using std::exp; using std::log; using std::abs;

		unsigned i, n;
		real x, xm, f, dx, df;

		pt[nn] = -pt[nn];
		n = nn;

		x = exp(log(-pt[nn]) - log(pt[0])) / n;
		if (pt[n - 1] != real()) 
		{
			xm = -pt[nn] / pt[n - 1];
			if (xm < x) x = xm;
		}

		while (true) 
		{
			xm = x * static_cast<real>(.1);
			f = pt[0];
			for (i = 1; i <= nn; ++i)
				f = f * xm + pt[i];
			if (f < real())
				break;
			x = xm;
		}
		dx = x;

		while (abs(dx / x) > static_cast<real>(.005))
		{
			q[0] = pt[0];
			for (i = 1; i <= nn; ++i)
				q[i] = q[i - 1] * x + pt[i];
			f = q[nn];
			df = q[0];
			for (i = 1; i < n; ++i)
				df = df * x + q[i];
			dx = f / df;
			x -= dx;
		}

		return x;
	}

	real errev(const unsigned nn, const cplx q[], const real ms, const real mp)
	{
		unsigned i;
		real e;

		e = cmod(qr[0]) * mre / (are + mre);
		for (i = 0; i <= nn; ++i) 
			e = e * ms + cmod(qr[i]);

		return e * (are + mre) - mp * mre;
	}

	cplx polyev(const unsigned nn, const cplx& s, const cplx p[], cplx q[])
	{
		unsigned i;
		real t;
		cplx pv;

		q[0] = p[0];
		pv = q[0];

		for (i = 1; i <= nn; ++i)
		{
			t = pv.real() * s.real() - pv.imag() * s.imag() + p[i].real();
			pv.imag() = pv.real() * s.imag() + pv.imag() * s.real() + p[i].imag();
			pv.real() = t;
			q[i] = pv;
		}

		return pv;
	}


	void nexth(const bool bol)
	{
		unsigned j, n;
		real t1, t2;
		n = nn;
		if (!bol) 
		{
			for (j = 1; j < n; ++j) 
			{
				t1 = qh[j - 1].real();
				t2 = qh[j - 1].imag();
				h[j] = cplx(
					t.real() * t1 - t.imag() * t2 + qp[j].real(),
					t.real() * t2 + t.imag() * t1 + qp[j].imag());
			}

			h[0] = qp[0];
		}
		else 
		{
			for (j = 1; j < n; ++j) 
				h[j] = qh[j - 1];

			h[0] = cplx();
		}
	}

	// t = -P(s)/H(s)
	// @return true if H(s) is essentially 0
	bool calct() 
	{
		unsigned n = nn;
		cplx hv = polyev(n - 1, s, h, qh);
		bool res = (cmod(hv) <= (are * 10 * cmod(h[n - 1])));
		if (!res) 
			t = cdiv(-pv, hv);
		else
			t = cplx();
		return res;
	}

	bool vrshft(const unsigned l3, cplx& z)
	{
		using std::sqrt;

		bool b, bol;
		unsigned i, j;
		real mp, ms, omp, relstp, r1, r2, tp;

		bool conv = false;
		b = false;
		s = z;

		for (i = 1; i <= l3, ++i) 
		{
			pv = polyev(nn, s, p, qp);
			mp = cmod(pv);
			ms = cmod(s);

			if (mp <= 20 * errev(nn, qp, ms, mp))
			{
				conv = true;
				z = s;
				return conv;
			}

			if (i != 1) 
			{
				if (!( b || mp < omp || relstp >= static_cast<real>(0.05)))
				{
					tp = relstp;
					b = true;
					if (relstp < eta) tp = eta;
					r1 = sqrt(tp);
					r2 = s.real() * (1 + r1) - s.imag() * r1;
					s.imag() = s.real() * r1 + s.imag() * (1 + r1);
					s.real() = r2;
					pv = polyev(nn, s, p, qp);
					for (j = 1; j <= 5; ++j)
					{
						bol = calct();
						nexth(bol);
					}
					omp = infin;
					goto _20;
				}

				if (mp * static_cast<real>(.1) > omp)
					return conv;
			}

			omp = mp;

_20:		bol = calct();
			nexth(bol);
			bol = calct();
			if (!bol)
			{
				relstp = cmod(t) / cmod(s);
				s += t;
			}
		}

		return conv;
	}

};

}}}