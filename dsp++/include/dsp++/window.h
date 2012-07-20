/*!
 * @file dsp++/window.h
 * @brief Window function generators.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_WINDOW_H_INCLUDED
#define DSP_WINDOW_H_INCLUDED

#include <dsp++/export.h>

#include <functional>
#include <cmath>

#include <boost/math/special_functions/sinc.hpp>
#include <boost/math/special_functions/bessel.hpp>
#include <boost/iterator.hpp>

namespace dsp { namespace wnd {

//! Constants governing generation of window functions.
enum window_type
{
	symmetric,	//!< Symmetric window.
	periodic  	//!< Periodic window, calculated by using window length of N + 1 and omitting the last sample.
};

/*!
 * @brief Base class for window generator functors.
 */
template<class Result>
struct window_function: public std::unary_function<size_t, Result>
{
	/*!
	 * @brief Initializes length and type fields of this functor.
	 * @param length the window length.
	 * @param type the window type.
	 */
	window_function(size_t length, window_type type = symmetric)
	 : 	length_(length)
	 , 	type_(type)
	 , 	l_(symmetric == type ? length : length + 1)
	{}
	//! @return length (N) of the generated window.
	size_t length() const {return length_;}
	//! @return type of generated window.
	window_type type() const {return type_;}

private:
	const size_t length_;
	const window_type type_;

protected:
	//! "Effective" window length used during calculations.
	const size_t l_;
};

/*!
 * @brief Implementation of iterator concept for window generators.
 * @tparam Window type of window this iterator applies to.
 */
template<class Window>
class window_iterator
 :	public boost::iterator<std::random_access_iterator_tag, typename Window::result_type, ptrdiff_t, void, void>
{
public:
	//! Just a typedef to save some keystrokes.
	typedef typename Window::result_type value_type;
	/*!
	 * @brief Construct iterator referencing n-th sample of specified window.
	 * @param window the window this iterator applies to.
	 * @param n sample number.
	 */
	window_iterator(const Window& window, ptrdiff_t n = 0): window_(&window), n_(n) {}
	/*!
	 * @brief Copy constructor.
	 * @param other iterator to copy.
	 */
	window_iterator(const window_iterator& other): window_(other.window_), n_(other.n_) {}
	/*!
	 * @brief Assignment operator.
	 * @param other iterator to copy.
	 * @return *this.
	 */
	window_iterator& operator=(const window_iterator& other)
	{window_ = other.window_; n_ = other.n_; return *this;}
	/*!
	 * @brief Get value of sample referenced by this iterator.
	 * @return n-th sample value.
	 */
	value_type operator*() const
	{return (n_ < 0 || n_ >= static_cast<ptrdiff_t>(window_->length()) ? value_type() : (*window_)(n_));}

	window_iterator& operator+=(ptrdiff_t n) {n_ += n; return *this;}
	window_iterator& operator-=(ptrdiff_t n) {n_ -= n; return *this;}
	window_iterator& operator++() {++n_; return *this;}
	window_iterator operator++(int) {return window_iterator(*window_, n_++); }
	window_iterator& operator--() {--n_; return *this;}
	window_iterator operator--(int) {return window_iterator(*window_, n_--);}
	window_iterator operator+(ptrdiff_t n) const {return window_iterator(*window_, n_ + n);}
	window_iterator operator-(ptrdiff_t n) const {return window_iterator(*window_, n_ - n);}
	ptrdiff_t operator-(const window_iterator& other) const {return n_ - other.n_;}
	bool operator==(const window_iterator& other) const {return (window_ == other.window_) && (n_ == other.n_);}
	bool operator!=(const window_iterator& other) const {return !operator==(other);}

private:
	const Window* window_;
	ptrdiff_t n_;
};

/*!
 * @def DSP_WINDOW_ITERATOR_SUPPORT
 * @brief Add iterator support to window generator.
 * @param type window class the iterator is being provided for.
 */
#define DSP_WINDOW_ITERATOR_SUPPORT(type) \
	typedef dsp::wnd::window_iterator<type> const_iterator; \
	const_iterator begin() const {return const_iterator(*this, 0);} \
	const_iterator end() const {return const_iterator(*this, window_function<Result>::length());}

