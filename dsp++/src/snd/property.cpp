#include <dsp++/snd/property.h>

#include <boost/format.hpp>

namespace dsp { namespace snd { namespace property {

namespace error {

unsupported::unsupported(string name, string_view file_format):
    error{boost::str(boost::format("property \"%1%\" is not supported by file format \"%2%\"")
        % name % file_format)},
    name{std::move(name)}
{}

read_only::read_only(string name):
    error{boost::str(boost::format("can't set read-only property \"%1%\"") % name)},
    name{std::move(name)}
{}

invalid_value::invalid_value(string property, string value):
    error{boost::str(boost::format("property \"%1%\" doesn't allow value \"%2%\"") % property % value)},
    property{std::move(property)},
    value{std::move(value)}
{}

}

}}}