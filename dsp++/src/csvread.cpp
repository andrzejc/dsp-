#include <dsp++/config.h>
#include <dsp++/csvread.h>
// TODO: create exception classes based on std:ios:fail for i/o operation failures
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>

namespace {

template<class T>
const char* sscanf_spec();

template<> inline
const char* sscanf_spec<float>() {return "%g";}

template<> inline
const char* sscanf_spec<double>() {return "%lg";}

template<class T>
void csvread_impl(std::istream& f, std::vector<std::vector<T>>& vec)
{
	while (f)
	{
		std::string line;
        std::getline(f, line);
		if (!f)
			break;

		vec.push_back(std::vector<T>());
		std::vector<T>& v = vec.back();

        std::stringstream  ss(line);
        std::string cell;

        while (std::getline(ss, cell, ','))
		{
#ifndef DSP_BOOST_DISABLED
			boost::algorithm::trim(cell);
			v.push_back(boost::lexical_cast<T>(cell));
#else // DSP_BOOST_DISABLED
			T var;
			if (1 != sscanf(cell.c_str(), sscanf_spec<T>(), &var))
				throw std::runtime_error("csvread: unable to read floating-point value \"" + cell + "\"");
			v.push_back(var);
#endif // DSP_BOOST_DISABLED
		}

		if (v.empty())
			vec.pop_back();
	}
	if (!f.eof())
		throw std::runtime_error("csvread: error reading CSV stream");
}

}

void dsp::csvread(std::istream& is, std::vector<std::vector<float>>& vec)
{
	csvread_impl(is, vec);
}

void dsp::csvread(std::istream& is, std::vector<std::vector<double>>& vec)
{
	csvread_impl(is, vec);
}

void dsp::csvread(const char* path, std::vector<std::vector<float>>& vec)
{
	std::ifstream f(path);
	csvread_impl(f, vec);
}

void dsp::csvread(const char* path, std::vector<std::vector<double>>& vec)
{
	std::ifstream f(path);
	csvread_impl(f, vec);
}

void dsp::csvread(const wchar_t* path, std::vector<std::vector<float>>& vec)
{
#ifdef _MSC_VER
	std::ifstream f(path);
	csvread_impl(f, vec);
#endif
}

void dsp::csvread(const wchar_t* path, std::vector<std::vector<double>>& vec)
{
#ifdef _MSC_VER
	std::ifstream f(path);
	csvread_impl(f, vec);
#endif
}

void dsp::csvread(const std::string& path, std::vector<std::vector<float>>& vec)
{
	std::ifstream f(path);
	csvread_impl(f, vec);
}

void dsp::csvread(const std::string& path, std::vector<std::vector<double>>& vec)
{
	std::ifstream f(path);
	csvread_impl(f, vec);
}

void dsp::csvread(const std::wstring& path, std::vector<std::vector<float>>& vec)
{
#ifdef _MSC_VER
	std::ifstream f(path);
	csvread_impl(f, vec);
#else
	csvread(path.c_str(), vec);
#endif
}

void dsp::csvread(const std::wstring& path, std::vector<std::vector<double>>& vec)
{
#ifdef _MSC_VER
	std::ifstream f(path);
	csvread_impl(f, vec);
#else
	csvread(path.c_str(), vec);
#endif
}
