// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/lame/error.h>

#ifndef DSPXX_LAME_DISABLED

#include <dsp++/snd/mpeg/property.h>
#include <dsp++/snd/lame/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/sample.h>
#include <dsp++/snd/buffer.h>
#include <dsp++/snd/property.h>

#include "../utility.h"

#include <lame/lame.h>

#include <absl/strings/numbers.h>
#include <absl/hash/hash.h>
#include <boost/format.hpp>

#include <cassert>
#include <cstring>
#include <functional>
#include <algorithm>
#include <limits>
#include <memory>
#include <cstdio>
#include <cerrno>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

namespace dsp { namespace snd { namespace lame {

namespace {
constexpr size_t WRITE_SAMPLE_COUNT = 512;

template<typename Sample> struct lame_encode_func;
template<>
struct lame_encode_func<float> {
    int operator()(lame_t gpf, const float* pcm, const int ns, unsigned char* buf, const int buf_size, unsigned channel_count) const {
        if (channel_count == 2) {
            return lame_encode_buffer_interleaved_ieee_float(gpf, pcm, ns, buf, buf_size);
        } else {
            return lame_encode_buffer_ieee_float(gpf, pcm, pcm, ns, buf, buf_size);
        }
    }
};
template<>
struct lame_encode_func<double> {
    int operator()(lame_t gpf, const double* pcm, const int ns, unsigned char* buf, const int buf_size, unsigned channel_count) const {
        if (channel_count == 2) {
            return lame_encode_buffer_interleaved_ieee_double(gpf, pcm, ns, buf, buf_size);
        } else {
            return lame_encode_buffer_ieee_double(gpf, pcm, pcm, ns, buf, buf_size);
        }
    }
};
template<>
struct lame_encode_func<short> {
    int operator()(lame_t gpf, const short* pcm, const int ns, unsigned char* buf, const int buf_size, unsigned channel_count) const {
        if (channel_count == 2) {
            return lame_encode_buffer_interleaved(gpf, const_cast<short*>(pcm), ns, buf, buf_size);
        } else {
            return lame_encode_buffer(gpf, pcm, pcm, ns, buf, buf_size);
        }
    }
};
#ifndef DSPXX_LAME_NO_INT_API
template<>
struct lame_encode_func<int> {
    int operator()(lame_t gpf, const int* pcm, const int ns, unsigned char* buf, const int buf_size, unsigned channel_count) const {
        if (channel_count == 2) {
            return lame_encode_buffer_interleaved_int(gpf, pcm, ns, buf, buf_size);
        } else {
            return lame_encode_buffer_int(gpf, pcm, pcm, ns, buf, buf_size);
        }
    }
};
#endif

const std::unordered_map<string_view, uint32_t, absl::Hash<string_view>> PROPERTY_TAGS = {
    { property::album, 'TALB' },
    { property::album_artist, 'TPE2' },
    { property::album_artist_sort_order, 'TSO2' },
    { property::album_sort_order, 'TSOA' },
    { property::artist, 'TPE1' },
    { property::artist_sort_order, 'TSOP' },
    { property::bpm, 'TBPM' },
    { property::comment, 'COMM' },
    { property::composer, 'TCOM' },
    { property::composer_sort_order, 'TSOC' },
    { property::conductor, 'TPE3' },
    { property::copyright, 'TCOP' },
    { property::date, 'TYER' },
    // { property::disk_count, 'TPOS' },
    // { property::disk_number, 'TPOS' },
    { property::genre, 'TCON' },
    { property::key, 'TKEY' },
    { property::software, 'TSSE' },
    { property::title, 'TIT2' },
    { property::title_sort_order, 'TSOT' },
    // { property::track_count, 'TRCK' },
    // { property::track_number, 'TRCK' },
};

}

struct lame::writer::impl {
    std::unique_ptr<byte_stream> file;
    using lame_ptr = std::unique_ptr<lame_global_flags, decltype(&lame_close)>;
    lame_ptr handle{nullptr, &lame_close};
    std::vector<uint8_t> buffer;
    file_format format;
    size_t frame_count = 0;
    bool bitstream_init = false;
    const int lame_quality = 2;
    std::unordered_map<string, string> tags;
    size_t stream_start_position;

