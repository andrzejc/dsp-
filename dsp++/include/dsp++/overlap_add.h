/*!
 * @file dsp++/overlap_add.h
 * @brief Definition of overlap_add class template.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_OVERLAPP_ADD_H_INCLUDED
#define DSP_OVERLAPP_ADD_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/fft.h>
#include <dsp++/pow2.h>
#include <dsp++/algorithm.h>
#include <dsp++/noncopyable.h>

#include <algorithm>
#include <functional>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif // !DSP_BOOST_CONCEPT_CHECKS_DISABLED

namespace dsp {

/*!
 * @brief Implementation of Overlapp-Add convolution (FIR filtering via frequency-domain multiplication).
 * @tparam Real real-number type representing input & output signal samples.
 * @tparam DFT type of DFT algorithm implementation used for frequency-domain filtering.
 * @see http://en.wikipedia.org/wiki/Overlap-add_method
 */
template<class Real, template<class, class> class DFT = dsp::fft>
class overlap_add: private noncopyable
{
public:
	typedef Real value_type;
	typedef std::complex<value_type> complex_type;
	typedef DFT<value_type, complex_type> transform_type;
	typedef DFT<complex_type, value_type> inverse_transform_type;
	typedef typename transform_type::input_allocator real_allocator;
	typedef typename transform_type::output_allocator complex_allocator;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;
	typedef complex_type* complex_iterator;
	typedef const complex_type* const_complex_iterator;

	/*!
	 * @brief Construct Overlap-Add algorithm functor with the specified operation frame length
	 * and filter impulse response.
	 * @param frame_length number of samples in a single operation frame.
	 * @param ir_begin start iterator of impulse response sequence.
	 * @param ir_end end iterator of impulse response sequence.
	 * @param preserve_ir_length if true, the impulse response is not checked against possible trailing
	 * 0's, which may be ignored to decrease the DFT transform length. This param is useful if the
	 * impulse response will be changed through a call to set_impulse_response() during the processing.
	 * @tparam Iterator type of the iterator used for passing the impulse response sequence
	 * (which must conform to bidirectional iterator concept).
	 */
	template<class Iterator>
	overlap_add(size_t frame_length, Iterator ir_begin, Iterator ir_end, bool preserve_ir_length = false);
	/*!
	 * @brief Construct Overlap-Add algorithm functor with the specified operation frame length
	 * and filter impulse response.
	 * @param frame_length number of samples in a single operation frame.
	 * @param ir start of impulse response vector.
	 * @param ir_length length of impulse response vector.
	 * @param preserve_ir_length if true, the impulse response is not checked against possible trailing
	 * 0's, which may be ignored to decrease the DFT transform length. This param is useful if the
	 * impulse response will be changed through a call to set_impulse_response() during the processing.
	 * @tparam Sample type convertible to value_type, which represents impulse response vector elements.
	 */
	template<class Sample>
	overlap_add(size_t frame_length, const Sample* ir, size_t ir_length, bool preserve_ir_length = false);

	//! @brief Free up allocated memory.
	~overlap_add();

	//! @return length of input/output frame
	size_t frame_length() const {return L_;}
	//! @return length of the impulse response of implemented FIR filter.
	size_t impulse_response_length() const {return M_;}
	//! @return transform size (N_)
	size_t transform_size() const {return N_;}

	//! @return pointer (serving as an iterator) to the first sample of the input/output frame.
	iterator begin() {return rbuf_;}
	//! @return pointer (serving as an iterator) to the first sample of the input/output frame.
	const_iterator begin() const {return rbuf_;}
	//! @return pointer (serving as an iterator) to the one-past-last sample of the input/output frame.
	iterator end() {return rbuf_ + L_;}
	//! @return pointer (serving as an iterator) to the one-past-last sample of the input/output frame.
	const_iterator end() const {return rbuf_ + L_;}
	/*!
	 * @brief Perform filtration of the current input frame, represented as samples in the range [begin(), end()),
	 * and store the result in the same sequence.
	 */
	void operator()();
	/*!
	 * @brief Replace filter's impulse response with a new one (of the same or shorter length).
	 * Invocation of this function will cause new impulse response transform to be calculated,
	 * which, as a side-effect will cause samples of current frame to be overwritten.
	 * @param begin start of new impulse response samples sequence.
	 * @param end end of new impulse response samples sequence.
	 * @tparam Iterator type of the iterator used for passing the impulse response sequence
	 * (which must conform to bidirectional iterator concept).
	 */
	template<class Iterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<Iterator>)),(void))