/*!
 * Apply a given windowing function to a sequence of samples specified by
 * the iterator range [first, last).
 * Typical usage:
 * @code
 * apply(seq.begin(), seq.end(), dsp::wnd::blackman<float>(256, symmetric, 0.18f));
 * @endcode
 * @param begin first sample of the sequence.
 * @param end one-past-last sample of the sequence.
 * @param window tapering function generator (functor).
 */
template<template <typename Result> class Window, class Iterator>
void apply(Iterator begin, Iterator end,
		const Window<typename std::iterator_traits<Iterator>::value_type>& window)
{
	for (size_t i = 0; begin != end; ++begin, ++i)
		*begin *= window(i);
}

/*!
 * Apply a given windowing function to a sequence of samples specified by
 * the iterator range [first, last). The windowing functor class is passed
 * as a template parameter and it is initialized with the length of the
 * sequence calculated through std::distance(begin, end).
 * Typical usage:
 * @code
 * apply<hamming>(seq.begin(), seq.end(), periodic);
 * @endcode
 * @param begin first sample of the sequence.
 * @param end one-past-last sample of the sequence.
 * @param type type of instantiated windowing function.
 */
template<template <typename Result> class Window, class Iterator>
void apply(Iterator begin, Iterator end, window_type type = symmetric)
{
	Window<typename std::iterator_traits<Iterator>::value_type> window(std::distance(begin, end), type);
	apply(begin, end, window);
}

/*!
 * @brief Base interface for family of polymorphic window generators.
 */
template<class Result>
class window {
public:
	/*!
	 * @brief Virtual destructor to ensure polymorphic behavior.
	 */
	virtual ~window() {}
	/*!
	 * @brief Generate a vector of window samples.
	 * @param start vector start.
	 * @param num number of samples to generate.
	 */
	virtual void generate(Result* start, size_t num) const = 0;
	/*!
	 * @brief Apply tapering to a vector of samples.
	 * @param start vector start.
	 * @param num number of samples to window.
	 */
	virtual void apply(Result* start, size_t num) const = 0;
};

/*!
 * @brief Polymorphic adapter for window_function-compatible functors
 * to conform to dsp::wnd::window interface.
 */
template<class Result, template <typename> class Window>
class window_adapter: public window<Result>
{
public:
	//! Just a typedef to save some keystrokes.
	typedef Result result_type;
	//! Just a typedef to save some keystrokes.
	typedef Window<Result> window_type;
	/*!
	 * @brief Construct window_adapter based on provided functor.
	 * @param window functor whose parameters will be copied to this object.
	 */
	explicit window_adapter(const window_type& window): window_(window) {}
	/*!
	 * Fill provided array with up to window_.length() samples, set the remainder
	 * to 0.
	 * @param start vector start.
	 * @param num vector length.
	 */
	void generate(Result* start, size_t num) const
	{
		size_t len = std::min(num, window_.length());
		std::copy(window_.begin(), window_.begin() + len, start);
		std::fill_n(start + len, num - len, Result());
	}
	//! @copydoc window::apply()
	void apply(Result* start, size_t num) const
	{
		size_t len = std::min(num, window_.length());
		dsp::wnd::apply(start, start + len, window_);
		std::fill_n(start + len, num - len, Result());
	}

private:
	Window<Result> window_;
};

//!@brief Unspecified window function, unusable and unimplemented.
template<class Result>
struct unspecified: public window_function<Result>
{
	//! @copydoc window_function::window_function()
	unspecified(size_t length, window_type type = symmetric);
	/*!
	 * @brief Calculate n-th sample of window function.
	 * @return window function sample for given point.
	 */
	Result operator()(size_t n) const;
	/*!
	 * @brief Window length.
	 * @return you guess.
	 */
	size_t length() const;

	/*!
	 * @typedef const_iterator
	 * @brief Iterator type for accessing this window as a sequence.
	 */
	/*!
	 * @fn begin()
	 * @brief Start of iterator sequence.
	 * @return iterator referencing 0-th sample of this window.
	 */
	/*!
	 * @fn end()
	 * @brief End of iterator sequence.
	 * @return iterator referencing one-past-last sample of this window.
	 */
	DSP_WINDOW_ITERATOR_SUPPORT(unspecified);
};

