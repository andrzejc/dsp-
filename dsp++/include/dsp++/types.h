/// @file dsp++/types.h
/// @brief Common type aliases used throughout whole of dsp++ APIs.
/// @author Andrzej Ciarkowski <andrzej.ciarkowski@gmail.com>
#pragma once
#include <dsp++/config.h>
#include <dsp++/stdint.h>

#include <string_view>  // IWYU pragma: export
#include <optional>  // IWYU pragma: export
#include <cstddef>  // IWYU pragma: export
#include <string>  // IWYU pragma: export

namespace dsp {

/// @brief Contract: @c dsp::string is either @c std::string or has the same API and
/// is implicitly convertible to/from it.
using std::string;

/// @brief Contract: @c dsp::string_view is either @c std::string_view or has the same API and
/// is implicitly convertible to/from it.
using std::string_view;

using std::optional;

/// @brief Contract: @c dsp::size_t is alias for @c std::size_t.
using std::size_t;
/// @brief Contract: @c dsp::ssize_t is signed type of exactly the same size as @c dsp::size_t.
using ssize_t = signed_of_t<size_t>;

}
