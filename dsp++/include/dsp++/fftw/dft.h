/*!
 * @file dsp++/fftw/dft.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_FFTW_DFT_H_INCLUDED
#define DSP_FFTW_DFT_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_FFTW_DISABLED

#include <dsp++/export.h>
#include <dsp++/fftw/traits.h>
#include <dsp++/fftw/allocator.h>
#include <boost/shared_ptr.hpp>
#include <dsp++/dft.h>

namespace dsp { namespace dft { namespace fftw {

	/*!
	 * @brief Flags controlling FFTW3 planner behavior.
	 * This is a direct mapping of libfftw3 @c FFTW_* flags with validation
	 * performed through @c BOOST_STATIC_ASSERT() in traits.cpp.
	 */
	namespace flag { enum spec
	{
		measure = (0U),
		destroy_input = (1U << 0),
		unaligned = (1U << 1),
		conserve_memory = (1U << 2),
		exhaustive = (1U << 3), /* NO_EXHAUSTIVE is default */
		preserve_input = (1U << 4), /* cancels FFTW_DESTROY_INPUT */
		patient = (1U << 5), /* IMPATIENT is default */
		estimate = (1U << 6),
		wisdom_only = (1U << 21),
	};}

	template<class Domain>
	class plan {
	public:
		typedef Domain domain_type;
		typedef traits<Domain> traits_type;
		typedef plan<Domain> this_type;

		//!@brief Calls @c traits::destroy_plan() to release the resources.
		//!@see fftw_destroy_plan()
		~plan() {}
		/*!
		 * @brief Calls @c traits::execute() to perform DFT calculation according
		 * to prepared plan.
		 * @see fftw_execute()
		 */
		void operator()() const {traits_type::execute(get_pointer(plan_));}

		//!@see fftw_cost()
		double cost() const {return traits_type::cost(get_pointer(plan_));}
		//!@see fftw_estimate_cost()
		double estimate_cost() const {return traits_type::cost(get_pointer(plan_));}

		/*!
		 * @brief Calls @c traits::forget_wisdom() to dispose of libfftw accumulated
		 * wisdom.
		 * @see fftw_forget_wisdom()
		 */
		static void forget_wisdom() {traits_type::forget_wisdom();}
		/*!
		 * @brief Calls @c traits::cleanup() to perform libfftw cleanup for this type's
		 * variant of library.
		 * @see fftw_cleanup()
		 */
		static void cleanup() {traits_type::cleanup();}
		/*!
		 * @brief Calls @c traits::set_timelimit() to configure maximum time (in seconds)
		 * the planner is allowed to run (-1 for no limit).
		 * @see fftw_set_timelimit()
		 */
		static void set_planner_timelimit(double t) {traits_type::set_timelimit(t);}

		static const size_t size_not_1d = 0;
		//! @return size of 1-dimensional transform (length of input/output vectors), size_not_1d if not 1-dimensional transform.
		size_t size() const {return size_1d_;}

	protected:
		/*!
		 * @brief Initialize this plan wrapper with the given fftw plan pointer
		 * (obtained using one of the traits functions).
		 * @param plan the plan pointer this instance will own.
		 * @param size1d size of unidimensional transform (passed from subclass if unidimensional transform is being created).
		 * @throw plan_unavailable when plan is NULL (i.e. planner function was unable
		 * to prepare the plan for specified input params).
		 */
		explicit plan(typename traits_type::plan_type plan, size_t size1d = size_not_1d)
		 : 	plan_(plan, &traits_type::destroy_plan), size_1d_(size1d)
		{detail::verify_plan_available(get_pointer(plan_));}
		//! Type of smart pointer used for copying the plan instance.
		typedef boost::shared_ptr<typename traits_type::plan_value_type> ref_type;
		//! Plan pointer (wrapped up in boost::shared_ptr to enable copying) for use by subclasses.
		ref_type plan_;
		/*!
		 * @brief Copy constructor.
		 * The DFT plan is made copy-constructible to make it possible to use it
		 * for functional programming with standard algorithms.
		 */
		explicit plan(const plan& other): plan_(other.plan_), size_1d_(other.size_1d_) {}
		/*!
		 * Initialize this plan object with the specified fftw plan instance.
		 * This is intended for more sophisticated construction of plan, when
		 * the single function call may be not enough.
		 * @param plan
		 * @param size1d size of unidimensional transform (passed from subclass if unidimensional transform is being created).
		 * @throw plan_unavailable when plan is NULL (i.e. planner function was unable
		 * to prepare the plan for specified input params).
		 */
		explicit plan(const ref_type& plan, size_t size1d = size_not_1d)
		 : 	plan_(plan), size_1d_(size1d)
		{detail::verify_plan_available(get_pointer(plan_));}

		static size_t find_size_1d(size_t rank, const unsigned* n) {return ((1 == rank) ? *n: size_not_1d);}
	private:
		size_t size_1d_;
	};

	/*!
	 * Generic (undefined) template class serving as a prototype for custom
	 * specializations below.
	 * @tparam Domain Real type specifying the variant of libfftw API to use.
	 * @tparam Input Type of elements of input sequence: either Domain or std::complex<Domain>.
	 * @tparam Output Type of elements of output sequence: either Domain or std::complex<Domain>.
	 */
	template<class Input, class Output>
	class dft: public plan<void> {
	public:
		typedef plan<void> base_type;
		//! Real number type.
		typedef Input domain_type;
		//! Type of input sequence samples.
		typedef Input input_type;
		//! Type of output sequence samples.
		typedef Output output_type;
		//! FFTW traits type specialized for the domain type.
		typedef traits<Input> traits_type;
		//! Optimal allocator of input vector.
		typedef dsp::dft::fftw::allocator<input_type> input_allocator;
		//! Optimal allocator of output vector.
		typedef dsp::dft::fftw::allocator<output_type> output_allocator;


		dft(size_t rank, const unsigned* n, input_type* in, output_type* out, unsigned flags = 0);
		dft(size_t n, input_type* in, output_type* out, sign::spec, unsigned flags = 0); //!< For compatibility with dsp::dft::fft
		dft(size_t n, input_type* in, output_type* out, unsigned flags = 0);
		dft(size_t n0, size_t n1, input_type* in, output_type* out, unsigned flags = 0);
		dft(size_t n0, size_t n1, size_t n2, input_type* in, output_type* out, unsigned flags = 0);
		dft(size_t rank, const unsigned* n, size_t howmany, input_type* in, const int* inembed, int istride, int idist,
				output_type* out, const int* onembed, int ostride, int odist, unsigned flags = 0);

		using base_type::operator ();
		void operator()(input_type* in, output_type* out) const;

		/*!
		 * @brief Copy constructor.
		 * @param other DFT plan of the same type to copy.
		 */
		explicit dft(const dft& other)
		 : 	base_type(static_cast<const base_type&>(other)) {}

	};

	/*!
	 * @brief Specialization of class @c dft for real-to-real data transforms.
	 */
	template<class Real>
	class dft<Real, Real>: public plan<Real> {
	public:
		typedef plan<Real> base_type;
		typedef Real domain_type;
		typedef Real input_type;
		typedef Real output_type;
		typedef traits<Real> traits_type;
		typedef dsp::dft::fftw::allocator<input_type> input_allocator;
		typedef dsp::dft::fftw::allocator<output_type> output_allocator;
		typedef dft<input_type, output_type> this_type;

		//!@see fftw_plan_r2r()
		dft(size_t rank, const unsigned* n, input_type* in, output_type* out, const r2r::kind* kind, unsigned flags = 0)
		 :	base_type(traits_type::plan_r2r(rank, n, in, out, kind, flags), base_type::find_size_1d(rank, n)) {}
		//!@see fftw_plan_r2r_1d()
		dft(size_t n, input_type* in, output_type* out, r2r::kind kind, unsigned flags = 0)
		 :	base_type(traits_type::plan_r2r_1d(static_cast<int>(n), in, out, kind, flags), n) {}
		//!@see fftw_plan_r2r_2d()
		dft(size_t n0, size_t n1, input_type* in, output_type* out, r2r::kind kind0, r2r::kind kind1, unsigned flags = 0)
		 :	base_type(traits_type::plan_r2r_2d(n0, n1, in, out, kind0, kind1, flags)) {}
		//!@see fftw_plan_r2r_3d()
		dft(size_t n0, size_t n1, size_t n2, input_type* in, output_type* out,
				r2r::kind kind0, r2r::kind kind1, r2r::kind kind2, unsigned flags = 0)
		 :	base_type(traits_type::plan_r2r_3d(n0, n1, n2, in, out, kind0, kind1, kind2, flags)) {}
		//!@see fftw_plan_many_r2r()
		dft(size_t rank, const unsigned* n, size_t howmany, input_type* in, const int* inembed, int istride, int idist,
				output_type* out, const int* onembed, int ostride, int odist, const r2r::kind* kind, unsigned flags = 0)
		 :	base_type(traits_type::plan_many_r2r(rank, n, howmany, in, inembed, istride, idist,
				 out, onembed, ostride, odist, kind, flags), base_type::find_size_1d(rank, n)) {}

		using base_type::operator ();

		//!@see fftw_execute_r2r()
		void operator()(input_type* in, output_type* out) const
		{traits_type::execute_r2r(get_pointer(base_type::plan_), in, out);}

		/*!
		 * @brief Copy constructor.
		 * @param other dft_r2r plan to copy.
		 */
		explicit dft(const dft& other)
		 : 	base_type(static_cast<const base_type&>(other)) {}
	};

	/*!
	 * @brief Specialization of class @c dft for real-to-complex data transforms.
	 */
	template<class Real>
	class dft<Real, std::complex<Real> >: public plan<Real> {
	public:
		typedef plan<Real> base_type;
		typedef Real input_type;
		typedef std::complex<Real> output_type;
		typedef traits<Real> traits_type;
		typedef dsp::dft::fftw::allocator<input_type> input_allocator;
		typedef dsp::dft::fftw::allocator<output_type> output_allocator;
		typedef dft<input_type, output_type> this_type;

		//!@see fftw_plan_dft_r2c()
		dft(size_t rank, const unsigned* n, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_r2c(rank, n, in, out, flags), base_type::find_size_1d(rank, n)) {}
		//!@see fftw_plan_dft_r2c_1d()
		dft(size_t n, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_r2c_1d(n, in, out, flags), n) {}
		//!Compatibility constructor for use as an alternative to dsp::dft::fft
		//!@see fftw_plan_dft_r2c_1d()
		dft(size_t n, input_type* in, output_type* out, sign::spec, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_r2c_1d(n, in, out, flags), n) {}
		//!@see fftw_plan_dft_r2c_2d()
		dft(size_t n0, size_t n1, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_r2c_2d(n0, n1, in, out, flags)) {}
		//!@see fftw_plan_dft_r2c_3d()
		dft(size_t n0, size_t n1, size_t n2, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_r2c_3d(n0, n1, n2, in, out, flags)) {}
		//!@see fftw_plan_many_dft_r2c()
		dft(size_t rank, const unsigned* n, size_t howmany, input_type* in, const int* inembed, int istride, int idist,
                output_type* out, const int* onembed, int ostride, int odist, unsigned flags = 0)
		 :	base_type(traits_type::plan_many_dft_r2c(rank, n, howmany, in, inembed, istride, idist,
				 out, onembed, ostride, odist, flags), base_type::find_size_1d(rank, n)) {}

		using base_type::operator ();

		//!@see fftw_execute_dft_r2c()
		void operator()(input_type* in, output_type* out) const
		{traits_type::execute_dft_r2c(get_pointer(base_type::plan_), in, out);}

		/*!
		 * @brief Copy constructor.
		 * @param other dft_r2c plan to copy.
		 */
		explicit dft(const dft& other)
		 : 	base_type(static_cast<const base_type&>(other)) {}
	};

	/*!
	 * @brief Specialization of class @c dft for complex-to-real data transforms.
	 */
	template<class Real>
	class dft<std::complex<Real>, Real>: public plan<Real> {
	public:
		typedef plan<Real> base_type;
		typedef std::complex<Real> input_type;
		typedef Real output_type;
		typedef traits<Real> traits_type;
		typedef dsp::dft::fftw::allocator<input_type> input_allocator;
		typedef dsp::dft::fftw::allocator<output_type> output_allocator;
		typedef dft<input_type, output_type> this_type;

		//!@see fftw_plan_dft_c2r()
		dft(size_t rank, const unsigned* n, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_c2r(rank, n, in, out, flags), base_type::find_size_1d(rank, n)) {}
		//!@see fftw_plan_dft_c2r_1d()
		dft(size_t n, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_c2r_1d(n, in, out, flags), n) {}
		//!Compatibility constructor for use as an alternative to dsp::dft::fft
		//!@see fftw_plan_dft_c2r_1d()
		dft(size_t n, input_type* in, output_type* out, sign::spec, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_c2r_1d(n, in, out, flags), n) {}
		//!@see fftw_plan_dft_c2r_2d()
		dft(size_t n0, size_t n1, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_c2r_2d(n0, n1, in, out, flags)) {}
		//!@see fftw_plan_dft_c2r_3d()
		dft(size_t n0, size_t n1, size_t n2, input_type* in, output_type* out, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_c2r_3d(n0, n1, n2, in, out, flags)) {}
		//!@see fftw_plan_many_dft_c2r()
		dft(size_t rank, const unsigned* n, size_t howmany, input_type* in, const int* inembed, int istride, int idist,
				output_type* out, const int* onembed, int ostride, int odist, unsigned flags = 0)
		 :	base_type(traits_type::plan_many_dft_c2r(rank, n, howmany, in, inembed, istride, idist,
				 out, onembed, ostride, odist, flags), base_type::find_size_1d(rank, n)) {}

		using base_type::operator ();

		//!@see fftw_execute_dft_c2r()
		void operator()(input_type* in, output_type* out) const
		{traits_type::execute_dft_c2r(get_pointer(base_type::plan_), in, out);}

		/*!
		 * @brief Copy constructor.
		 * @param other dft_c2r plan to copy.
		 */
		explicit dft(const dft& other)
		 : 	base_type(static_cast<const base_type&>(other)) {}
	};

	/*!
	 * @brief Specialization of class @c dft for complex-to-complex data transforms.
	 */
	template<class Real>
	class dft<std::complex<Real>, std::complex<Real> >: public plan<Real> {
	public:
		typedef plan<Real> base_type;
		typedef std::complex<Real> input_type;
		typedef std::complex<Real> output_type;
		typedef traits<Real> traits_type;
		typedef dsp::dft::fftw::allocator<input_type> input_allocator;
		typedef dsp::dft::fftw::allocator<output_type> output_allocator;
		typedef dft<input_type, output_type> this_type;

		//!@see fftw_plan_dft()
		dft(size_t rank, const unsigned* n, input_type* in, output_type* out, sign::spec sign, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft(rank, n, in, out, sign, flags), base_type::find_size_1d(rank, n)) {}
		//!@see fftw_plan_dft_1d()
		dft(size_t n, input_type* in, output_type* out, sign::spec sign = sign::forward, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_1d(n, in, out, sign, flags), n) {}
		//!@see fftw_plan_dft_2d()
		dft(size_t n0, size_t n1, input_type* in, output_type* out, sign::spec sign, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_2d(n0, n1, in, out, sign, flags)) {}
		//!@see fftw_plan_dft_3d()
		dft(size_t n0, size_t n1, size_t n2, input_type* in, output_type* out, sign::spec sign, unsigned flags = 0)
		 :	base_type(traits_type::plan_dft_3d(n0, n1, n2, in, out, sign, flags)) {}
		//!@see fftw_plan_many_dft()
		dft(size_t rank, const unsigned* n, size_t howmany, input_type* in, const int* inembed, int istride, int idist,
				output_type* out, const int* onembed, int ostride, int odist, sign::spec sign, unsigned flags = 0)
		 :	base_type(traits_type::plan_many_dft(rank, n, howmany, in, inembed, istride, idist,
				 out, onembed, ostride, odist, sign, flags), base_type::find_size_1d(rank, n)) {}

		using base_type::operator ();

		//!@see fftw_execute_dft()
		void operator()(input_type* in, output_type* out) const
		{traits_type::execute_dft(get_pointer(base_type::plan_), in, out);}

		/*!
		 * @brief Copy constructor.
		 * @param other dft plan to copy.
		 */
		explicit dft(const dft& other)
		 : 	base_type(static_cast<const base_type&>(other)) {}
	};

} } }

#endif // !DSP_FFTW_DISABLED

#endif /* DSP_FFTW_DFT_H_INCLUDED */
