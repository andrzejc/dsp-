/*!
 * @file dsp++/csvread.h
 * @brief Utilities for reading data from CSV files.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_CSVREAD_H_INCLUDED
#define DSP_CSVREAD_H_INCLUDED

#include <dsp++/export.h>
#include <iosfwd>
#include <string>
#include <vector>

namespace dsp {

/// @brief Read rows of CSV file as vectors of samples.
/// @param [in] is input stream
/// @param [out] vec vector of vectors representing rows of data
/// @throw std::exception if file has malformed data
DSPXX_API void csvread(std::istream& is, std::vector<std::vector<float> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
DSPXX_API void csvread(std::istream& is, std::vector<std::vector<double> >& vec);

/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const std::wstring& path, std::vector<std::vector<float> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const std::wstring& path, std::vector<std::vector<double> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const std::string& path, std::vector<std::vector<float> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const std::string& path, std::vector<std::vector<double> >& vec);

/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const wchar_t* path, std::vector<std::vector<float> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const wchar_t* path, std::vector<std::vector<double> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const char* path, std::vector<std::vector<float> >& vec);
/// @overload csvread(std::istream&, std::vector<std::vector<float> >&)
/// @param [in] path CSV file path
DSPXX_API void csvread(const char* path, std::vector<std::vector<double> >& vec);

}

#endif /* DSP_CSVREAD_H_INCLUDED */
