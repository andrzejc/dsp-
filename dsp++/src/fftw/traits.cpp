/*!
 * @file traits.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <dsp++/config.h>
#include <dsp++/fftw/plan_unavailable.h>

// emit vtable and type_info for the exception in this translation unit
dsp::dft::fftw::plan_unavailable::~plan_unavailable() throw() {}

#if !DSP_FFTW3_DISABLED

#include <dsp++/fft.h> // for dft_sign_forward/dft_sign_backward
#include <dsp++/fftw/allocator.h>
#include <dsp++/fftw/dft.h>
#include <boost/static_assert.hpp>
#include <fftw3.h>

using namespace dsp::dft::fftw;

void dsp::dft::fftw::detail::verify_plan_available(void* plan)
{
	if (NULL == plan)
		throw plan_unavailable();
}

namespace {
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::R2HC) == FFTW_R2HC);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::HC2R) == FFTW_HC2R);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::DHT) == FFTW_DHT);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::REDFT00) == FFTW_REDFT00);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::REDFT01) == FFTW_REDFT01);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::REDFT10) == FFTW_REDFT10);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::REDFT11) == FFTW_REDFT11);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::RODFT00) == FFTW_RODFT00);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::RODFT01) == FFTW_RODFT01);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::RODFT10) == FFTW_RODFT10);
	BOOST_STATIC_ASSERT(static_cast<int>(r2r::RODFT11) == FFTW_RODFT11);

	BOOST_STATIC_ASSERT(flag::measure == FFTW_MEASURE);
	BOOST_STATIC_ASSERT(flag::destroy_input == FFTW_DESTROY_INPUT);
	BOOST_STATIC_ASSERT(flag::unaligned == FFTW_UNALIGNED);
	BOOST_STATIC_ASSERT(flag::conserve_memory == FFTW_CONSERVE_MEMORY);
	BOOST_STATIC_ASSERT(flag::exhaustive == FFTW_EXHAUSTIVE);
	BOOST_STATIC_ASSERT(flag::preserve_input == FFTW_PRESERVE_INPUT);
	BOOST_STATIC_ASSERT(flag::patient == FFTW_PATIENT);
	BOOST_STATIC_ASSERT(flag::estimate == FFTW_ESTIMATE);
	BOOST_STATIC_ASSERT(flag::wisdom_only == FFTW_WISDOM_ONLY);

	BOOST_STATIC_ASSERT(dsp::dft::sign::forward == FFTW_FORWARD);
	BOOST_STATIC_ASSERT(dsp::dft::sign::backward == FFTW_BACKWARD);
	BOOST_STATIC_ASSERT(sizeof(int) == sizeof(unsigned));	// unsigned is (unsigned int), so it must be the same size and it should be fine to cast unsigned/signed pointers too
}

#define MANGLE(prefix, name) prefix ## name
#define COMPLEX_CAST(prefix, param) reinterpret_cast<MANGLE(prefix, complex)*>(param)
#define KIND_CAST(prefix, param) static_cast<MANGLE(prefix, r2r_kind)>(param)
#define PKIND_CAST(prefix, param) reinterpret_cast<const MANGLE(prefix, r2r_kind)*>(param)
#define INT(param) static_cast<int>(param)
#define CPINT(param) reinterpret_cast<const int*>(param)

#define DEFINE_TRAITS(prefix, type) \
BOOST_STATIC_ASSERT(sizeof(dsp::dft::fftw::r2r::kind) == sizeof(MANGLE(prefix, r2r_kind)));\
\
void traits<type>::execute(const plan_type p) {MANGLE(prefix, execute)(p);}\
\
traits<type>::plan_type traits<type>::plan_dft(size_t rank, const unsigned* n,\
		    complex_type* in, complex_type* out, dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_dft)(INT(rank), CPINT(n), COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_1d(size_t n, complex_type* in, complex_type* out, dsp::dft::sign::spec sign,\
		       unsigned flags)\
{return MANGLE(prefix, plan_dft_1d)(INT(n), COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_2d(size_t n0, size_t n1,\
		       complex_type* in, complex_type* out, dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_dft_2d)(INT(n0), INT(n1), COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_3d(size_t n0, size_t n1, size_t n2,\
		       complex_type* in, complex_type* out, dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_dft_3d)(INT(n0), INT(n1), INT(n2), COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_many_dft(size_t rank, const unsigned* n,\
                         size_t howmany,\
                         complex_type* in, const int* inembed,\
                         int istride, int idist,\
                         complex_type* out, const int* onembed,\
                         int ostride, int odist,\
                         dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_many_dft)(INT(rank), CPINT(n), INT(howmany), COMPLEX_CAST(prefix, in), inembed, istride, idist, COMPLEX_CAST(prefix, out), onembed, ostride, odist, sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_dft(size_t rank, const fftw_iodim* dims,\
			 size_t howmany_rank,\
			 const fftw_iodim* howmany_dims,\
			 complex_type* in, complex_type* out,\
			 dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_guru_dft)(INT(rank), dims, INT(howmany_rank), howmany_dims, COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_split_dft(size_t rank, const fftw_iodim* dims,\
			 size_t howmany_rank,\
			 const fftw_iodim* howmany_dims,\
			 real_type* ri, real_type* ii, real_type* ro, real_type* io,\
			 unsigned flags)\
{return MANGLE(prefix, plan_guru_split_dft)(INT(rank), dims, INT(howmany_rank), howmany_dims, ri, ii, ro, io, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_dft(size_t rank,\
                         const fftw_iodim64* dims,\
			 size_t howmany_rank,\
			 const fftw_iodim64* howmany_dims,\
			 complex_type* in, complex_type* out,\
			 dsp::dft::sign::spec sign, unsigned flags)\
{return MANGLE(prefix, plan_guru64_dft)(INT(rank), dims, INT(howmany_rank), howmany_dims, COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out), sign, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_split_dft(size_t rank,\
                         const fftw_iodim64* dims,\
			 size_t howmany_rank,\
			 const fftw_iodim64* howmany_dims,\
			 real_type* ri, real_type* ii, real_type* ro, real_type* io,\
			 unsigned flags)\
{return MANGLE(prefix, plan_guru64_split_dft)(INT(rank), dims, INT(howmany_rank), howmany_dims, ri, ii, ro, io, flags);}\
\
void traits<type>::execute_dft(const plan_type p, complex_type* in, complex_type* out)\
{MANGLE(prefix, execute_dft)(p, COMPLEX_CAST(prefix, in), COMPLEX_CAST(prefix, out));}\
\
void traits<type>::execute_split_dft(const plan_type p, real_type* ri, real_type* ii,\
                                      real_type* ro, real_type* io)\
{MANGLE(prefix, execute_split_dft)(p, ri, ii, ro, io);}\
\
traits<type>::plan_type traits<type>::plan_many_dft_r2c(size_t rank, const unsigned* n,\
                             size_t howmany,\
                             real_type* in, const int* inembed,\
                             int istride, int idist,\
                             complex_type* out, const int* onembed,\
                             int ostride, int odist,\
                             unsigned flags)\
{return MANGLE(prefix, plan_many_dft_r2c)(INT(rank), CPINT(n), INT(howmany), in, inembed, istride, idist, COMPLEX_CAST(prefix, out), onembed, ostride, odist, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_r2c(size_t rank, const unsigned* n,\
                        real_type* in, complex_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_r2c)(INT(rank), CPINT(n), in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_r2c_1d(size_t n,real_type* in,complex_type* out,unsigned flags)\
{return MANGLE(prefix, plan_dft_r2c_1d)(INT(n), in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_r2c_2d(size_t n0, size_t n1,\
			   real_type* in, complex_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_r2c_2d)(INT(n0), INT(n1), in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_r2c_3d(size_t n0, size_t n1, size_t n2,\
			   real_type* in, complex_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_r2c_3d)(INT(n0), INT(n1), INT(n2), in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_many_dft_c2r(size_t rank, const unsigned* n,\
			     size_t howmany,\
			     complex_type* in, const int* inembed,\
			     int istride, int idist,\
			     real_type* out, const int* onembed,\
			     int ostride, int odist,\
			     unsigned flags)\
{return MANGLE(prefix, plan_many_dft_c2r)(INT(rank), CPINT(n), INT(howmany), COMPLEX_CAST(prefix, in), inembed, istride, idist, out, onembed, ostride, odist, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_c2r(size_t rank, const unsigned* n,\
                        complex_type* in, real_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_c2r)(INT(rank), CPINT(n), COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_c2r_1d(size_t n, complex_type* in, real_type* out,unsigned flags)\
{return MANGLE(prefix, plan_dft_c2r_1d)(INT(n), COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_c2r_2d(size_t n0, size_t n1,\
			   complex_type* in, real_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_c2r_2d)(INT(n0), INT(n1), COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_dft_c2r_3d(size_t n0, size_t n1, size_t n2,\
			   complex_type* in, real_type* out, unsigned flags)\
{return MANGLE(prefix, plan_dft_c2r_3d)(INT(n0), INT(n1), INT(n2), COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_dft_r2c(size_t rank, const fftw_iodim* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim* howmany_dims,\
			     real_type* in, complex_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru_dft_r2c)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_dft_c2r(size_t rank, const fftw_iodim* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim* howmany_dims,\
			     complex_type* in, real_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru_dft_c2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_split_dft_r2c(\
                             size_t rank, const fftw_iodim* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim* howmany_dims,\
			     real_type* in, real_type* ro, real_type* io,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru_split_dft_r2c)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, ro, io, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_split_dft_c2r(\
                             size_t rank, const fftw_iodim* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim* howmany_dims,\
			     real_type* ri, real_type* ii, real_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru_split_dft_c2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, ri, ii, out, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_dft_r2c(size_t rank,\
                             const fftw_iodim64* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim64* howmany_dims,\
			     real_type* in, complex_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru64_dft_r2c)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, COMPLEX_CAST(prefix, out), flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_dft_c2r(size_t rank,\
                             const fftw_iodim64* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim64* howmany_dims,\
			     complex_type* in, real_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru64_dft_c2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, COMPLEX_CAST(prefix, in), out, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_split_dft_r2c(\
                             size_t rank, const fftw_iodim64* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim64* howmany_dims,\
			     real_type* in, real_type* ro, real_type* io,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru64_split_dft_r2c)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, ro, io, flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_split_dft_c2r(\
                             size_t rank, const fftw_iodim64* dims,\
			     size_t howmany_rank,\
			     const fftw_iodim64* howmany_dims,\
			     real_type* ri, real_type* ii, real_type* out,\
			     unsigned flags)\
{return MANGLE(prefix, plan_guru64_split_dft_c2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, ri, ii, out, flags);}\
\
void traits<type>::execute_dft_r2c(const plan_type p, real_type* in, complex_type* out)\
{MANGLE(prefix, execute_dft_r2c)(p, in, COMPLEX_CAST(prefix, out));}\
\
void traits<type>::execute_dft_c2r(const plan_type p, complex_type* in, real_type* out)\
{MANGLE(prefix, execute_dft_c2r)(p, COMPLEX_CAST(prefix, in), out);}\
\
void traits<type>::execute_split_dft_r2c(const plan_type p,\
                                          real_type* in, real_type* ro, real_type* io)\
{MANGLE(prefix, execute_split_dft_r2c)(p, in, ro, io);}\
\
void traits<type>::execute_split_dft_c2r(const plan_type p,\
                                          real_type* ri, real_type* ii, real_type* out)\
{MANGLE(prefix, execute_split_dft_c2r)(p, ri, ii, out);}\
\
traits<type>::plan_type traits<type>::plan_many_r2r(size_t rank, const unsigned* n,\
                         size_t howmany,\
                         real_type* in, const int* inembed,\
                         int istride, int idist,\
                         real_type* out, const int* onembed,\
                         int ostride, int odist,\
                         const r2r::kind* kind, unsigned flags)\
{return MANGLE(prefix, plan_many_r2r)(INT(rank), CPINT(n), INT(howmany), in, inembed, istride, idist, out, onembed, ostride, odist, PKIND_CAST(prefix, kind), flags);}\
\
traits<type>::plan_type traits<type>::plan_r2r(size_t rank, const unsigned* n, real_type* in, real_type* out,\
                    const r2r::kind* kind, unsigned flags)\
{return MANGLE(prefix, plan_r2r)(INT(rank), CPINT(n), in, out, PKIND_CAST(prefix, kind), flags);}\
\
traits<type>::plan_type traits<type>::plan_r2r_1d(size_t n, real_type* in, real_type* out,\
                       r2r::kind kind, unsigned flags)\
{return MANGLE(prefix, plan_r2r_1d)(INT(n), in, out, KIND_CAST(prefix, kind), flags);}\
\
traits<type>::plan_type traits<type>::plan_r2r_2d(size_t n0, size_t n1, real_type* in, real_type* out,\
                       r2r::kind kind0, r2r::kind kind1,\
                       unsigned flags)\
{return MANGLE(prefix, plan_r2r_2d)(INT(n0), INT(n1), in, out, KIND_CAST(prefix, kind0), KIND_CAST(prefix, kind1), flags);}\
\
traits<type>::plan_type traits<type>::plan_r2r_3d(size_t n0, size_t n1, size_t n2,\
                       real_type* in, real_type* out, r2r::kind kind0,\
                       r2r::kind kind1, r2r::kind kind2,\
                       unsigned flags)\
{return MANGLE(prefix, plan_r2r_3d)(INT(n0), INT(n1), INT(n2), in, out, KIND_CAST(prefix, kind0), KIND_CAST(prefix, kind1), KIND_CAST(prefix, kind2), flags);}\
\
traits<type>::plan_type traits<type>::plan_guru_r2r(size_t rank, const fftw_iodim* dims,\
                         size_t howmany_rank,\
                         const fftw_iodim* howmany_dims,\
                         real_type* in, real_type* out,\
                         const r2r::kind* kind, unsigned flags)\
{return MANGLE(prefix, plan_guru_r2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, out, PKIND_CAST(prefix, kind), flags);}\
\
traits<type>::plan_type traits<type>::plan_guru64_r2r(size_t rank, const fftw_iodim64* dims,\
                         size_t howmany_rank,\
                         const fftw_iodim64* howmany_dims,\
                         real_type* in, real_type* out,\
                         const r2r::kind* kind, unsigned flags)\
{return MANGLE(prefix, plan_guru64_r2r)(INT(rank), dims, INT(howmany_rank), howmany_dims, in, out, PKIND_CAST(prefix, kind), flags);}\
\
void traits<type>::execute_r2r(const plan_type p, real_type* in, real_type* out)\
{MANGLE(prefix, execute_r2r)(p, in, out);}\
\
void traits<type>::destroy_plan(plan_type p)\
{MANGLE(prefix, destroy_plan)(p);}\
\
void traits<type>::forget_wisdom(void)\
{MANGLE(prefix, forget_wisdom)();}\
\
void traits<type>::cleanup(void)\
{MANGLE(prefix, cleanup)();}\
\
void traits<type>::set_timelimit(double t)\
{MANGLE(prefix, set_timelimit)(t);}\
\
void traits<type>::plan_with_nthreads(unsigned nthreads)\
{MANGLE(prefix, plan_with_nthreads)(INT(nthreads));}\
\
int traits<type>::init_threads(void)\
{return MANGLE(prefix, init_threads)();}\
\
void traits<type>::cleanup_threads(void)\
{MANGLE(prefix, cleanup_threads)();}\
\
int traits<type>::export_wisdom_to_filename(const char* filename)\
{return MANGLE(prefix, export_wisdom_to_filename)(filename);}\
\
void traits<type>::export_wisdom_to_file(FILE* output_file)\
{MANGLE(prefix, export_wisdom_to_file)(output_file);}\
\
char* traits<type>::export_wisdom_to_string(void)\
{return MANGLE(prefix, export_wisdom_to_string)();}\
\
void traits<type>::export_wisdom(fftw_write_char_func write_char,\
                                  void* data)\
{MANGLE(prefix, export_wisdom)(write_char, data);}\
\
int traits<type>::import_system_wisdom(void)\
{return MANGLE(prefix, import_system_wisdom)();}\
\
int traits<type>::import_wisdom_from_filename(const char* filename)\
{return MANGLE(prefix, import_wisdom_from_filename)(filename);}\
\
int traits<type>::import_wisdom_from_file(FILE* input_file)\
{return MANGLE(prefix, import_wisdom_from_file)(input_file);}\
\
int traits<type>::import_wisdom_from_string(const char* input_string)\
{return MANGLE(prefix, import_wisdom_from_string)(input_string);}\
\
int traits<type>::import_wisdom(fftw_read_char_func read_char, void* data)\
{return MANGLE(prefix, import_wisdom)(read_char, data);}\
\
void traits<type>::fprint_plan(const plan_type p, FILE* output_file)\
{MANGLE(prefix, fprint_plan)(p, output_file);}\
\
void traits<type>::print_plan(const plan_type p)\
{MANGLE(prefix, print_plan)(p);}\
\
void* traits<type>::malloc(size_t n)\
{return MANGLE(prefix, malloc)(n);}\
\
traits<type>::real_type* traits<type>::alloc_real(size_t n)\
{return MANGLE(prefix, alloc_real)(n);}\
\
traits<type>::complex_type* traits<type>::alloc_complex(size_t n)\
{return reinterpret_cast<complex_type*>(MANGLE(prefix, alloc_complex)(n));}\
\
void traits<type>::free(void* p)\
{MANGLE(prefix, free)(p);}\
\
void traits<type>::flops(const plan_type p,\
                          double* add, double* mul, double* fmas)\
{MANGLE(prefix, flops)(p, add, mul, fmas);}\
\
double traits<type>::estimate_cost(const plan_type p)\
{return MANGLE(prefix, estimate_cost)(p);}\
\
double traits<type>::cost(const plan_type p)\
{return MANGLE(prefix, cost)(p);} \
\
template class DSPXX_API dsp::dft::fftw::dft<type, type>; \
template class DSPXX_API dsp::dft::fftw::dft<type, std::complex<type> >; \
template class DSPXX_API dsp::dft::fftw::dft<std::complex<type>, type>; \
template class DSPXX_API dsp::dft::fftw::dft<std::complex<type>, std::complex<type> >;
    
#if DSP_FFTW_HAVE_FLOAT
DEFINE_TRAITS(fftwf_, float)
#endif // DSP_FFTW_HAVE_FLOAT

#if DSP_FFTW_HAVE_DOUBLE
DEFINE_TRAITS(fftw_, double)
#endif // DSP_FFTW_HAVE_FLOAT

#if DSP_FFTW_HAVE_LONG_DOUBLE
DEFINE_TRAITS(fftwl_, long double)
#endif // DSP_FFTW_HAVE_FLOAT

#if DSP_FFTW_HAVE_QUAD
DEFINE_TRAITS(fftwq_, quad)
#endif // DSP_FFTW_HAVE_FLOAT

void* allocator_base::allocate(size_t n)
{
	void* p =
#if DSP_FFTW_HAVE_FLOAT
			traits<float>::malloc(n);
#elif DSP_FFTW_HAVE_DOUBLE
			traits<double>::malloc(n);
#elif DSP_FFTW_HAVE_LONG_DOUBLE
			traits<long double>::malloc(n);
#elif DSP_FFTW_HAVE_QUAD
			traits<quad>::malloc(n);
#else
			malloc(n);
#endif
	if (NULL == p)
		throw std::bad_alloc();
	return p;
}

void allocator_base::deallocate(void* p)
{
#if DSP_FFTW_HAVE_FLOAT
			traits<float>::free(p);
#elif DSP_FFTW_HAVE_DOUBLE
			traits<double>::free(p);
#elif DSP_FFTW_HAVE_LONG_DOUBLE
			traits<long double>::free(p);
#elif DSP_FFTW_HAVE_QUAD
			traits<quad>::free(p);
#else
			free(p);
#endif
}
    


#endif // !DSP_FFTW3_DISABLED