/*!
 * @brief Rectangular (Dirichlet) window generator.
 * Rectangular window is given by the formula:
 * \f[
 * \omega\left(n\right) = 1
 * \f]
 * @see http://www.mathworks.com/help/toolbox/signal/ref/rectwin.html
 */
template<class Result>
struct rectwin: public window_function<Result>
{
	/*!
	 * @copydoc window_function::window_function()
	 * @note the parameters are allowed just for the consistency with other
	 * window function generators, as rectangular window doesn't use them.
	 */
	rectwin(size_t length = 0, window_type type = symmetric): window_function<Result>(length, type) {}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const {return Result(1);}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(rectwin);
};
/*!
 * @brief Create rectangular window adapter of specified length.
 * @param length window length
 * @return new instance of window_adapter specialized for dsp::wnd::rectwin.
 */
template<class Result>
window<Result>* create_rectwin(size_t length)
{return new window_adapter<Result, rectwin>(rectwin<Result>(length));}

/*!
 * @brief Hamming window generator.
 * Hamming window is given by the formula:
 * \f[
 * \omega\left(n\right) = 0.54 - 0.46\cdot \cos\left({2\pi{n}}\over{N-1}\right)
 * \f]
 * @see http://www.mathworks.com/help/toolbox/signal/ref/hamming.html
 */
template<class Result>
struct hamming: public window_function<Result>
{
	//! @copydoc window_function::window_function()
	hamming(size_t length, window_type type = symmetric): window_function<Result>(length, type) {}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		return 0.54 - 0.46 * cos(Result(2 * M_PI) * n / (window_function<Result>::l_ - 1));
	}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(hamming);
};
/*!
 * @brief Create Hamming window adapter of specified length.
 * @param length window length
 * @param type window type
 * @return new instance of window_adapter specialized for dsp::wnd::hamming.
 */
template<class Result>
window<Result>* create_hamming(size_t length, window_type type = symmetric)
{return new window_adapter<Result, hamming>(hamming<Result>(length, type));}

/*!
 * @brief Hann window generator.
 * Hann window is given by the formula:
 * \f[
 * \omega\left(n\right) = 0.5 \cdot \left(1 - \cos\left({2\pi{n}}\over{N-1}\right)\right)
 * \f]
 * @see http://www.mathworks.com/help/toolbox/signal/ref/hann.html
 */
template<class Result>
struct hann: public window_function<Result>
{
	//! @copydoc dsp::wnd::window_function::window_function()
	hann(size_t length, window_type type = symmetric): window_function<Result>(length, type) {}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		return 0.5 * (1 - cos(Result(2 * M_PI) * n / (window_function<Result>::l_ - 1)));
	}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(hann);
};
/*!
 * @brief Create Hann window adapter of specified length.
 * @param length window length
 * @param type window type
 * @return new instance of window_adapter specialized for dsp::wnd::hann.
 */
template<class Result>
window<Result>* create_hann(size_t length, window_type type = symmetric)
{return new window_adapter<Result, hann>(hann<Result>(length, type));}

/*!
 * @brief Blackman window generator.
 * Blackman window is given by the formula:
 * \f[
 * \omega\left(n\right) = a_{0} - a_{1}\cdot\cos\left(\frac{2\pi{n}}{N - 1}\right) +
 * 			a_{2}\cdot\cos\left(\frac{4\pi{n}}{N-1}\right)
 * \f]
 * where
 * \f[
 * a_{0}={{1 - \alpha}\over{2}} \text{; } a_{1}={{1}\over{2}} \text{; } a_{2}={{\alpha}\over{2}}
 * \f]
 * The \f$\alpha\f$ typically defaults to 0.16.
 * @see http://www.mathworks.com/help/toolbox/signal/ref/blackman.html
 */