    size_t read_id3v2_header_length() {
        file->seek(stream_start_position, SEEK_SET);
        char id3v2Header[10];
        if (sizeof(id3v2Header) != file->read(id3v2Header, sizeof(id3v2Header))) {
            return 0;
        }
        if (0 == std::strncmp(id3v2Header, "ID3", 3)) {
            return (((id3v2Header[6] & 0x7f) << 21)
                  | ((id3v2Header[7] & 0x7f) << 14)
                  | ((id3v2Header[8] & 0x7f) << 7)
                  |  (id3v2Header[9] & 0x7f))
                + sizeof(id3v2Header);
        }
        else {
            return 0;
        }
    }

    void final_flush() {
        if (!bitstream_init) {
            return;
        }
        buffer.resize(std::max<size_t>(buffer.size(), 7200));
        int err = lame_encode_flush(handle.get(), &buffer[0], buffer.size());
        bitstream_init = false;
        if (err < 0) {
            throw error{err, boost::str(boost::format("lame_encode_flush failed with error %1%") % err)};
        }
        if (err > 0) {
            file->write(&buffer[0], err);
        }
        file->flush();
        auto lametag_size = lame_get_lametag_frame(handle.get(), nullptr, 0);
        if (lametag_size > 0) {
            buffer.resize(std::max<size_t>(buffer.size(), lametag_size));
            lametag_size = lame_get_lametag_frame(handle.get(), &buffer[0], buffer.size());
            assert(lametag_size > 0);
            assert(lametag_size <= buffer.size());
            byte_stream::position_saver save{*file};
            file->seek(stream_start_position + read_id3v2_header_length(), SEEK_SET);
            file->write(&buffer[0], lametag_size);
            file->flush();
        }
    }

    void close() {
        try {
            final_flush();
            file = {};
            format = {};
            frame_count = 0;
            tags = {};
        } catch (std::exception&) {
            file = {};
            format = {};
            frame_count = 0;
            tags = {};
            throw;
        }
    }

    ~impl() {
        try {
            close();
        } catch (std::exception&) {
        }
    }

    void new_handle(const file_format& fmt) {
        // Initialize 2 encoders the same way, one is a dry run, 2nd one will be initialized on first write
        // after dryrun passes
        handle.reset(lame_init());
        bitstream_init = false;
        int err;
        // Disable any resampling done by libmp3lame by setting out_samplerate directly
        if (LAME_NOERROR != (err = lame_set_out_samplerate(handle.get(), fmt.sample_rate()))) {
            throw error{err, "lame_set_out_samplerate"};
        }
        if (LAME_NOERROR != (err = lame_set_in_samplerate(handle.get(), fmt.sample_rate()))) {
            throw error{err, "lame_set_in_samplerate"};
        }
        if (LAME_NOERROR != (err = lame_set_num_channels(handle.get(), fmt.channel_count()))) {
            throw error{err, "lame_set_num_channels failed"};
        }
        if (LAME_NOERROR != (err = lame_set_quality(handle.get(), lame_quality))) {
            throw error{err, "lame_set_quality failed"};
        }

        auto pos = fmt.sample_format().find('.');
        if (pos != fmt.sample_format().npos) {
            string_view vbr_mode = string_view{fmt.sample_format().data() + pos + 1, fmt.sample_format().length() - pos - 1};
            string_view bitrate;
            if ((pos = vbr_mode.find('@')) != vbr_mode.npos) {
                bitrate = vbr_mode;
                bitrate.remove_prefix(pos + 1);
                vbr_mode.remove_suffix(vbr_mode.length() - pos);
            }
            if (!vbr_mode.empty()) {
                set_property(mpeg::property::vbr, vbr_mode);
            }
            if (!bitrate.empty()) {
                set_property(property::bitrate, bitrate);
            }
        }
        id3tag_init(handle.get());
        id3tag_add_v2(handle.get());
    }

    void ensure_bitstream_init() {
        if (bitstream_init) {
            return;
        }
        int err;
        if (LAME_NOERROR != (err = lame_init_params(handle.get()))) {
            throw error{err, "lame_init_params failed"};
        }
        bitstream_init = true;
    }