#else
	void
#endif
	set_impulse_response(Iterator begin, Iterator end)
	{
		size_t n = dsp::copy_at_most_n(begin, end, rbuf_, M_);
		std::fill_n(rbuf_ + n, N_ - n, value_type());
		prepare_ir_dft(false);
	}

	/*!
	 * @brief Replace filter's impulse response with a new one (of the same or shorter length).
	 * Invocation of this function will cause new impulse response transform to be calculated,
	 * which, as a side-effect will cause samples of current frame to be overwritten.
	 * @param ir start of impulse response vector.
	 * @param ir_length length of impulse response vector.
	 * @tparam Sample type convertible to value_type, which represents impulse response vector elements.
	 */
	template<class Sample>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::Convertible<Sample, Real>)),(void))
#else
	void
#endif
	set_impulse_response(const Sample* ir, size_t ir_length)
	{
		ir_length = std::min(M_, ir_length);
		std::copy(ir, ir + ir_length, rbuf_);
		std::fill_n(rbuf_ + ir_length, N_ - ir_length, value_type());
		prepare_ir_dft(false);
	}

	//! @brief Read impulse response transform \f$H(z)\f$.
	//! @return start of impulse response transform vector.
	const_complex_iterator H_begin() const {return cbuf_ + N_;}
	//! @brief Read impulse response transform \f$H(z)\f$.
	//! @return end of impulse response transform vector.
	const_complex_iterator H_end() const {return cbuf_ + 2 * N_;}
	std::pair<const_complex_iterator, const_complex_iterator> H() const {return std::make_pair(H_begin(), H_end());}

	//! @brief Access/modify impulse response transform \f$H(z)\f$.
	//! @return start of impulse response transform vector.
	complex_iterator H_begin() {return cbuf_ + N_;}
	//! @brief Access/modify impulse response transform \f$H(z)\f$.
	//! @return end of impulse response transform vector.
	complex_iterator H_end() {return cbuf_ + 2 * N_;}
	std::pair<complex_iterator, complex_iterator> H() {return std::make_pair(H_begin(), H_end());}

private:
	void prepare_ir_dft(bool zero_tail);

	/*!
	 * @brief Find length of the non-zero portion of impulse response given it as an iterator range.
	 * @tparam Iterator iterator type. Bidirectional iterator is expected, this is verified through
	 * boost concept checks library.
	 * @param begin start of impulse response samples sequence.
	 * @param end end of impulse response samples sequence.
	 * @return length of non-zero portion of input sequence.
	 */
	template<class Iterator> static
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<Iterator>)),(size_t))
#else
	size_t
#endif
	nonzero_length(Iterator begin, Iterator end)
	{
		size_t length = 0;
		size_t zeros = 0;
		typedef typename std::iterator_traits<Iterator>::value_type value_type;
		const value_type zero = value_type();
		bool only_zeros = true;
		while (end != begin)
		{
			++length;
			--end;
			if (only_zeros)
			{
				if (zero == *end)
					++zeros;
				else
					only_zeros = false;
			}
		}
		return length - zeros;
	}



	real_allocator ralloc_;
	complex_allocator calloc_;
	const size_t L_; 	//!< Frame length.
	const size_t M_;	//!< Impulse response length.
	const size_t N_;	//!< DFT Transform length (nextpow2(L_ + M_)).
	value_type* rbuf_;		//!< Real-valued buffer (of length N_ + M_, first N_ samples serve as input buffer
							//!< for DFT and output buffer for IDFT, last M_ samples are used for storing overlapping
							//!< fragment of previous frame.
	complex_type* cbuf_; 	//!< Complex-valued buffer (of length 2 * N_), first N_ samples serve as
							//!< output buffer for DFT and input buffer for IDFT, last N_ samples
							//!< are used for storing pre-computed transform of impulse response.
	transform_type dft_;	//!< DFT functor
	inverse_transform_type idft_; //!< IDFT functor
};

