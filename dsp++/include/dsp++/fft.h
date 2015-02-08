/*!
 * @file dsp++/fft.h
 * @brief Simple, low-overhead template-based FFT implementation.
 * @author (of dsp++ wrapper) Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 * @author (of original code) Vladimir Mirnyi (see copyright notice below)
 * @copyright Copyright &copy; 2006 by Volodymyr Myrnyy (Vladimir Mirnyi)
 * Permission to use, copy, modify, distribute and sell this software for any
 * purpose is hereby granted without fee, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and this
 * permission notice appear in supporting documentation.
 */
#ifndef DSP_FFT_H_INCLUDED
#define DSP_FFT_H_INCLUDED

#include <dsp++/dft.h>

#include <complex>
#include <memory>
#include <algorithm>
#include <functional>

#include <dsp++/fft/detail.h>

namespace dsp {

using std::complex;
	
namespace dft {

template<class Input, class Output>
class fft {
	typedef void domain_type;
	typedef Input input_type;
	typedef Output output_type;
	typedef std::allocator<input_type> input_allocator;
	typedef std::allocator<output_type> output_allocator;
};

template<class Real>
class fft<Real, complex<Real> >;
template<class Real>
class fft<complex<Real>, Real >;


/*!
 * @brief Functor for computing Discrete Fourier Transform (DFT).
 * This class implements DFT of complex sequences. The complex number domain
 * is determined by the template parameter Real. The DFT is calculated
 * through FFT algorithm (Cooley-Tukey) with Danielson-Lanczos recursion.
 * The "canonical" DFT equation is:
 * @f[
 * X_k = \sum_{n=0}^{N-1} x_n \cdot e^{-i2\pi\frac{k}{N}n}
 * @f]
 * so the sign of the exponent is -1, which is reflected in the value of
 * @link dft::sign::forward @endlink constant (for forward DFT). The equation
 * for inverse DFT is:
 * @f[
 * x_n = \frac{1}{N}\sum_{k=0}^{N-1} X_k \cdot e^{i2\pi\frac{k}{N}n}
 * @f]
 * where the sign of exponent is 1 (and consequently the value of constant
 * @link dft::sign::backward @endlink). This class implements both forward and inverse-DFT
 * through the sign parameter of the constructor, however it doesn't perform
 * the normalization of IDFT results (the @f$\frac{1}{N}@f$ factor in the formula).
 * To obtain correctly normalized values, the output sequence should be
 * divided by N.
 * @see W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P.Flannery. <em>Numerical Recipes in C++</em>. Cambridge university press, 2002.
 * @tparam Real floating-point type using during the calculations.
 */
template<class Real>
class fft<complex<Real>, complex<Real> > {
public:
	typedef Real domain_type;
	typedef complex<Real> input_type;
	typedef complex<Real> output_type;
	typedef std::allocator<input_type> input_allocator;
	typedef std::allocator<output_type> output_allocator;

	/*!
	 * @brief Initialize FFT functor to calculate DFT via FFT of power-of-2 sized sequence.
	 * @param N transform length (and the size of input/output vectors).
	 * @param input the input samples (may be @c NULL if only operator()(input_type*, output_type*) const
	 * is going to be used).
	 * @param output the vector which will contain samples of calculated transform.
	 * May be @c NULL if only operator()(input_type*, output_type*) const is going to be used, or if the
	 * input is not @c NULL, and the transform should be performed in-place.
	 * @param sign transform direction (sign of the exponent in canonical DFT equation).
	 * @throw std::domain_error is thrown if N is not an integer power of 2.
	 * @throw std::out_of_range is thrown if @f$N < 2@f$ or @f$N > 2^{28}@f$.
	 */
	fft(size_t N, input_type* input = NULL, output_type* output = NULL, enum_class_ref(sign) sign = sign::forward)
	 :	size_(N), input_(input), output_(output), sign_(sign)
	 ,	impl_(&detail::fft_impl<Real>::get(N))
	{}

	/*!
	 * @brief Copy constructor.
	 * Since only references to const, preallocated objects are used this is cheap.
	 * @param other the fft instance to copy.
	 */
	fft(const fft& other)
	 : size_(other.size_), input_(other.input_), output_(other.output_), sign_(other.sign_)
	 ,	impl_(other.impl_)
	{}

	/*!
	 * @brief Invoke the FFT algorithm for the specified input & output vectors.
	 * @param input input samples vector.
	 * @param output output samples vector (may be @c NULL, if transform should be performed in-place).
	 */
	void operator()(input_type* input, output_type* output) const
	{
		if (NULL == output)
			output = input;
		else
			std::copy(input, input + size_, output);
		impl_->fft(output, sign_);
	}

	//! @brief Invoke the FFT algorithm for the input & output vectors set up in the constructor.
	void operator()() const	{operator()(input_, output_);}

	//! @return the transform size (N in the equation above).
	size_t size() const {return size_;}

private:
	size_t size_;
	input_type* input_;
	output_type* output_;
	enum_class_ref(sign) sign_;
	const detail::fft_impl<Real>* impl_;
	friend class fft<Real, complex<Real> >;
	friend class fft<complex<Real>, Real >;
};

/*!
 * @brief Forward FFT of real data. The resulting complex output sequence will have Hermitian symmetry.
 */
template<class Real>
class fft<Real, complex<Real> > {
	typedef fft<complex<Real>, complex<Real> > fft_type;
public:
	typedef Real domain_type;
	typedef Real input_type;
	typedef complex<Real> output_type;
	typedef std::allocator<input_type> input_allocator;
	typedef std::allocator<output_type> output_allocator;