    static void dryrun_bitstream_init(const file_format& fmt) {
        impl dryrun;
        dryrun.new_handle(fmt);
        dryrun.ensure_bitstream_init();
        // avoid attempt to flush stream in destructor
        dryrun.bitstream_init = false;
    }

    void open(std::unique_ptr<byte_stream> f, const file_format& fmt) {
        close();

        if (!fmt.file_type().empty() && fmt.file_type() != file_type::label::mpeg) {
            throw std::invalid_argument{"file_type is not mpeg"};
        }
        dryrun_bitstream_init(fmt);
        try {
            new_handle(fmt);
            format = fmt;
            format.set_file_type(file_type::label::mpeg);
            file = std::move(f);
            stream_start_position = file->position();
        } catch (std::exception&) {
            handle = {};
            file = {};

            throw;
        }
    }

    size_t seek(ssize_t off, int whence) {
        throw std::logic_error{"lame writer is not seekable"};
    }

    optional<string> property(string_view property) {
        static const std::unordered_map<string_view, optional<string>(*)(impl& i), absl::Hash<string_view>> property_map = {
            { mpeg::property::version, [](impl& i) -> optional<string> {
                switch (lame_get_version(i.handle.get())) {
                case 0:
                    return "2.0";
                case 1:
                    return "1.0";
                case 2:
                    return "2.5";
                default:
                    return {};
                }
            }},
            { mpeg::property::mode, [](impl& i) -> optional<string> {
                switch (lame_get_mode(i.handle.get())) {
                case STEREO:
                    return "Standard Stereo";
                case JOINT_STEREO:
                    return "Joint Stereo";
                case DUAL_CHANNEL:
                    return "Dual Channel";
                case MONO:
                    return "Mono";
                default:
                    return {};
                }
            }},
            { mpeg::property::layer, [](impl& i) -> optional<string> {
                return "3";
            }},
            { mpeg::property::vbr, [](impl& i) -> optional<string> {
                switch (lame_get_VBR(i.handle.get())) {
                case vbr_off:
                    return "CBR";
                case vbr_abr:
                    return "ABR";
                case vbr_mt:
                case vbr_rh:
                case vbr_mtrh:
                    return "VBR";
                default:
                    return {};
                }
            }},
            { property::bitrate, [](impl& i) -> optional<string> {
                int res;
                switch (lame_get_VBR(i.handle.get())) {
                case vbr_abr:
                    res = lame_get_VBR_mean_bitrate_kbps(i.handle.get());
                    break;
                default:
                    res = lame_get_brate(i.handle.get());
                }
                if (res > 0) {
                    return boost::str(boost::format("%1%k") % res);
                }
                return {};
            }},
        };
        auto it = property_map.find(property);
        if (it != property_map.end()) {
            return it->second(*this);
        }
        auto tag_it = tags.find(string{property});
        if (tag_it != tags.end()) {
            return tag_it->second;
        }
        return {};
    }

    void sync_abr_to_bitrate() {
        int res = lame_get_brate(handle.get());
        if (res > 0) {
            int err = lame_set_VBR_mean_bitrate_kbps(handle.get(), res);
            assert(LAME_NOERROR == err);
            lame_set_VBR_min_bitrate_kbps(handle.get(), res / 2);
            lame_set_VBR_max_bitrate_kbps(handle.get(), res + res / 2);
        }
    }

    void sync_bitrate_to_abr() {
        int res = lame_get_VBR_mean_bitrate_kbps(handle.get());
        if (res > 0) {
            int err =lame_set_brate(handle.get(), res);
            assert(LAME_NOERROR == err);
        }
    }

    template<const char* property>
    static void throw_read_only(impl& i, string_view) {
        throw snd::property::error::read_only{property};
    }