template<class Real, template<class, class> class DFT> inline
overlap_add<Real, DFT>::~overlap_add()
{
	calloc_.deallocate(cbuf_, 2 * N_);
	ralloc_.deallocate(rbuf_, N_ + M_);
}

template<class Real, template<class, class> class DFT> inline
void overlap_add<Real, DFT>::prepare_ir_dft(bool zero_tail)
{
	std::fill_n(rbuf_ + M_, (zero_tail ? N_ : N_ - M_), value_type()); 	 // pad impulse response with
												// 0's up to length N and initialize
												// overlapping region to 0's too (if zero_tail is set)
	dft_();										// calculate the DFT of impulse response
	std::copy(cbuf_, cbuf_ + N_, cbuf_ + N_);	// copy calculated transform to the destination
}

template<class Real, template<class, class> class DFT>
template<class Iterator> inline
overlap_add<Real, DFT>::overlap_add(size_t frame_length, Iterator ir_begin, Iterator ir_end, bool preserve_ir_length)
 :	L_(frame_length)
 , 	M_(preserve_ir_length ? std::distance(ir_begin, ir_end) : nonzero_length(ir_begin, ir_end))	// find real length of the impulse response
 ,	N_(nextpow2(L_ + M_))				// calculate transform size
 , 	rbuf_(ralloc_.allocate(N_ + M_))	// don't even bother with calling construct() on these, they are just numbers
 , 	cbuf_(calloc_.allocate(2 * N_))
 ,	dft_(N_, rbuf_, cbuf_)
 , 	idft_(N_, cbuf_, rbuf_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::BidirectionalIterator<Iterator>));
#endif
	std::copy_n(ir_begin, M_, rbuf_); 		// copy impulse response to rbuf to calculate its DFT
	prepare_ir_dft(true);
}

template<class Real, template<class, class> class DFT>
template<class Sample> inline
overlap_add<Real, DFT>::overlap_add(size_t frame_length, const Sample* ir, size_t ir_length, bool preserve_ir_length)
 :	L_(frame_length)
 ,	M_(preserve_ir_length ? ir_length : nonzero_length(ir, ir + ir_length))
 ,	N_(nextpow2(L_ + M_))
 ,	rbuf_(ralloc_.allocate(N_ + M_))
 ,	cbuf_(calloc_.allocate(2 * N_))
 ,	dft_(N_, rbuf_, cbuf_)
 ,	idft_(N_, cbuf_, rbuf_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<Sample, Real>));
#endif
	std::copy_n(ir, M_, rbuf_); 		// copy impulse response to rbuf to calculate its DFT
	prepare_ir_dft(true);
}

template<class Real, template<class, class> class DFT> inline
void overlap_add<Real, DFT>::operator ()()
{
	std::fill_n(rbuf_ + L_, N_ - L_, value_type()); 	// fill the input vector with 0's starting from L up to N
	dft_(rbuf_, cbuf_);									// obtain DFT of the current (zero-padded) frame
	std::transform(cbuf_, cbuf_ + N_, cbuf_ + N_, cbuf_, std::multiplies<complex_type>()); // multiply the transforms
	idft_(cbuf_, rbuf_);								// perform IDFT
	std::transform(rbuf_, rbuf_ + N_, rbuf_, std::bind2nd(std::divides<value_type>(), static_cast<value_type>(N_)));
														// normalize the IDFT by a factor of 1/N
	std::transform(rbuf_, rbuf_ + M_, rbuf_ + N_, rbuf_, std::plus<value_type>());	// add the "tail" M samples
														// left from previous step
	std::copy(rbuf_ + L_, rbuf_ + L_ + M_, rbuf_ + N_);	// save the "tail" M samples for next step
}

}

#endif /* DSP_OVERLAPP_ADD_H_INCLUDED */
