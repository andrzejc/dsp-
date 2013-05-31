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
	typedef boost::upgrade_lock<mutex> upgrade_lock;
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

	struct band_channel
	{
		d::overlap_add<s_t> filter_;
		d::compressor<s_t> compressor_;

		//band_channel(size_t 
	};

	typedef boost::shared_ptr<band_channel> band_channel_ptr;

	struct band {
		std::vector<band_channel_ptr> channels_;
		r::multiband_compressor::band_params params_;
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

	void set_crossover_frequency_impl(unsigned index, float freq, det::upgrade_lock& rl, bool force);

	void set_band_params_direct(band& b, const r::multiband_compressor::band_params& p, bool force);
	void set_band_params_impl(unsigned index, const r::multiband_compressor::band_params& p, det::upgrade_lock& rl);
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

void r::multiband_compressor::fill_band_params(unsigned index, band_params& p) const {
	det::shared_lock l(impl_->mutex_);
	if (index >= impl_->bands_.size())
		throw std::out_of_range("dsp::rt::multiband_compressor::fill_band_params() index out of range");
	p = impl_->bands_[index].params_;
}

void det::mbc_impl::set_band_params_direct(det::mbc_impl::band& b, const r::multiband_compressor::band_params& p, bool force) {
	if (p.attack_ms != b.params_.attack_ms || force) {
		for (unsigned i = 0; i < channel_count_; ++i)
			b.channels_[i]->compressor_.set_attack(static_cast<size_t>(sample_rate_ * p.attack_ms / 1000.f + .5f));
		b.params_.attack_ms = p.attack_ms;
	}
	b.params_.bypass = p.bypass;
	if (p.gain_dB != b.params_.gain_dB || force) {
		for (unsigned i = 0; i < channel_count_; ++i)
			b.channels_[i]->compressor_.set_gain_dB(p.gain_dB);
		b.params_.gain_dB = p.gain_dB;
	}
	b.params_.mute = p.mute;
	if (p.ratio != b.params_.ratio || force) {
		for (unsigned i = 0; i < channel_count_; ++i)
			b.channels_[i]->compressor_.set_ratio(p.ratio);
		b.params_.ratio = p.ratio;
	}
	if (p.release_ms != b.params_.release_ms || force) {
		for (unsigned i = 0; i < channel_count_; ++i)
			b.channels_[i]->compressor_.set_release(static_cast<size_t>(sample_rate_ * p.release_ms / 1000.f + .5f));
		b.params_.release_ms = p.release_ms;
	}
	if (p.threshold_dB != b.params_.threshold_dB || force) {
		for (unsigned i = 0; i < channel_count_; ++i)
			b.channels_[i]->compressor_.set_threshold_dB(p.threshold_dB);
		b.params_.threshold_dB = p.threshold_dB;
	}
}

void det::mbc_impl::set_band_params_impl(unsigned index, const r::multiband_compressor::band_params& p, det::upgrade_lock& rl) {
	if (index >= bands_.size())
		throw std::out_of_range("dsp::rt::multiband_compressor::set_band_params() index out of range");

	band& b = bands_[index];
	if (p == b.params_)
		return;

	det::upgrade_to_unique_lock wl(rl);
	set_band_params_direct(b, p, false);
}

void r::multiband_compressor::set_band_params(unsigned index, const r::multiband_compressor::band_params& p) {
	det::upgrade_lock rl(impl_->mutex_);
	impl_->set_band_params_impl(index, p, rl);
}

namespace {
static void design_fir(float lo, float hi, unsigned fs, double* ir, size_t ir_max_length) {
	float nl = lo / fs, nh = hi / fs;
	if (1.1 * nl >= 0.9 * nh)
		throw std::range_error("dsp::rt::multiband_compressor::set_crossover_frequency() resulting frequency band too narrow");

	double f[6], a[6] = {0, 0, 1, 1, 0, 0}, w[3] = {1, 1, 1};
	f[0] = 0; f[1] = 0.95 * nl; f[2] = 1.05 * nl; f[3] = 0.95 * nh; f[4] = 1.05 * nh; f[5] = 0.5;
	bool res;
	if (lo <= 20.f) {
		f[2] = 0;
		res = d::firpm(ir_max_length - 1, ir, 2, &f[2], &a[2], &w[1]);
	}
	else if (hi >= fs - 20.f) {
		f[3] = 0.5;
		res = d::firpm(ir_max_length - 1, ir, 2, f, a, w);
	}
	else
		res = d::firpm(ir_max_length - 1, ir, 3, f, a, w);
}
}

void det::mbc_impl::set_crossover_frequency_impl(unsigned index, float freq, det::upgrade_lock& rl, bool force) {
	size_t sz = xover_.size();
	if (index >= sz)
		throw std::out_of_range("dsp::rt::multiband_compressor::set_crossover_frequency() index out of range");
	if (freq == xover_[index] && !force)
		return;

	float prev = 0.f;
	float next = sample_rate_ / 2.f;
	if (index > 0)
		prev = xover_[index - 1];
	if (sz > 0 && index < sz - 1)
		next = xover_[index + 1];
	if (freq <= prev || freq >= next)
		throw std::domain_error("dsp::rt::multiband_compressor::set_crossover_frequency() frequency no monotonically increasing or outside Nyquist range");

	const size_t len = ir_length_max;
	d::trivial_array<double> arr(len * 2);
	design_fir(prev, freq, sample_rate_, &arr[0], len);
	design_fir(freq, next, sample_rate_, &arr[len], len);

	det::upgrade_to_unique_lock wl(rl);
	xover_[index] = freq;
	for (unsigned i = 0; i < channel_count_; ++i) {
		bands_[index].channels_[i]->filter_.set_impulse_response(&arr[0], len);
		bands_[index + 1].channels_[i]->filter_.set_impulse_response(&arr[len], len);
	}
}

void r::multiband_compressor::set_crossover_frequency(unsigned index, float freq) {
	det::upgrade_lock rl(impl_->mutex_);
	impl_->set_crossover_frequency_impl(index, freq, rl, false);
}

void r::multiband_compressor::fill_params(r::multiband_compressor::params& p) const {
	det::shared_lock l(impl_->mutex_);
	p.sample_rate = impl_->sample_rate_;
	p.channel_count = impl_->channel_count_;
	p.block_size = impl_->block_size_;
	p.crossover_frequencies = impl_->xover_;
	p.bands.resize(impl_->bands_.size());
	for (size_t i = 0; i < p.bands.size(); ++i)
		p.bands[i] = impl_->bands_[i].params_;
	p.limiter_threshold_dB = impl_->limiter_.threshold_dB();
	p.limiter_bypass = impl_->limiter_bypass_;
}

void r::multiband_compressor::set_params(const r::multiband_compressor::params& p) {
	if (p.bands.size() > 1 && p.crossover_frequencies.size() != p.bands.size() - 1)
		throw std::logic_error("dsp::rt::multiband_compressor::set_params() crossover frequency/bands count mismatch");

	det::upgrade_lock rl(impl_->mutex_);
	bool changed = false;
	bool bands_changed = false;
	bool xover_changed = false;
	if (p.crossover_frequencies != impl_->xover_)
		xover_changed = changed = true;
	else if (p.bands.size() != impl_->bands_.size()) 
		bands_changed = changed = true;
	else {
		for (size_t i = 0; i < p.bands.size(); ++i)
			if (p.bands[i] != impl_->bands_[i].params_) {
				bands_changed = changed = true;
				break;
			}
	}
	if (!changed && p.limiter_threshold_dB != impl_->limiter_.threshold_dB())
		changed = true;
	if (!changed && p.limiter_bypass != impl_->limiter_bypass_)
		changed = true;

	if (!changed)
		return;

	det::upgrade_to_unique_lock wl(rl);
	int xover_force = -1;
	if (p.bands.size() < impl_->bands_.size()) {
		// number of bands decreased, last band will need to be recalculated to become HP
		xover_force = int(p.bands.size()) - 1;
		impl_->bands_.resize(p.bands.size());
	}
	else if (p.bands.size() > impl_->bands_.size()) {
		// number of bands increased, all bands starting from current last one need to be recalculated
		xover_force = impl_->xover_.size();
		size_t start = impl_->bands_.size();
		// add missing band entries
		impl_->bands_.resize(p.bands.size());

		//for (size_t i = start; i < p.bands.size(); ++i) 
		//	impl_->add_band_direct(p.bands[i]);
		// TODO implement add_band_direct
	}


}