    void set_tag(uint32_t tag, string_view desc, string_view text) {
        char fid[5] = { (char)(tag >> 24), (char)(tag >> 16), (char)(tag >> 8), (char)tag, 0 };
        string formatted;
        if (tag == 'TXXX' || tag == 'WXXX' || tag == 'COMM') {
            formatted = boost::str(boost::format("%1%=%2%=%3%") % fid % desc % text);
        } else {
            assert(desc.empty());
            formatted = boost::str(boost::format("%1%=%2%") % fid % text);
        }
        int err = id3tag_set_fieldvalue(handle.get(), formatted.c_str());
        assert(0 == err);
    }

    void set_track_disk_num(uint32_t tag, string prop, string track_disk_count_prop, string val) {
        if (val.find('/') != val.npos) {
            throw property::error::invalid_value{prop, val};
        }
        tags[prop] = val;
        auto total_it = tags.find(track_disk_count_prop);
        if (total_it != tags.end() && !total_it->second.empty()) {
            val = boost::str(boost::format("%1%/%2%") % val % total_it->second);
        }
        set_tag(tag, "", val);
    }

    void set_track_disk_count(uint32_t tag, string prop, string track_disk_number_prop, string val) {
        if (val.find('/') != val.npos) {
            throw property::error::invalid_value{prop, val};
        }
        tags[prop] = val;
        string num;
        auto num_it = tags.find(track_disk_number_prop);
        if (num_it != tags.end()) {
            num = num_it->second;
        }
        if (val.empty()) {
            val = boost::str(boost::format("%1%") % num );
        } else {
            val = boost::str(boost::format("%1%/%2%") % num % val);
        }
        set_tag(tag, "", val);
    }

    static const std::unordered_map<string_view, void(*)(impl& i, string_view val), absl::Hash<string_view>> PROPERTY_SETTERS;

    void set_property(string_view prop, string_view val) {
        auto it = PROPERTY_SETTERS.find(prop);
        if (it != PROPERTY_SETTERS.end()) {
            it->second(*this, val);
            return;
        }
        auto tag_it = PROPERTY_TAGS.find(prop);
        if (tag_it != PROPERTY_TAGS.end()) {
            set_tag(tag_it->second, "", val);
        } else {
            set_tag('TXXX', prop, val);
        }
        tags[string{prop}] = string{val};
    }

