#include <dsp++/rt/multiband_compressor.h>
#include <dsp++/snd/format.h>
#include <dsp++/filter_design.h>
#include <dsp++/overlap_add.h>
#include <dsp++/dynamics.h>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include <stdexcept>

namespace d = dsp;
namespace s = d::snd;
namespace r = d::rt;
namespace det = r::detail;

namespace dsp { namespace rt { namespace detail {
	typedef boost::upgrade_mutex mutex;
	typedef boost::shared_lock<mutex> shared_lock;
	typedef boost::unique_lock<mutex> unique_lock;
	typedef boost::upgrade_to_unique_lock<mutex> upgrade_to_unique_lock;

	typedef r::multiband_compressor::sample_type s_t;
} } }

struct det::mbc_impl 
{
	static const size_t ir_length_max = 256;

	mutex mutex_;
	const unsigned sample_rate_;
	const unsigned channel_count_;
	const unsigned block_size_;

	d::trivial_array<s_t> ir_;

	struct band {
		d::overlap_add<s_t> filter_;
		d::compressor<s_t> compressor_;
	};


	std::vector<float> xover_;
	std::vector<band> bands_;
	d::limiter<s_t> limiter_;
	bool limiter_bypass_;

	mbc_impl(unsigned sample_rate, unsigned channel_count, unsigned block_size)
	 :	sample_rate_(sample_rate)
	 ,	channel_count_(channel_count)
	 ,	block_size_(block_size)
	 ,	ir_(ir_length_max)
	 ,	limiter_bypass_(false)
	{
	}

};

r::multiband_compressor::multiband_compressor(unsigned sample_rate, unsigned channel_count, unsigned block_size)
 : impl_(new det::mbc_impl(sample_rate, channel_count, block_size))
{
}

r::multiband_compressor::multiband_compressor(const r::multiband_compressor::params& p)
 : impl_(new det::mbc_impl(p.sample_rate, p.channel_count, p.block_size))
{
}

r::multiband_compressor::~multiband_compressor()
{
	impl_.reset();
}

void r::multiband_compressor::set_limiter_threshold_dB(float t) {
	det::unique_lock l(impl_->mutex_);
	impl_->limiter_.set_threshold_dB(t);
}

float r::multiband_compressor::limiter_threshold_dB() const {
	det::shared_lock l(impl_->mutex_);
	return impl_->limiter_.threshold_dB();
}

void r::multiband_compressor::set_limiter_bypass(bool b) {
	det::unique_lock l(impl_->mutex_);
	impl_->limiter_bypass_ = b;
}

bool r::multiband_compressor::is_limiter_bypass() const {
	det::shared_lock l(impl_->mutex_);
	return impl_->limiter_bypass_;
}

unsigned r::multiband_compressor::band_count() const {
	det::shared_lock l(impl_->mutex_);
	return impl_->bands_.size();
}

unsigned r::multiband_compressor::block_size() const {
	return impl_->block_size_;
}

unsigned r::multiband_compressor::channel_count() const {
	return impl_->channel_count_;
}

unsigned r::multiband_compressor::sample_rate() const {
	return impl_->sample_rate_;
}

void r::multiband_compressor::fill_format(s::format& f) const {
	f.set_channel_mask(0);
	f.set_channel_count(impl_->channel_count_);
	f.set_sample_format(s::sample::label::f32);
	f.set_sample_rate(impl_->sample_rate_);
}

float r::multiband_compressor::crossover_frequency(unsigned index) const {
	det::shared_lock l(impl_->mutex_);
	if (index >= impl_->xover_.size())
		throw std::out_of_range("dsp::rt::multiband_compressor::crossover_frequency() index out of range");
	return impl_->xover_[index];
}

void r::multiband_compressor::set_crossover_frequency(unsigned index, float freq) {
	det::shared_lock rl(impl_->mutex_);
	size_t sz = impl_->xover_.size();
	if (index >= sz)
		throw std::out_of_range("dsp::rt::multiband_compressor::set_crossover_frequency() index out of range");
	if (freq == impl_->xover_[index])
		return;

	float prev = 0.f;
	float next = impl_->sample_rate_ / 2.f;
	if (index > 0)
		prev = impl_->xover_[index - 1];
	if (sz > 0 && index < sz - 1)
		next = impl_->xover_[index + 1];
	if (freq <= prev || freq >= next)
		throw std::domain_error("dsp::rt::multiband_compressor::set_crossover_frequency() frequency no monotonically increasing or outside Nyquist range");

	//double f[6], a[6], w[3];
	//d::trivial_array<double> arr(det::mbc_impl::ir_length_max);
	//d::firpm(det::mbc_impl::ir_length_max - 1, &arr[0], 3, 
	// XXX finish filter design and 
}
