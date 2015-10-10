/*!
 * @file xcorr_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp> 

#include <dsp++/xcorr.h>
#include <dsp++/float.h>

const double x[] = {0.5376671552658081,1.833885073661804,-2.258846759796143,0.862173318862915,0.318765252828598,-1.307688355445862,-0.4335920214653015};
const double a[] = {-0.233128182363105,	-1.49825893784674,	-1.24734239711250,	3.62817718923044,	-1.61909088509753,	-4.67897467218546,	11.4976141757680,	-4.67897467218546,	-1.61909088509753,	3.62817718923044,	-1.24734239711250,	-1.49825893784674,	-0.233128182363105};
const double a_b[] = {-0.0333040260518722,-0.214036991120963,-0.178191771016071,0.518311027032920,-0.231298697871075,-0.668424953169352,1.64251631082400,-0.668424953169352,-0.231298697871075,0.518311027032920,-0.178191771016071,-0.214036991120963,-0.0333040260518722};
const double a_u[] = {-0.233128182363105,-0.749129468923371,-0.415780799037500,0.907044297307610,-0.323818177019506,-0.779829112030911,1.64251631082400,-0.779829112030911,-0.323818177019506,0.907044297307610,-0.415780799037500,-0.749129468923371,-0.233128182363105};
const double a_c[] = {-0.0202762224231213,-0.130310420487445,-0.108487063319741,0.315559135466301,-0.140819726627275,-0.406951790228508,1,-0.406951790228508,-0.140819726627275,0.315559135466301,-0.108487063319741,-0.130310420487445,-0.0202762224231213};

const double y[] = {0.342624466538650, 3.578396939725761, 2.769437029884877, -1.349886940156521, 3.034923466331855, 0.725404224946106, -0.063054873189656};
const double xa[] = {-0.0339025333023251, 0.274390627601723, 3.10451785891205, 3.14696952966276, -7.23660502279516, 12.9823232056811, -0.626818624491665, -9.78054251654882, 3.64337517914841, -1.60018995031109, -5.77101682979227, -1.99961037062859, -0.148559235337899};
const double xa_b[] = {-0.00484321904318930, 0.0391986610859605, 0.443502551273150, 0.449567075666108, -1.03380071754217, 1.85461760081159, -0.0895455177845235, -1.39722035950697, 0.520482168449774, -0.228598564330155, -0.824430975684609, -0.285658624375513, -0.0212227479054141};
const double xa_u[] = {-0.0339025333023251, 0.137195313800862, 1.03483928630402, 0.786742382415689, -1.44732100455903, 2.16372053428018, -0.0895455177845235, -1.63009041942480, 0.728675035829683, -0.400047487577772, -1.92367227659742, -0.999805185314296, -0.148559235337899};
const double xa_c[] = {-0.00176320337191762, 0.0142705111590222, 0.161459803260094, 0.163667630278320, -0.376361443661006, 0.675184826085294, -0.0325995907865879, -0.508666576346371, 0.189484701445448, -0.0832226987563697, -0.300139114765952, -0.103995760923623, -0.00772627055156735};

#define C(r,i) std::complex<double>(r,i)

const std::complex<double> cx[] = {C(0.342624466538650, 0.714742903826096), C(3.57839693972576, - 0.204966058299775), C(2.76943702988488, - 0.124144348216312), C(-1.34988694015652, 1.48969760778547), C(3.03492346633185, 1.40903448980048), C(0.725404224946106, 1.41719241342961), C(-0.0630548731896562, 0.671497133608081)};
const std::complex<double> ca[] = {C(0.458343668896671, - 0.275139370348661),	C(0.898199970560132, - 2.35704645157174),	C(4.09425527753994, - 5.38538104020854),	C(14.0920726172348, - 10.3417374316844),	C(5.84125297807090, - 6.44418531808852),	C(10.1983256834861, 1.11415128427821),	39.3874402661408,	C(10.1983256834861, - 1.11415128427821),	C(5.84125297807090, 6.44418531808852),	C(14.0920726172348, 10.3417374316844),	C(4.09425527753994, 5.38538104020854),	C(0.898199970560132, 2.35704645157174),	C(0.458343668896671, 0.275139370348661)};
const std::complex<double> cy[] = {C(-1.20748692268504, - 0.726885133383238),	C(0.717238651328839, 0.303440924786016),	C(1.63023528916473, - 0.293871467096658),	C(0.488893770311789, 0.787282803758638),	C(1.03469300991786, - 0.888395631757642)};
const std::complex<double> cxa[] = {C(-1.11022302462516e-16, 8.88178419700125e-16),	C(4.44089209850063e-16, - 1.11022302462516e-15),	C(-0.280463333034500, 1.04392556588348),	C(4.61484501563575, 3.04664626455637),	C(4.91241210191168, 0.680374124902906),	C(4.89255277614245, - 0.772756698901497),	C(8.52373141420494, 4.70981583397090),	C(-2.77700994034861, 4.36137022536625),	C(1.57216738417938, 7.59051407354066),	C(4.41539308962119, 0.211156355648813),	C(-4.03864262273866, 2.37716617069569),	C(-1.74751789274146, - 0.683198631824812),	C(-0.411963348741099, - 0.856657657361172)};


BOOST_AUTO_TEST_SUITE(xcorr)

BOOST_AUTO_TEST_CASE(acorr_none)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L);
	float aa[A];
	xcorr(x, aa, dsp::xcorr_base::scaling_none);
	BOOST_CHECK(std::equal(aa, aa + A, a, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(acorr_biased)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L);
	float aa[A];
	xcorr(x, aa, dsp::xcorr_base::scaling_biased);
	BOOST_CHECK(std::equal(aa, aa + A, a_b, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(acorr_unbiased)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L);
	float aa[A];
	xcorr(x, aa, dsp::xcorr_base::scaling_unbiased);
	BOOST_CHECK(std::equal(aa, aa + A, a_u, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(acorr_coeff)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L);
	float aa[A];
	xcorr(x, aa, dsp::xcorr_base::scaling_coeff);
	BOOST_CHECK(std::equal(aa, aa + A, a_c, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(xcorr_none)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L, L);
	float aa[A];
	xcorr(x, y, aa, dsp::xcorr_base::scaling_none);
	BOOST_CHECK(std::equal(aa, aa + A, xa, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(xcorr_biased)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L, L);
	float aa[A];
	xcorr(x, y, aa, dsp::xcorr_base::scaling_biased);
	BOOST_CHECK(std::equal(aa, aa + A, xa_b, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(xcorr_unbiased)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L, L);
	float aa[A];
	xcorr(x, y, aa, dsp::xcorr_base::scaling_unbiased);
	BOOST_CHECK(std::equal(aa, aa + A, xa_u, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(xcorr_coeff)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<double> xcorr(L, L);
	float aa[A];
	xcorr(x, y, aa, dsp::xcorr_base::scaling_coeff);
	BOOST_CHECK(std::equal(aa, aa + A, xa_c, dsp::within_range<float>(0.00001f)));
}

BOOST_AUTO_TEST_CASE(acorr_complex_none)
{
	const size_t L = 7;
	const size_t A = 2 * L - 1;
	dsp::xcorr<std::complex<double> > xcorr(L);
	std::complex<double> aa[A];
	xcorr(cx, aa, dsp::xcorr_base::scaling_none);
	BOOST_CHECK(std::equal(aa, aa + A, ca, dsp::within_range<std::complex<double> >(0.0001f)));
}

//BOOST_AUTO_TEST_CASE(acorr_biased()
//{
//	const size_t L = 7;
//	const size_t A = 2 * L - 1;
//	dsp::xcorr<double> xcorr(L);
//	float aa[A];
//	const double* x = ::x;
//	float* out = aa;
//	xcorr(&x, &out, dsp::xcorr<double>::scaling_biased);
//	BOOST_CHECK(std::equal(aa, aa + A, a_b, dsp::within_range<float>(0.00001f)));
//}
//
//BOOST_AUTO_TEST_CASE(acorr_unbiased()
//{
//	const size_t L = 7;
//	const size_t A = 2 * L - 1;
//	dsp::xcorr<double> xcorr(L);
//	float aa[A];
//	const double* x = ::x;
//	float* out = aa;
//	xcorr(&x, &out, dsp::xcorr<double>::scaling_unbiased);
//	BOOST_CHECK(std::equal(aa, aa + A, a_u, dsp::within_range<float>(0.00001f)));
//}
//
//BOOST_AUTO_TEST_CASE(acorr_coeff()
//{
//	const size_t L = 7;
//	const size_t A = 2 * L - 1;
//	dsp::xcorr<double> xcorr(L);
//	float aa[A];
//	const double* x = ::x;
//	float* out = aa;
//	xcorr(&x, &out, dsp::xcorr<double>::scaling_coeff);
//	BOOST_CHECK(std::equal(aa, aa + A, a_c, dsp::within_range<float>(0.00001f)));
//}

BOOST_AUTO_TEST_CASE(xcorr_complex_none)
{
	const size_t Lx = 7;
	const size_t Ly = 5;
	const size_t A = 2 * Lx - 1;
	dsp::xcorr<std::complex<double> > xcorr(Lx, Ly);
	std::complex<double> aa[A];
	xcorr(cx, cy, aa, dsp::xcorr_base::scaling_none);
	BOOST_CHECK(std::equal(aa, aa + A, cxa, dsp::within_range<std::complex<double> >(0.0001f)));
}

BOOST_AUTO_TEST_SUITE_END()