    template<typename Sample>
    size_t write_frames(const Sample* samples, size_t frames) {
        ensure_bitstream_init();
        size_t total = 0;
        while (true) {
            size_t write_size = std::min(WRITE_SAMPLE_COUNT / format.channel_count(), frames - total);
            buffer.resize(static_cast<size_t>(1.25 * write_size * format.channel_count() + 7200.5));
            int res = lame_encode_func<Sample>{}(handle.get(), samples, write_size, &buffer[0], buffer.size(), format.channel_count());
            if (res < 0) {
                throw error{res, "lame_encode_buffer_interleaved variant failed"};
            }
            total += write_size;
            frame_count += write_size;
            file->write(&buffer[0], res);
            if (total == frames) {
                return total;
            }
        }
    }
};


const std::unordered_map<string_view, void(*)(writer::impl& i, string_view val), absl::Hash<string_view>> writer::impl::PROPERTY_SETTERS = {
    { mpeg::property::version, throw_read_only<mpeg::property::version> },
    { mpeg::property::layer, throw_read_only<mpeg::property::layer> },
    { mpeg::property::mode, [](impl& i, string_view val) {
        MPEG_mode mode;
        if (detail::istrequal(val, string_view{"Standard Stereo"})) {
            mode = STEREO;
        } else if (detail::istrequal(val, string_view{"Joint Stereo"})) {
            mode = JOINT_STEREO;
        // LAME doesn't support dual channel
        // } else if (detail::istrequal(val, string_view{"Dual Channel"})) {
        //     mode = DUAL_CHANNEL;
        } else if (detail::istrequal(val, string_view{"Mono"})) {
            mode = MONO;
        } else {
            throw property::error::invalid_value{mpeg::property::mode, string{val}};
        }
        int err;
        if (LAME_NOERROR != (err = lame_set_mode(i.handle.get(), mode))) {
            throw error{err, "lame_set_mode failed"};
        }
    }},
    { mpeg::property::vbr, [](impl& i, string_view val) {
        vbr_mode mode;
        if (detail::istrequal(val, string_view{"CBR"})) {
            mode = vbr_off;
        } else if (detail::istrequal(val, string_view{"ABR"})) {
            mode = vbr_abr;
        } else if (detail::istrequal(val, string_view{"VBR"})) {
            mode = vbr_mtrh;
        } else {
            throw property::error::invalid_value{mpeg::property::vbr, string{val}};
        }
        int old_mode = lame_get_VBR(i.handle.get());
        int err;
        if (LAME_NOERROR != (err = lame_set_VBR(i.handle.get(), mode))) {
            throw error{err, "lame_set_VBR failed"};
        }
        if (mode != old_mode) {
            if (mode == vbr_abr) {
                i.sync_abr_to_bitrate();
            } else if (old_mode == vbr_abr) {
                i.sync_bitrate_to_abr();
            }
        }
    }},
    { property::bitrate, [](impl& i, string_view val) {
        string_view num = val;
        if (!num.empty() && std::tolower(num.back()) == 'k') {
            num.remove_suffix(1);
        }
        int res = 0;
        if (!absl::SimpleAtoi(num, &res) || res < 0) {
            throw property::error::invalid_value{property::bitrate, string{val}};
        }
        int err;
        switch (lame_get_VBR(i.handle.get())) {
        case vbr_abr:
            if (LAME_NOERROR != (err = lame_set_VBR_mean_bitrate_kbps(i.handle.get(), res))) {
                throw error{err, "lame_set_VBR_mean_bitrate_kbps failed"};
            }
            lame_set_VBR_min_bitrate_kbps(i.handle.get(), res / 2);
            lame_set_VBR_max_bitrate_kbps(i.handle.get(), res + res / 2);
            break;
        default:
            if (LAME_NOERROR != (err = lame_set_brate(i.handle.get(), res))) {
                throw error{err, "lame_set_brate failed"};
            }
        }
    }},
    { property::track_number, [](impl& i, string_view val) {
        i.set_track_disk_num('TRCK', property::track_number, property::track_count, string{val});
    }},
    { property::track_count, [](impl& i, string_view val) {
        i.set_track_disk_count('TRCK', property::track_count, property::track_number, string{val});
    }},
    { property::disk_number, [](impl& i, string_view val) {
        i.set_track_disk_num('TPOS', property::disk_number, property::disk_count, string{val});
    }},
    { property::disk_count, [](impl& i, string_view val) {
        i.set_track_disk_count('TPOS', property::disk_count, property::disk_number, string{val});
    }}
};


writer::writer():
    impl_{new impl}
{}

writer::~writer() {
}

void writer::open(const char* path, const file_format& format) {
    impl_->open(std::make_unique<stdio_stream>(path, "w+b"), format);
}

void writer::open(std::unique_ptr<byte_stream> stream, const file_format& format) {
    impl_->open(std::move(stream), format);
}

void writer::close() {
    impl_->close();
}

bool writer::is_open() const {
    return impl_->handle != nullptr;
}

const file_format& writer::format() const {
    return impl_->format;
}

size_t writer::frame_count() const {
    return impl_->frame_count;
}

unsigned writer::sample_rate() const {
    return impl_->format.sample_rate();
}

unsigned writer::channel_count() const {
    return impl_->format.channel_count();
}

size_t writer::seek(ssize_t offset, int whence) {
    return impl_->seek(offset, whence);
}

optional<string> writer::property(string_view prop) {
    return impl_->property(prop);
}

void writer::set_property(string_view prop, string_view val) {
    impl_->set_property(prop, val);
}

size_t writer::write_frames(const float* buf, size_t count) {
    return impl_->write_frames(buf, count);
}

size_t writer::write_frames(const short* buf, size_t count) {
    return impl_->write_frames(buf, count);
}

size_t writer::write_frames(const int* buf, size_t count) {
#ifndef DSPXX_LAME_NO_INT_API
    return impl_->write_frames(buf, count);
#else
    // TODO implement me
    throw std::logic_error{"not implemented"};
#endif
}

size_t writer::write_frames(const double* buf, size_t count) {
    return impl_->write_frames(buf, count);
}

void writer::commit() {
}

}}}

#endif // !DSPXX_LAME_DISABLED
