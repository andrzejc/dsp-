/*!
 * @file snd/io.cpp
 * @brief Implementation of libsndfile C++ wrapper.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
//#include "pch.h"
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/sndfile_error.h>

#if !DSP_SNDFILE_DISABLED

#include <dsp++/snd/reader.h>
#include <dsp++/snd/writer.h>
#include <dsp++/snd/format.h>
#include <sndfile.h>
#include <cassert>
#include <cstring>
#include <functional>
#include <algorithm>
#include <limits>

#include <boost/scoped_array.hpp>

#include "../utility.h"

using namespace dsp::snd;

typedef iobase::size_type sz_t;

io::~io()
{
}

#ifdef _WIN32
#if defined(_MSC_VER) || (__MSVCRT_VERSION__ >= 0x800)
#define off_t  __int64
#define ftello _ftelli64
#define fseeko _fseeki64
#else
typedef long off_t;
#define ftello ftell
#define fseeko fseek
#endif
#endif

stdio::stdio(FILE* file, bool own_file)
 : 	file_(file)
 , 	own_file_(own_file)
{
}

sz_t stdio::size()
{
	off_t pos = ftello(file_);
	if (pos < 0)
		return -1;

	if (0 != fseeko(file_, 0, SEEK_END))
		return -1;

	off_t res = ftello(file_);
	if (0 != fseeko(file_, pos, SEEK_SET))
		return -1;

	return res;
}

sz_t stdio::seek(sz_t offset, int whence)
{
	if (0 != fseeko(file_, offset, whence))
		return -1;
	return ftello(file_);
}

sz_t stdio::read(void* buf, size_t size)
{
	return fread(buf, size, 1, file_);
}

sz_t stdio::write(const void* buf, size_t size)
{
	return fwrite(buf, size, 1, file_);
}

sz_t stdio::position()
{
	return ftello(file_);
}

stdio::~stdio()
{
	if (own_file_)
		fclose(file_);
}

namespace {

struct file_type_entry {
	const char* label;
	int type;
};

const file_type_entry file_types[] =
{
	{file_type::label::aiff, 		SF_FORMAT_AIFF},
	{file_type::label::au, 			SF_FORMAT_AU},
	{file_type::label::core_audio, 	SF_FORMAT_CAF},
	{file_type::label::flac, 		SF_FORMAT_FLAC},
	{file_type::label::matlab4, 	SF_FORMAT_MAT4},
	{file_type::label::matlab5, 	SF_FORMAT_MAT5},
	{file_type::label::ogg,	 		SF_FORMAT_OGG},
	{file_type::label::raw, 		SF_FORMAT_RAW},
	{file_type::label::wav, 		SF_FORMAT_WAV},
	{file_type::label::wav64, 		SF_FORMAT_W64},
};

struct channel_map_entry {
	channel::type our;
	int their;
};

const channel_map_entry channel_map[] =
{
	{channel::type_left, SF_CHANNEL_MAP_LEFT},
	{channel::type_right, SF_CHANNEL_MAP_RIGHT},
	{channel::type_center, SF_CHANNEL_MAP_CENTER},
	{channel::type_lfe, SF_CHANNEL_MAP_LFE},
	{channel::type_left_surround, SF_CHANNEL_MAP_REAR_LEFT},
	{channel::type_right_surround, SF_CHANNEL_MAP_REAR_RIGHT},
	{channel::type_left_center, SF_CHANNEL_MAP_FRONT_LEFT_OF_CENTER},
	{channel::type_right_center, SF_CHANNEL_MAP_FRONT_RIGHT_OF_CENTER},
	{channel::type_surround, SF_CHANNEL_MAP_REAR_CENTER},
	{channel::type_side_left, SF_CHANNEL_MAP_SIDE_LEFT},
	{channel::type_side_right, SF_CHANNEL_MAP_SIDE_RIGHT},
	{channel::type_top, SF_CHANNEL_MAP_TOP_CENTER},
	{channel::type_top_front_left, SF_CHANNEL_MAP_TOP_FRONT_LEFT},
	{channel::type_top_front_center, SF_CHANNEL_MAP_TOP_FRONT_CENTER},
	{channel::type_top_front_right, SF_CHANNEL_MAP_TOP_FRONT_RIGHT},
	{channel::type_top_back_left, SF_CHANNEL_MAP_TOP_REAR_LEFT},
	{channel::type_top_back_center, SF_CHANNEL_MAP_TOP_REAR_CENTER},
	{channel::type_top_back_right, SF_CHANNEL_MAP_TOP_REAR_RIGHT},
};

static int map_format(const file_format& f)
{
	int res = 0;
	const std::string& ft = f.type();
	const file_type_entry* const  e = dsp::detail::match_element(file_types, &file_type_entry::label, ft);
	if (NULL != e)
		res |= e->type;
	sample::type st = f.sample_type();
	unsigned bs = f.sample_bits();
	switch (st) {
	case sample::type_pcm_float:
		if (bs <= 32)
			res |= SF_FORMAT_FLOAT;
		else
			res |= SF_FORMAT_DOUBLE;
		break;
	case sample::type_pcm_signed:
		switch (bs) {
		case 8: res |= SF_FORMAT_PCM_S8; break;
		case 16: res |= SF_FORMAT_PCM_16; break;
		case 24: res |= SF_FORMAT_PCM_24; break;
		case 32: res |= SF_FORMAT_PCM_32; break;
		}
		break;
	case sample::type_pcm_unsigned:
		if (9 == bs)
			res |= SF_FORMAT_PCM_U8;
		break;
	default:
		break;
	}
	if ((res & SF_FORMAT_OGG) && sample::type_unknown == st)
		res |= SF_FORMAT_VORBIS;
	return res;
}

static void map_format(int format, dsp::snd::file_format& f)
{
	const file_type_entry* const e = dsp::detail::match_element(file_types, &file_type_entry::type, format & SF_FORMAT_TYPEMASK);
	if (NULL != e)
		f.set_type(e->label);
	else if (format & SF_FORMAT_WAVEX)
		f.set_type(file_type::label::wav);

	switch (format & SF_FORMAT_SUBMASK) {
	case SF_FORMAT_PCM_S8: f.set_sample_format(sample::label::s8); break;
	case SF_FORMAT_PCM_U8: f.set_sample_format(sample::label::u8); break;
	case SF_FORMAT_PCM_16: f.set_sample_format(sample::label::s16); break;
	case SF_FORMAT_PCM_24: f.set_sample_format(sample::label::s24); break;
	case SF_FORMAT_PCM_32: f.set_sample_format(sample::label::s32); break;
	case SF_FORMAT_FLOAT: f.set_sample_format(sample::label::f32); break;
	case SF_FORMAT_DOUBLE: f.set_sample_format(sample::label::f64); break;
	}
}

}

struct dsp::snd::base_impl
{
	SNDFILE* sf_;
	SF_INFO info_;
	io* io_;
	bool read_;
	bool own_io_;

	base_impl(bool read)
	 : 	sf_(NULL)
	 , 	io_(NULL)
	 , 	read_(read)
	 , 	own_io_(false)
	{
	}

	void close_io()
	{
		if (own_io_)
			delete io_;

		own_io_ = false;
		io_ = NULL;
	}

	void close()
	{
		if (NULL != sf_)
		{
			sf_close(sf_);
			sf_ = NULL;
		}

		close_io();
	}

	~base_impl()
	{
		close();
	}

	void throw_error()
	{
		int code = sf_error(sf_);
		assert(SF_ERR_NO_ERROR != code);
		throw sndfile_error(code, sf_error_number(code));
	}

	void init_info(file_format* f, SF_INFO* info)
	{
		if (NULL != f)
		{
			memset(&info_, 0, sizeof(info_));
			info_.samplerate = f->sample_rate();
			info_.channels = f->channel_count();
			info_.format = map_format(*f);
		}
		else if (NULL != info)
			info_ = *info;
		else
			memset(&info_, 0, sizeof(info_));
	}

	void read_channel_map(file_format& f)
	{
		boost::scoped_array<int> arr(new int[info_.channels]);
		int res = sf_command(sf_, SFC_GET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * info_.channels);
		if (0 != res)
			return;

		channel::layout layout;
		for (int i = 0; i < info_.channels; ++i)
		{
			const channel_map_entry* const e = dsp::detail::match_element(channel_map, &channel_map_entry::their, arr[i]);
			if (NULL == e)
				return;

			layout.set(e->our, true);
		}
		f.set_channel_layout(layout);
	}

	void write_channel_map(const file_format& f)
	{
		boost::scoped_array<int> arr(new int[f.channel_count()]);
		unsigned cc = 0;
		for (int i = 0; i < channel::type_label_count; ++i)
		{
			if (!f.channel_layout().test(i))
				continue;

			const channel_map_entry* const e = dsp::detail::match_element(channel_map, &channel_map_entry::our, i);
			if (NULL == e)
				return;

			arr[cc++] = e->their;
		}
		if (cc != f.channel_count())
			return;
		sf_command(sf_, SFC_SET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * cc);
	}

	void fill_info(file_format* f, SF_INFO* info)
	{
		if (NULL != f)
		{
			f->set_sample_rate(info_.samplerate);
			f->set_channel_count(info_.channels);
			map_format(info_.format, *f);

			if (read_)
				read_channel_map(*f);
			else
				write_channel_map(*f);
		}
		if (NULL != info)
			*info = info_;
	}

	void open(const char* path, file_format* fmt, void* native_info)
	{
		close();
		init_info(fmt, static_cast<SF_INFO*>(native_info));
		if (NULL == (sf_ =  sf_open(path, read_ ? SFM_READ : SFM_WRITE, &info_)))
			throw_error();
		fill_info(fmt, static_cast<SF_INFO*>(native_info));
	}

	void open(int fd, bool own_fd, file_format* fmt, void* native_info)
	{
		close();
		init_info(fmt, static_cast<SF_INFO*>(native_info));
		if (NULL == (sf_ = sf_open_fd(fd, read_ ? SFM_READ : SFM_WRITE, &info_, own_fd ? 1 : 0)))
			throw_error();
		fill_info(fmt, static_cast<SF_INFO*>(native_info));
	}

	static sf_count_t io_size(void* p) {return static_cast<io*>(p)->size();}
	static sf_count_t io_seek(sf_count_t offset, int whence, void* p)
	{return static_cast<io*>(p)->seek(offset, whence);}
	static sf_count_t io_read(void* buf, sf_count_t count, void* p)
	{
		if (count < 0 || count > std::numeric_limits<unsigned>::max())
			return -1;
		return static_cast<io*>(p)->read(buf, static_cast<size_t>(count));
	}

	static sf_count_t io_write(const void* buf, sf_count_t count, void* p)
	{
		if (count < 0 || count > std::numeric_limits<size_t>::max())
			return -1;
		return static_cast<io*>(p)->write(buf, static_cast<size_t>(count));
	}

	static sf_count_t io_tell(void* p) {return static_cast<io*>(p)->position();}

	static const SF_VIRTUAL_IO sf_vio;

	void open(io* in, bool own_io, file_format* fmt, void* native_info)
	{
		if (NULL == in)
			throw std::invalid_argument("in == NULL");

		close();
		init_info(fmt, static_cast<SF_INFO*>(native_info));
		if (NULL == (sf_ = sf_open_virtual(const_cast<SF_VIRTUAL_IO*>(&sf_vio),
				read_ ? SFM_READ : SFM_WRITE, &info_, in)))
		{
			if (own_io)
				delete in;
			throw_error();
		}
		io_ = in;
		own_io_ = own_io;
		fill_info(fmt, static_cast<SF_INFO*>(native_info));
	}

};

const SF_VIRTUAL_IO base_impl::sf_vio = {
		&base_impl::io_size, &base_impl::io_seek, &base_impl::io_read,
		&base_impl::io_write, &base_impl::io_tell
};

iobase::iobase(bool read)
 : 	impl_(new base_impl(read))
{
}

iobase::~iobase()
{
	delete impl_;
}

void iobase::open(const char* path, dsp::snd::file_format* fmt, void* native_info)
{
	impl_->open(path, fmt, native_info);
}

void iobase::open(int fd, bool own_fd, dsp::snd::file_format* fmt, void* native_info)
{
	impl_->open(fd, own_fd, fmt, native_info);
}

void iobase::open(io* in, bool own_io, dsp::snd::file_format* fmt, void* native_info)
{
	impl_->open(in, own_io, fmt, native_info);
}

void iobase::open(FILE* f, bool own_file, dsp::snd::file_format* fmt, void* native_info)
{
	open(new stdio(f, own_file), true, fmt, native_info);
}

unsigned iobase::channel_count() const
{
	return impl_->info_.channels;
}

unsigned iobase::sample_rate() const
{
	return impl_->info_.samplerate;
}

bool iobase::is_seekable() const
{
	return (0 != impl_->info_.seekable);
}

sz_t iobase::frame_count() const
{
	return impl_->info_.frames;
}

SNDFILE* iobase::raw_handle()
{
	return impl_->sf_;
}

sz_t iobase::seek(sz_t frames, int whence)
{
	sz_t res;
	if ((res = sf_seek(impl_->sf_, frames, whence)) < 0)
		impl_->throw_error();
	return res;
}

void iobase::close()
{
	impl_->close();
}

void iobase::throw_last_error()
{
	impl_->throw_error();
}

int iobase::command(int cmd, void* data, int datasize)
{
	return sf_command(impl_->sf_, cmd, data, datasize);
}

reader::reader()
 :	iobase(true)
{
}

#define READ_ITEMS(items, type, name) \
sz_t reader::read_ ## items (type* buf, sz_t count) \
{ \
	sz_t res; \
	if ((res = sf_ ## name ## _ ## type (raw_handle(), buf, count)) < 0) \
		throw_last_error(); \
	return res; \
}

#define READ_FRAMES(type) READ_ITEMS(frames, type, readf)
#define READ_SAMPLES(type) READ_ITEMS(samples, type, read)

READ_FRAMES(float);
READ_FRAMES(short);
READ_FRAMES(int);
READ_FRAMES(double);

READ_SAMPLES(float);
READ_SAMPLES(short);
READ_SAMPLES(int);
READ_SAMPLES(double);

writer::writer()
 :	iobase(false)
{
}

#define WRITE_ITEMS(items, type, name) \
sz_t writer::write_ ## items (const type* buf, sz_t count) \
{ \
	sz_t res; \
	if ((res = sf_ ## name ## _ ## type (raw_handle(), buf, count)) < 0) \
		throw_last_error(); \
	return res; \
}

#define WRITE_FRAMES(type) WRITE_ITEMS(frames, type, writef)
#define WRITE_SAMPLES(type) WRITE_ITEMS(samples, type, write)

WRITE_FRAMES(float);
WRITE_FRAMES(short);
WRITE_FRAMES(int);
WRITE_FRAMES(double);

WRITE_SAMPLES(float);
WRITE_SAMPLES(short);
WRITE_SAMPLES(int);
WRITE_SAMPLES(double);

#endif // !DSP_SNDFILE_DISABLED
