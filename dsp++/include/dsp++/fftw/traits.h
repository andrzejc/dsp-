/*!
 * @file traits.h
 * @brief Traits classes for templated use of fftw.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_FFTW_TRAITS_H_INCLUDED
#define DSP_FFTW_TRAITS_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_FFTW_DISABLED

#include <dsp++/export.h>
#include <complex>
#include <cstdio>

#if DSP_FFTW_HAVE_FLOAT
typedef struct fftwf_plan_s* fftwf_plan;
#endif

#if DSP_FFTW_HAVE_DOUBLE
typedef struct fftw_plan_s* fftw_plan;
#endif

#if DSP_FFTW_HAVE_LONG_DOUBLE
typedef struct fftwl_plan_s* fftwl_plan;
#endif

#if DSP_FFTW_HAVE_QUAD
typedef struct fftwq_plan_s* fftwq_plan;
#endif

typedef struct fftw_iodim64_do_not_use_me fftw_iodim64;
typedef struct fftw_iodim_do_not_use_me fftw_iodim;
typedef void (*fftw_write_char_func)(char c, void* );
typedef int (*fftw_read_char_func)(void* );

namespace dsp { namespace fftw {

	//! @brief Implementation details. Do not use.
	namespace detail {
		void DSPXX_API verify_plan_available(void* plan);
	}

	/*!
	 * @brief Kind of real-to-real transform.
	 * This is a direct mapping of libfftw3 fftw_r2r_kind enum, with validation
	 * performed through @c BOOST_STATIC_ASSERT() in traits.cpp.
	 * @see fftw_r2r_kind
	 */
	enum r2r_kind
	{
		kind_R2HC=0,
		kind_HC2R=1,
		kind_DHT=2,
		kind_REDFT00=3,
		kind_REDFT01=4,
		kind_REDFT10=5,
		kind_REDFT11=6,
		kind_RODFT00=7,
		kind_RODFT01=8,
		kind_RODFT10=9,
		kind_RODFT11=10
	};

	/*!
	 * @brief Traits class representing whole libfftw3 API for a specific real type.
	 * @tparam Real real type, APIs for which will be used.
	 */
	template<class Real>
	class traits {
	public:
		typedef Real real_type;
		//! @brief C++ complex type for the given @c real_type.
		typedef std::complex<Real> complex_type;
		typedef void* plan_type;
		typedef void plan_value_type;

		static void execute(const plan_type p);

		static plan_type plan_dft(int rank, const int* n,
				    complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_dft_1d(int n, complex_type* in, complex_type* out, int sign,
				       unsigned flags);
		static plan_type plan_dft_2d(int n0, int n1,
				       complex_type* in, complex_type* out, int sign, unsigned flags);
		static plan_type plan_dft_3d(int n0, int n1, int n2,
				       complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_many_dft(int rank, const int* n,
		                         int howmany,
		                         complex_type* in, const int* inembed,
		                         int istride, int idist,
		                         complex_type* out, const int* onembed,
		                         int ostride, int odist,
		                         int sign, unsigned flags);

		static plan_type plan_guru_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru_split_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static plan_type plan_guru64_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru64_split_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static void execute_dft(const plan_type p, complex_type* in, complex_type* out);
		static void execute_split_dft(const plan_type p, real_type* ri, real_type* ii,
		                                      real_type* ro, real_type* io);

		static plan_type plan_many_dft_r2c(int rank, const int* n,
		                             int howmany,
		                             real_type* in, const int* inembed,
		                             int istride, int idist,
		                             complex_type* out, const int* onembed,
		                             int ostride, int odist,
		                             unsigned flags);

		static plan_type plan_dft_r2c(int rank, const int* n,
		                        real_type* in, complex_type* out, unsigned flags);

		static plan_type plan_dft_r2c_1d(int n,real_type* in,complex_type* out,unsigned flags);
		static plan_type plan_dft_r2c_2d(int n0, int n1,
					   real_type* in, complex_type* out, unsigned flags);
		static plan_type plan_dft_r2c_3d(int n0, int n1,
					   int n2,
					   real_type* in, complex_type* out, unsigned flags);


		static plan_type plan_many_dft_c2r(int rank, const int* n,
					     int howmany,
					     complex_type* in, const int* inembed,
					     int istride, int idist,
					     real_type* out, const int* onembed,
					     int ostride, int odist,
					     unsigned flags);

		static plan_type plan_dft_c2r(int rank, const int* n,
		                        complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_dft_c2r_1d(int n,complex_type* in,real_type* out,unsigned flags);
		static plan_type plan_dft_c2r_2d(int n0, int n1,
					   complex_type* in, real_type* out, unsigned flags);
		static plan_type plan_dft_c2r_3d(int n0, int n1,
					   int n2,
					   complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_guru_dft_r2c(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru_dft_c2r(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru_split_dft_r2c(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru_split_dft_c2r(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_dft_r2c(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru64_dft_c2r(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_split_dft_r2c(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru64_split_dft_c2r(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static void execute_dft_r2c(const plan_type p, real_type* in, complex_type* out);
		static void execute_dft_c2r(const plan_type p, complex_type* in, real_type* out);

		static void execute_split_dft_r2c(const plan_type p,
		                                          real_type* in, real_type* ro, real_type* io);
		static void execute_split_dft_c2r(const plan_type p,
		                                          real_type* ri, real_type* ii, real_type* out);

		static plan_type plan_many_r2r(int rank, const int* n,
		                         int howmany,
		                         real_type* in, const int* inembed,
		                         int istride, int idist,
		                         real_type* out, const int* onembed,
		                         int ostride, int odist,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r(int rank, const int* n, real_type* in, real_type* out,
		                    const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r_1d(int n, real_type* in, real_type* out,
		                       r2r_kind kind, unsigned flags);
		static plan_type plan_r2r_2d(int n0, int n1, real_type* in, real_type* out,
		                       r2r_kind kind0, r2r_kind kind1,
		                       unsigned flags);
		static plan_type plan_r2r_3d(int n0, int n1, int n2,
		                       real_type* in, real_type* out, r2r_kind kind0,
		                       r2r_kind kind1, r2r_kind kind2,
		                       unsigned flags);

		static plan_type plan_guru_r2r(int rank, const fftw_iodim* dims,
		                         int howmany_rank,
		                         const fftw_iodim* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_guru64_r2r(int rank, const fftw_iodim64* dims,
		                         int howmany_rank,
		                         const fftw_iodim64* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static void execute_r2r(const plan_type p, real_type* in, real_type* out);

		static void destroy_plan(plan_type p);
		static void forget_wisdom(void);
		static void cleanup(void);

		static void set_timelimit(double t);

		static void plan_with_nthreads(int nthreads);
		static int init_threads(void);
		static void cleanup_threads(void);

		static int export_wisdom_to_filename(const char* filename);
		static void export_wisdom_to_file(FILE* output_file);
		static char* export_wisdom_to_string(void);
		static void export_wisdom(fftw_write_char_func write_char,
		                                  void* data);
		static int import_system_wisdom(void);
		static int import_wisdom_from_filename(const char* filename);
		static int import_wisdom_from_file(FILE* input_file);
		static int import_wisdom_from_string(const char* input_string);
		static int import_wisdom(fftw_read_char_func read_char, void* data);

		static void fprint_plan(const plan_type p, FILE* output_file);
		static void print_plan(const plan_type p);

		static void* malloc(size_t n);
		static real_type* alloc_real(size_t n);
		static complex_type* alloc_complex(size_t n);
		static void free(void* p);

		static void flops(const plan_type p,
		                          double* add, double* mul, double* fmas);
		static double estimate_cost(const plan_type p);
		static double cost(const plan_type p);
	};

#if DSP_FFTW_HAVE_FLOAT
	/*!
	 * @brief Explicit specialization of traits template for type @c float.
	 */
	template<>
	class DSPXX_API traits<float> {
	public:
		typedef float real_type;
		typedef std::complex<float> complex_type;
		typedef fftwf_plan plan_type;
		typedef fftwf_plan_s plan_value_type;

		static void execute(const plan_type p);

		static plan_type plan_dft(int rank, const int* n,
				    complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_dft_1d(int n, complex_type* in, complex_type* out, int sign,
				       unsigned flags);
		static plan_type plan_dft_2d(int n0, int n1,
				       complex_type* in, complex_type* out, int sign, unsigned flags);
		static plan_type plan_dft_3d(int n0, int n1, int n2,
				       complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_many_dft(int rank, const int* n,
		                         int howmany,
		                         complex_type* in, const int* inembed,
		                         int istride, int idist,
		                         complex_type* out, const int* onembed,
		                         int ostride, int odist,
		                         int sign, unsigned flags);

		static plan_type plan_guru_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru_split_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static plan_type plan_guru64_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru64_split_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static void execute_dft(const plan_type p, complex_type* in, complex_type* out);
		static void execute_split_dft(const plan_type p, real_type* ri, real_type* ii,
		                                      real_type* ro, real_type* io);

		static plan_type plan_many_dft_r2c(int rank, const int* n,
		                             int howmany,
		                             real_type* in, const int* inembed,
		                             int istride, int idist,
		                             complex_type* out, const int* onembed,
		                             int ostride, int odist,
		                             unsigned flags);

		static plan_type plan_dft_r2c(int rank, const int* n,
		                        real_type* in, complex_type* out, unsigned flags);

		static plan_type plan_dft_r2c_1d(int n,real_type* in,complex_type* out,unsigned flags);
		static plan_type plan_dft_r2c_2d(int n0, int n1,
					   real_type* in, complex_type* out, unsigned flags);
		static plan_type plan_dft_r2c_3d(int n0, int n1,
					   int n2,
					   real_type* in, complex_type* out, unsigned flags);


		static plan_type plan_many_dft_c2r(int rank, const int* n,
					     int howmany,
					     complex_type* in, const int* inembed,
					     int istride, int idist,
					     real_type* out, const int* onembed,
					     int ostride, int odist,
					     unsigned flags);

		static plan_type plan_dft_c2r(int rank, const int* n,
		                        complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_dft_c2r_1d(int n,complex_type* in,real_type* out,unsigned flags);
		static plan_type plan_dft_c2r_2d(int n0, int n1,
					   complex_type* in, real_type* out, unsigned flags);
		static plan_type plan_dft_c2r_3d(int n0, int n1,
					   int n2,
					   complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_guru_dft_r2c(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru_dft_c2r(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru_split_dft_r2c(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru_split_dft_c2r(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_dft_r2c(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru64_dft_c2r(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_split_dft_r2c(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru64_split_dft_c2r(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static void execute_dft_r2c(const plan_type p, real_type* in, complex_type* out);
		static void execute_dft_c2r(const plan_type p, complex_type* in, real_type* out);

		static void execute_split_dft_r2c(const plan_type p,
		                                          real_type* in, real_type* ro, real_type* io);
		static void execute_split_dft_c2r(const plan_type p,
		                                          real_type* ri, real_type* ii, real_type* out);

		static plan_type plan_many_r2r(int rank, const int* n,
		                         int howmany,
		                         real_type* in, const int* inembed,
		                         int istride, int idist,
		                         real_type* out, const int* onembed,
		                         int ostride, int odist,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r(int rank, const int* n, real_type* in, real_type* out,
		                    const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r_1d(int n, real_type* in, real_type* out,
		                       r2r_kind kind, unsigned flags);
		static plan_type plan_r2r_2d(int n0, int n1, real_type* in, real_type* out,
		                       r2r_kind kind0, r2r_kind kind1,
		                       unsigned flags);
		static plan_type plan_r2r_3d(int n0, int n1, int n2,
		                       real_type* in, real_type* out, r2r_kind kind0,
		                       r2r_kind kind1, r2r_kind kind2,
		                       unsigned flags);

		static plan_type plan_guru_r2r(int rank, const fftw_iodim* dims,
		                         int howmany_rank,
		                         const fftw_iodim* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_guru64_r2r(int rank, const fftw_iodim64* dims,
		                         int howmany_rank,
		                         const fftw_iodim64* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static void execute_r2r(const plan_type p, real_type* in, real_type* out);

		static void destroy_plan(plan_type p);
		static void forget_wisdom(void);
		static void cleanup(void);

		static void set_timelimit(double t);

		static void plan_with_nthreads(int nthreads);
		static int init_threads(void);
		static void cleanup_threads(void);

		static int export_wisdom_to_filename(const char* filename);
		static void export_wisdom_to_file(FILE* output_file);
		static char* export_wisdom_to_string(void);
		static void export_wisdom(fftw_write_char_func write_char,
		                                  void* data);
		static int import_system_wisdom(void);
		static int import_wisdom_from_filename(const char* filename);
		static int import_wisdom_from_file(FILE* input_file);
		static int import_wisdom_from_string(const char* input_string);
		static int import_wisdom(fftw_read_char_func read_char, void* data);

		static void fprint_plan(const plan_type p, FILE* output_file);
		static void print_plan(const plan_type p);

		static void* malloc(size_t n);
		static real_type* alloc_real(size_t n);
		static complex_type* alloc_complex(size_t n);
		static void free(void* p);

		static void flops(const plan_type p,
		                          double* add, double* mul, double* fmas);
		static double estimate_cost(const plan_type p);
		static double cost(const plan_type p);
	};
#endif // DSP_FFTW_HAVE_FLOAT

#if DSP_FFTW_HAVE_DOUBLE
	/*!
	 * @brief Explicit specialization of traits template for type @c double.
	 */
	template<>
	class DSPXX_API traits<double> {
	public:
		typedef double real_type;
		typedef std::complex<double> complex_type;
		typedef fftw_plan plan_type;
		typedef fftw_plan_s plan_value_type;

		static void execute(const plan_type p);

		static plan_type plan_dft(int rank, const int* n,
				    complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_dft_1d(int n, complex_type* in, complex_type* out, int sign,
				       unsigned flags);
		static plan_type plan_dft_2d(int n0, int n1,
				       complex_type* in, complex_type* out, int sign, unsigned flags);
		static plan_type plan_dft_3d(int n0, int n1, int n2,
				       complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_many_dft(int rank, const int* n,
		                         int howmany,
		                         complex_type* in, const int* inembed,
		                         int istride, int idist,
		                         complex_type* out, const int* onembed,
		                         int ostride, int odist,
		                         int sign, unsigned flags);

		static plan_type plan_guru_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru_split_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static plan_type plan_guru64_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru64_split_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static void execute_dft(const plan_type p, complex_type* in, complex_type* out);
		static void execute_split_dft(const plan_type p, real_type* ri, real_type* ii,
		                                      real_type* ro, real_type* io);

		static plan_type plan_many_dft_r2c(int rank, const int* n,
		                             int howmany,
		                             real_type* in, const int* inembed,
		                             int istride, int idist,
		                             complex_type* out, const int* onembed,
		                             int ostride, int odist,
		                             unsigned flags);

		static plan_type plan_dft_r2c(int rank, const int* n,
		                        real_type* in, complex_type* out, unsigned flags);

		static plan_type plan_dft_r2c_1d(int n,real_type* in,complex_type* out,unsigned flags);
		static plan_type plan_dft_r2c_2d(int n0, int n1,
					   real_type* in, complex_type* out, unsigned flags);
		static plan_type plan_dft_r2c_3d(int n0, int n1,
					   int n2,
					   real_type* in, complex_type* out, unsigned flags);


		static plan_type plan_many_dft_c2r(int rank, const int* n,
					     int howmany,
					     complex_type* in, const int* inembed,
					     int istride, int idist,
					     real_type* out, const int* onembed,
					     int ostride, int odist,
					     unsigned flags);

		static plan_type plan_dft_c2r(int rank, const int* n,
		                        complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_dft_c2r_1d(int n,complex_type* in,real_type* out,unsigned flags);
		static plan_type plan_dft_c2r_2d(int n0, int n1,
					   complex_type* in, real_type* out, unsigned flags);
		static plan_type plan_dft_c2r_3d(int n0, int n1,
					   int n2,
					   complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_guru_dft_r2c(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru_dft_c2r(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru_split_dft_r2c(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru_split_dft_c2r(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_dft_r2c(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru64_dft_c2r(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_split_dft_r2c(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru64_split_dft_c2r(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static void execute_dft_r2c(const plan_type p, real_type* in, complex_type* out);
		static void execute_dft_c2r(const plan_type p, complex_type* in, real_type* out);

		static void execute_split_dft_r2c(const plan_type p,
		                                          real_type* in, real_type* ro, real_type* io);
		static void execute_split_dft_c2r(const plan_type p,
		                                          real_type* ri, real_type* ii, real_type* out);

		static plan_type plan_many_r2r(int rank, const int* n,
		                         int howmany,
		                         real_type* in, const int* inembed,
		                         int istride, int idist,
		                         real_type* out, const int* onembed,
		                         int ostride, int odist,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r(int rank, const int* n, real_type* in, real_type* out,
		                    const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r_1d(int n, real_type* in, real_type* out,
		                       r2r_kind kind, unsigned flags);
		static plan_type plan_r2r_2d(int n0, int n1, real_type* in, real_type* out,
		                       r2r_kind kind0, r2r_kind kind1,
		                       unsigned flags);
		static plan_type plan_r2r_3d(int n0, int n1, int n2,
		                       real_type* in, real_type* out, r2r_kind kind0,
		                       r2r_kind kind1, r2r_kind kind2,
		                       unsigned flags);

		static plan_type plan_guru_r2r(int rank, const fftw_iodim* dims,
		                         int howmany_rank,
		                         const fftw_iodim* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_guru64_r2r(int rank, const fftw_iodim64* dims,
		                         int howmany_rank,
		                         const fftw_iodim64* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static void execute_r2r(const plan_type p, real_type* in, real_type* out);

		static void destroy_plan(plan_type p);
		static void forget_wisdom(void);
		static void cleanup(void);

		static void set_timelimit(double t);

		static void plan_with_nthreads(int nthreads);
		static int init_threads(void);
		static void cleanup_threads(void);

		static int export_wisdom_to_filename(const char* filename);
		static void export_wisdom_to_file(FILE* output_file);
		static char* export_wisdom_to_string(void);
		static void export_wisdom(fftw_write_char_func write_char,
		                                  void* data);
		static int import_system_wisdom(void);
		static int import_wisdom_from_filename(const char* filename);
		static int import_wisdom_from_file(FILE* input_file);
		static int import_wisdom_from_string(const char* input_string);
		static int import_wisdom(fftw_read_char_func read_char, void* data);

		static void fprint_plan(const plan_type p, FILE* output_file);
		static void print_plan(const plan_type p);

		static void* malloc(size_t n);
		static real_type* alloc_real(size_t n);
		static complex_type* alloc_complex(size_t n);
		static void free(void* p);

		static void flops(const plan_type p,
		                          double* add, double* mul, double* fmas);
		static double estimate_cost(const plan_type p);
		static double cost(const plan_type p);
	};
#endif // DSP_FFTW_HAVE_DOUBLE

#if DSP_FFTW_HAVE_LONG_DOUBLE
	/*!
	 * @brief Explicit specialization of traits template for type @c long double.
	 */
	template<>
	class DSPXX_API traits<long double> {
	public:
		typedef long double real_type;
		typedef std::complex<long double> complex_type;
		typedef fftwl_plan plan_type;
		typedef fftwl_plan_s plan_value_type;

		static void execute(const plan_type p);

		static plan_type plan_dft(int rank, const int* n,
				    complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_dft_1d(int n, complex_type* in, complex_type* out, int sign,
				       unsigned flags);
		static plan_type plan_dft_2d(int n0, int n1,
				       complex_type* in, complex_type* out, int sign, unsigned flags);
		static plan_type plan_dft_3d(int n0, int n1, int n2,
				       complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_many_dft(int rank, const int* n,
		                         int howmany,
		                         complex_type* in, const int* inembed,
		                         int istride, int idist,
		                         complex_type* out, const int* onembed,
		                         int ostride, int odist,
		                         int sign, unsigned flags);

		static plan_type plan_guru_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru_split_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static plan_type plan_guru64_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru64_split_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static void execute_dft(const plan_type p, complex_type* in, complex_type* out);
		static void execute_split_dft(const plan_type p, real_type* ri, real_type* ii,
		                                      real_type* ro, real_type* io);

		static plan_type plan_many_dft_r2c(int rank, const int* n,
		                             int howmany,
		                             real_type* in, const int* inembed,
		                             int istride, int idist,
		                             complex_type* out, const int* onembed,
		                             int ostride, int odist,
		                             unsigned flags);

		static plan_type plan_dft_r2c(int rank, const int* n,
		                        real_type* in, complex_type* out, unsigned flags);

		static plan_type plan_dft_r2c_1d(int n,real_type* in,complex_type* out,unsigned flags);
		static plan_type plan_dft_r2c_2d(int n0, int n1,
					   real_type* in, complex_type* out, unsigned flags);
		static plan_type plan_dft_r2c_3d(int n0, int n1,
					   int n2,
					   real_type* in, complex_type* out, unsigned flags);


		static plan_type plan_many_dft_c2r(int rank, const int* n,
					     int howmany,
					     complex_type* in, const int* inembed,
					     int istride, int idist,
					     real_type* out, const int* onembed,
					     int ostride, int odist,
					     unsigned flags);

		static plan_type plan_dft_c2r(int rank, const int* n,
		                        complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_dft_c2r_1d(int n,complex_type* in,real_type* out,unsigned flags);
		static plan_type plan_dft_c2r_2d(int n0, int n1,
					   complex_type* in, real_type* out, unsigned flags);
		static plan_type plan_dft_c2r_3d(int n0, int n1,
					   int n2,
					   complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_guru_dft_r2c(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru_dft_c2r(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru_split_dft_r2c(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru_split_dft_c2r(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_dft_r2c(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru64_dft_c2r(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_split_dft_r2c(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru64_split_dft_c2r(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static void execute_dft_r2c(const plan_type p, real_type* in, complex_type* out);
		static void execute_dft_c2r(const plan_type p, complex_type* in, real_type* out);

		static void execute_split_dft_r2c(const plan_type p,
		                                          real_type* in, real_type* ro, real_type* io);
		static void execute_split_dft_c2r(const plan_type p,
		                                          real_type* ri, real_type* ii, real_type* out);

		static plan_type plan_many_r2r(int rank, const int* n,
		                         int howmany,
		                         real_type* in, const int* inembed,
		                         int istride, int idist,
		                         real_type* out, const int* onembed,
		                         int ostride, int odist,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r(int rank, const int* n, real_type* in, real_type* out,
		                    const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r_1d(int n, real_type* in, real_type* out,
		                       r2r_kind kind, unsigned flags);
		static plan_type plan_r2r_2d(int n0, int n1, real_type* in, real_type* out,
		                       r2r_kind kind0, r2r_kind kind1,
		                       unsigned flags);
		static plan_type plan_r2r_3d(int n0, int n1, int n2,
		                       real_type* in, real_type* out, r2r_kind kind0,
		                       r2r_kind kind1, r2r_kind kind2,
		                       unsigned flags);

		static plan_type plan_guru_r2r(int rank, const fftw_iodim* dims,
		                         int howmany_rank,
		                         const fftw_iodim* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_guru64_r2r(int rank, const fftw_iodim64* dims,
		                         int howmany_rank,
		                         const fftw_iodim64* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static void execute_r2r(const plan_type p, real_type* in, real_type* out);

		static void destroy_plan(plan_type p);
		static void forget_wisdom(void);
		static void cleanup(void);

		static void set_timelimit(double t);

		static void plan_with_nthreads(int nthreads);
		static int init_threads(void);
		static void cleanup_threads(void);

		static int export_wisdom_to_filename(const char* filename);
		static void export_wisdom_to_file(FILE* output_file);
		static char* export_wisdom_to_string(void);
		static void export_wisdom(fftw_write_char_func write_char,
		                                  void* data);
		static int import_system_wisdom(void);
		static int import_wisdom_from_filename(const char* filename);
		static int import_wisdom_from_file(FILE* input_file);
		static int import_wisdom_from_string(const char* input_string);
		static int import_wisdom(fftw_read_char_func read_char, void* data);

		static void fprint_plan(const plan_type p, FILE* output_file);
		static void print_plan(const plan_type p);

		static void* malloc(size_t n);
		static real_type* alloc_real(size_t n);
		static complex_type* alloc_complex(size_t n);
		static void free(void* p);

		static void flops(const plan_type p,
		                          double* add, double* mul, double* fmas);
		static double estimate_cost(const plan_type p);
		static double cost(const plan_type p);
	};
#endif // DSP_FFTW_HAVE_LONG_DOUBLE

#if DSP_FFTW_HAVE_QUAD
	typedef __float128 quad;
	/*!
	 * @brief Explicit specialization of traits template for type @c quad.
	 */
	template<>
	class DSPXX_API traits<quad> {
	public:
		typedef long double real_type;
		typedef std::complex<quad> complex_type;
		typedef fftwq_plan plan_type;
		typedef fftwq_plan_s plan_value_type;

		static void execute(const plan_type p);

		static plan_type plan_dft(int rank, const int* n,
				    complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_dft_1d(int n, complex_type* in, complex_type* out, int sign,
				       unsigned flags);
		static plan_type plan_dft_2d(int n0, int n1,
				       complex_type* in, complex_type* out, int sign, unsigned flags);
		static plan_type plan_dft_3d(int n0, int n1, int n2,
				       complex_type* in, complex_type* out, int sign, unsigned flags);

		static plan_type plan_many_dft(int rank, const int* n,
		                         int howmany,
		                         complex_type* in, const int* inembed,
		                         int istride, int idist,
		                         complex_type* out, const int* onembed,
		                         int ostride, int odist,
		                         int sign, unsigned flags);

		static plan_type plan_guru_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru_split_dft(int rank, const fftw_iodim* dims,
					 int howmany_rank,
					 const fftw_iodim* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static plan_type plan_guru64_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 complex_type* in, complex_type* out,
					 int sign, unsigned flags);
		static plan_type plan_guru64_split_dft(int rank,
		                         const fftw_iodim64* dims,
					 int howmany_rank,
					 const fftw_iodim64* howmany_dims,
					 real_type* ri, real_type* ii, real_type* ro, real_type* io,
					 unsigned flags);

		static void execute_dft(const plan_type p, complex_type* in, complex_type* out);
		static void execute_split_dft(const plan_type p, real_type* ri, real_type* ii,
		                                      real_type* ro, real_type* io);

		static plan_type plan_many_dft_r2c(int rank, const int* n,
		                             int howmany,
		                             real_type* in, const int* inembed,
		                             int istride, int idist,
		                             complex_type* out, const int* onembed,
		                             int ostride, int odist,
		                             unsigned flags);

		static plan_type plan_dft_r2c(int rank, const int* n,
		                        real_type* in, complex_type* out, unsigned flags);

		static plan_type plan_dft_r2c_1d(int n,real_type* in,complex_type* out,unsigned flags);
		static plan_type plan_dft_r2c_2d(int n0, int n1,
					   real_type* in, complex_type* out, unsigned flags);
		static plan_type plan_dft_r2c_3d(int n0, int n1,
					   int n2,
					   real_type* in, complex_type* out, unsigned flags);


		static plan_type plan_many_dft_c2r(int rank, const int* n,
					     int howmany,
					     complex_type* in, const int* inembed,
					     int istride, int idist,
					     real_type* out, const int* onembed,
					     int ostride, int odist,
					     unsigned flags);

		static plan_type plan_dft_c2r(int rank, const int* n,
		                        complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_dft_c2r_1d(int n,complex_type* in,real_type* out,unsigned flags);
		static plan_type plan_dft_c2r_2d(int n0, int n1,
					   complex_type* in, real_type* out, unsigned flags);
		static plan_type plan_dft_c2r_3d(int n0, int n1,
					   int n2,
					   complex_type* in, real_type* out, unsigned flags);

		static plan_type plan_guru_dft_r2c(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru_dft_c2r(int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru_split_dft_r2c(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru_split_dft_c2r(
		                             int rank, const fftw_iodim* dims,
					     int howmany_rank,
					     const fftw_iodim* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_dft_r2c(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, complex_type* out,
					     unsigned flags);
		static plan_type plan_guru64_dft_c2r(int rank,
		                             const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     complex_type* in, real_type* out,
					     unsigned flags);

		static plan_type plan_guru64_split_dft_r2c(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* in, real_type* ro, real_type* io,
					     unsigned flags);
		static plan_type plan_guru64_split_dft_c2r(
		                             int rank, const fftw_iodim64* dims,
					     int howmany_rank,
					     const fftw_iodim64* howmany_dims,
					     real_type* ri, real_type* ii, real_type* out,
					     unsigned flags);

		static void execute_dft_r2c(const plan_type p, real_type* in, complex_type* out);
		static void execute_dft_c2r(const plan_type p, complex_type* in, real_type* out);

		static void execute_split_dft_r2c(const plan_type p,
		                                          real_type* in, real_type* ro, real_type* io);
		static void execute_split_dft_c2r(const plan_type p,
		                                          real_type* ri, real_type* ii, real_type* out);

		static plan_type plan_many_r2r(int rank, const int* n,
		                         int howmany,
		                         real_type* in, const int* inembed,
		                         int istride, int idist,
		                         real_type* out, const int* onembed,
		                         int ostride, int odist,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r(int rank, const int* n, real_type* in, real_type* out,
		                    const r2r_kind* k, unsigned flags);

		static plan_type plan_r2r_1d(int n, real_type* in, real_type* out,
		                       r2r_kind kind, unsigned flags);
		static plan_type plan_r2r_2d(int n0, int n1, real_type* in, real_type* out,
		                       r2r_kind kind0, r2r_kind kind1,
		                       unsigned flags);
		static plan_type plan_r2r_3d(int n0, int n1, int n2,
		                       real_type* in, real_type* out, r2r_kind kind0,
		                       r2r_kind kind1, r2r_kind kind2,
		                       unsigned flags);

		static plan_type plan_guru_r2r(int rank, const fftw_iodim* dims,
		                         int howmany_rank,
		                         const fftw_iodim* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static plan_type plan_guru64_r2r(int rank, const fftw_iodim64* dims,
		                         int howmany_rank,
		                         const fftw_iodim64* howmany_dims,
		                         real_type* in, real_type* out,
		                         const r2r_kind* k, unsigned flags);

		static void execute_r2r(const plan_type p, real_type* in, real_type* out);

		static void destroy_plan(plan_type p);
		static void forget_wisdom(void);
		static void cleanup(void);

		static void set_timelimit(double t);

		static void plan_with_nthreads(int nthreads);
		static int init_threads(void);
		static void cleanup_threads(void);

		static int export_wisdom_to_filename(const char* filename);
		static void export_wisdom_to_file(FILE* output_file);
		static char* export_wisdom_to_string(void);
		static void export_wisdom(fftw_write_char_func write_char,
		                                  void* data);
		static int import_system_wisdom(void);
		static int import_wisdom_from_filename(const char* filename);
		static int import_wisdom_from_file(FILE* input_file);
		static int import_wisdom_from_string(const char* input_string);
		static int import_wisdom(fftw_read_char_func read_char, void* data);

		static void fprint_plan(const plan_type p, FILE* output_file);
		static void print_plan(const plan_type p);

		static void* malloc(size_t n);
		static real_type* alloc_real(size_t n);
		static complex_type* alloc_complex(size_t n);
		static void free(void* p);

		static void flops(const plan_type p,
		                          double* add, double* mul, double* fmas);
		static double estimate_cost(const plan_type p);
		static double cost(const plan_type p);
	};
#endif // DSP_FFTW_HAVE_LONG_DOUBLE

} }

#endif // !DSP_FFTW_DISABLED

#endif /* DSP_FFTW_TRAITS_H_INCLUDED */