	/*!
	 * @brief Initialize FFT functor to perform forward, real-to-complex data DFT of specified transform length.
	 * @param N transform length (and the size of input/output vectors). Must be the integer power of 2.
	 * @param input the input samples (may be @c NULL if only operator()(input_type*, output_type*) const
	 * is going to be used).
	 * @param output the vector which will contain samples of calculated transform.
	 * May be @c NULL if only operator()(input_type*, output_type*) const is going to be used, or if the
	 * input is not @c NULL, and the transform should be performed in-place.
	 * @throw std::domain_error is thrown if N is not an integer power of 2.
	 * @throw std::out_of_range is thrown if @f$N < 2@f$ or @f$N > 2^{28}@f$.
	 */
	fft(size_t N, input_type* input = NULL, output_type* output = NULL, enum_class_ref(sign) = sign::forward)
	 :	fft_(N, reinterpret_cast<typename fft_type::input_type*>(input), output, sign::forward)
	{}

	/*!
	 * @brief Copy constructor.
	 * Since only references to const, preallocated objects are used this is cheap.
	 * @param other the fft instance to copy.
	 */
	fft(const fft& other): fft_(other.fft_) {}

	/*!
	 * @brief Invoke FFT algorithm on the vectors provided in the constructor (which must not be @c NULL in this case).
	 * @note Contrary to the version of this  operator specialized for complex-to-complex transform,
	 * output param must not be set to @c NULL, as the input and output types (and the byte sizes of vectors) differ.
	 * @param input input samples vector.
	 * @param output output samples vector.
	 */
	void operator()(input_type* input, output_type* output) const
	{
		std::copy(input, input + fft_.size_, output);
		fft_.operator()(output, NULL);
	}

	//! @brief Invoke the FFT algorithm for the input & output vectors set up in the constructor.
	void operator()() const	{operator()(const_cast<input_type*>(reinterpret_cast<const input_type*>(fft_.input_)), fft_.output_);}

	//! @return the transform size (N in the equation above).
	size_t size() const {return fft_.size();}
private:
	fft_type fft_;
};

/*!
 * @brief Inverse FFT of complex, Hermitian symmetry data.
 */
template<class Real>
class fft<complex<Real>, Real> {
	typedef fft<complex<Real>, complex<Real> > fft_type;
public:
	typedef Real domain_type;
	typedef complex<Real> input_type;
	typedef Real output_type;
	typedef std::allocator<input_type> input_allocator;
	typedef std::allocator<output_type> output_allocator;

	/*!
	 * @brief Initialize FFT functor to perform backward, complex-to-real data DFT of specified transform length.
	 * @param N transform length (and the size of input/output vectors). Must be the integer power of 2.
	 * @param input the input samples (may be @c NULL if only operator()(input_type*, output_type*) const
	 * is going to be used).
	 * @param output the vector which will contain samples of calculated transform.
	 * May be @c NULL if only operator()(input_type*, output_type*) const is going to be used, or if the
	 * input is not @c NULL, and the transform should be performed in-place.
	 * @throw std::domain_error is thrown if N is not an integer power of 2.
	 * @throw std::out_of_range is thrown if @f$N < 2@f$ or @f$N > 2^{28}@f$.
	 */
	fft(size_t N, input_type* input = NULL, output_type* output = NULL, enum_class_ref(sign) = sign::backward)
	 :	fft_(N, input, reinterpret_cast<complex<Real>*>(output), sign::backward)
	{}

	/*!
	 * @brief Copy constructor.
	 * Since only references to const, preallocated objects are used this is cheap.
	 * @param other the fft instance to copy.
	 */
	fft(const fft& other): fft_(other.fft_) {}

	/*!
	 * @brief Invoke FFT algorithm on the vectors provided in the constructor (which must not be @c NULL in this case).
	 * @note Contrary to the version of this  operator specialized for complex-to-complex transform,
	 * output param must not be set to @c NULL, as the input and output types (and the byte sizes of vectors) differ.
	 * It is enough to provide N/2 + 1 complex samples in the input vector, as the remaining ones will be ignored
	 * (they are assumed to have Hermitian symmetry), nevertheless the vector still must be big enough to store all N
	 * complex samples, as it is used for as temporary space. Consequently, be prepared that its contents will be
	 * overwritten.
	 * @param input input samples vector.
	 * @param output output samples vector.
	 */
	void operator()(input_type* input, output_type* output) const
	{
		for (size_t i = fft_.size_ / 2 + 1; i < fft_.size_; ++i)
			input[i] = std::conj(input[fft_.size_ - i]);
		fft_.operator()(reinterpret_cast<input_type*>(input), NULL);
		for (size_t i = 0; i < fft_.size_; ++i)
			*output++ = (input++)->real();
	}

	//! @brief Invoke the FFT algorithm for the input & output vectors set up in the constructor.
	void operator()() const
	{operator()(fft_.input_, const_cast<output_type*>(reinterpret_cast<const output_type*>(fft_.output_)));}

	//! @return the transform size (N in the equation above).
	size_t size() const {return fft_.size();}
private:
	fft_type fft_;
};

}} // namespace dsp::dft

#endif // !DSP_FFT_H_INCLUDED