template<class Result>
struct blackman: public window_function<Result>
{
	//! Default value of alpha parameter.
	static const Result alpha_default = 0.16;
	/*!
	 * @copydoc window_function::window_function()
	 * @param alpha \f$\alpha\f$ parameter in the equation above.
	 */
	blackman(size_t length, window_type type = symmetric, Result alpha = alpha_default)
	 : 	window_function<Result>(length, type)
	 ,	alpha_(alpha)
	 ,	a0_((1 - alpha) / 2)
	 ,	a1_(0.5)
	 ,	a2_(alpha / 2)
	{}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		return a0_ - a1_ * cos(Result(2 * M_PI) * n / (window_function<Result>::l_ - 1)) +
				a2_ * cos(Result(4 * M_PI) * n / (window_function<Result>::l_ - 1));
	}
	//! @return value of \f$\alpha\f$ parameter.
	Result alpha() const {return alpha_;}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(blackman);
private:
	const Result alpha_;
	const Result a0_;
	const Result a1_;
	const Result a2_;
};
/*!
 * @brief Create Blackman window adapter of specified length.
 * @param length window length
 * @param type window type
 * @param alpha value of alpha parameter
 * @return new instance of window_adapter specialized for dsp::wnd::blackman.
 */
template<class Result>
window<Result>* create_blackman(size_t length, window_type type = symmetric,
		Result alpha = blackman<Result>::alpha_default)
{return new window_adapter<Result, blackman>(blackman<Result>(length, type));}

/*!
 * @brief Cosine window generator.
 * Cosine (sine) window is given by the formula:
 * \f[
 * \omega\left(n\right) = \cos\left(\frac{\pi{n}}{N-1} - \frac{\pi}{2}\right)
 * \f]
 */
template<class Result>
struct cosine: public window_function<Result>
{
	//! @copydoc window_function::window_function()
	cosine(size_t length, window_type type = symmetric): window_function<Result>(length, type) {}
	//!@copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		return cos(Result(M_PI) * n / (window_function<Result>::l_ - 1) - Result(M_PI_2));
	}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(cosine);
};
/*!
 * @brief Create cosine window adapter of specified length.
 * @param length window length
 * @param type window type
 * @return new instance of window_adapter specialized for dsp::wnd::cosine.
 */
template<class Result>
window<Result>* create_cosine(size_t length, window_type type = symmetric)
{return new window_adapter<Result, cosine>(cosine<Result>(length, type));}

/*!
 * @brief Bartlett window generator.
 * Bartlett window is given by the formula:
 * \f[
 * \omega\left(n\right) = {{2}\over{N - 1}} \cdot \left({{N-1}\over{2}} - \left|n - {{N - 1}\over{2}}\right|\right)
 * \f]
 * Bartlett window is sometimes also named triangular window, however it has zero-valued
 * end points.
 * @see http://www.mathworks.com/help/toolbox/signal/ref/bartlett.html
 */
template<class Result>
struct bartlett: public window_function<Result>
{
	//! @copydoc window_function::window_function()
	bartlett(size_t length, window_type type = symmetric): window_function<Result>(length, type) {}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		const size_t N = window_function<Result>::l_;
		return Result(2) / (N - 1) * (Result(N - 1)/2 - abs(n - (Result(N - 1)/2)));
	}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(bartlett);
};
/*!
 * @brief Create Bartlett window adapter of specified length.
 * @param length window length
 * @param type window type
 * @return new instance of window_adapter specialized for dsp::wnd::bartlett.
 */
template<class Result>
window<Result>* create_bartlett(size_t length, window_type type = symmetric)
{return new window_adapter<Result, bartlett>(bartlett<Result>(length, type));}

/*!
 * @brief Triangular window generator.
 * Triangular window is given by the formula:
 * \f[
 * \omega\left(n\right) = {{2}\over{N+1}}\cdot\left({{N+1}\over{2}}-\left|n - {{N-1}\over{2}}\right|\right)
 * \f]
 * Triangular window has non-zero end-points.
 * @see http://www.mathworks.com/help/toolbox/signal/ref/triang.html
 */
template<class Result>
struct triang: public window_function<Result>
{
	//! @copydoc window_function::window_function()
	triang(size_t length, window_type type = symmetric): window_function<Result>(length, type) {}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		const size_t N = window_function<Result>::l_;
		return Result(2) / (N + 1) * (Result(N + 1)/2 - abs(n - (Result(N - 1)/2)));
	}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(triang);
};
/*!
 * @brief Create triangular window adapter of specified length.
 * @param length window length
 * @param type window type
 * @return new instance of window_adapter specialized for dsp::wnd::triang.
 */
template<class Result>
window<Result>* create_triang(size_t length, window_type type = symmetric)
{return new window_adapter<Result, triang>(triang<Result>(length, type));}

/*!
 * @brief Gaussian window generator.
 * Gaussian window is given by the formula:
 * \f[
 * \omega\left(n\right) = e^{-{{1}\over{2}}{\left({n - \left(N-1\right)/2}
 * \over{\sigma N/2}\right)}^2} \quad\text{ for }\quad \sigma\le0.5
 * \f]
 */
template<class Result>
struct gausswin: public window_function<Result>
{
	//! @brief Default value of @f$\sigma@f$ parameter.
	static const Result sigma_default = 0.4;
	//! @copydoc window_function::window_function()
	//! @param sigma \f$\sigma\f$ parameter in the equation above.
	gausswin(size_t length, window_type type = symmetric, Result sigma = sigma_default)
	 : 	window_function<Result>(length, type)
	 ,	sigma_(sigma){}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		const size_t N = window_function<Result>::l_;
		return exp(-Result(1)/2 * pow((Result(n) - Result(N - 1)/2)/(sigma_ * N / 2), 2));
	}
	//! @return value of @f$\sigma@f$ parameter.
	Result sigma() const {return sigma_;}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(gausswin);
private:
	const Result sigma_;
};
/*!
 * @brief Create Gaussian window adapter of specified length.
 * @param length window length
 * @param type window type
 * @param sigma value of @f$\sigma@f$ parameter
 * @return new instance of window_adapter specialized for dsp::wnd::gausswin.
 */
template<class Result>
window<Result>* create_gausswin(size_t length, window_type type = symmetric,
		Result sigma = gausswin<Result>::sigma_default)
{return new window_adapter<Result, gausswin>(gausswin<Result>(length, type, sigma));}

/*!
 * @brief Kaiser window generator.
 * Kaiser window is given by the formula:
 * \f[
 * \omega\left(n\right) = \frac{I_{0}\left(\pi\alpha\sqrt{1 - {\left({{2n}\over{N-1}}-1\right)}^2}\right)}
 *		{I_{0}\left(\pi\alpha\right)}
 * \f]
 * where @f$I_0(x)@f$ is the modified Bessel function of order zero and @f$\alpha@f$ is the window parameter.
 */
template<class Result>
struct kaiser: public window_function<Result>
{
	//! Default value of @f$\alpha@f$ parameter.
	static const Result alpha_default = 3;
	//! @copydoc window_function::window_function()
	//! @param alpha @f$\alpha@f$ parameter in the equation above.
	kaiser(size_t length, window_type type = symmetric, Result alpha = alpha_default)
	 : 	window_function<Result>(length, type)
	 ,	alpha_(alpha){}
	//! @copydoc unspecified::operator()(size_t) const
	Result operator()(size_t n) const
	{
		const size_t N = window_function<Result>::l_;
		return boost::math::cyl_bessel_i(0, Result(M_PI) * alpha_ * std::sqrt(1 - std::pow(Result(2*n)/(N-1) - 1, 2))) /
				boost::math::cyl_bessel_i(0, Result(M_PI) * alpha_);
	}
	//! @return value of @f$\alpha@f$ parameter.
	Result alpha() const {return alpha_;}

	/*! @typedef const_iterator
	 *  @copydoc unspecified::const_iterator */
	/*! @fn begin()
	 *  @copydoc unspecified::begin() */
	/*! @fn end()
	 *  @copydoc unspecified::end() */
	DSP_WINDOW_ITERATOR_SUPPORT(kaiser);
private:
	const Result alpha_;
};
/*!
 * @brief Create Kaiser window adapter of specified length.
 * @param length window length
 * @param type window type
 * @param alpha value of @f$\alpha@f$ parameter
 * @return new instance of window_adapter specialized for dsp::wnd::kaiser.
 */
template<class Result>
window<Result>* create_kaiser(size_t length, window_type type = symmetric,
		Result alpha = kaiser<Result>::alpha_default)
{return new window_adapter<Result, kaiser>(kaiser<Result>(length, type, alpha));}

}}

#endif /* DSP_WINDOW_H_INCLUDED */
