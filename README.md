dsp++ library
===============
This is README file for dsp++ repository, whose master copy is located at
[GitHub](https://github.com/andrzejc/dspxx).

Builds:

 * develop [![Build Status](https://travis-ci.org/andrzejc/dspxx.svg?branch=develop)](https://travis-ci.org/andrzejc/dspxx)
 * master [![Build Status](https://travis-ci.org/andrzejc/dspxx.svg?branch=master)](https://travis-ci.org/andrzejc/dspxx)

Introduction
------------
**dsp++ library** aims at delivering reusable digital signal processing
algorithms to C++ projects. The library is written in modern C++ with extensive
use of templates and STL constructs.

Repo contents
-------------
The repository hosts several "projects", with dsp++ library being the main one
and the others are used for testing and demonstrating some of the library
features. The contents of the directories is as follows:

 *  *dsp++* - the library project, probably the reason why you cloned this repo
 *  *dsp++-test* - library unit testing project using Boost.Test, updated when
    time allows
 *  *compressor\_demo* - a sample project demonstrating how to create very
    simple compressor using dsp++
 *  *filter\_demo* - a sample project demonstrating use of Matlab-designed
    filters with dsp++ filtering classes
 *  *multiband\_compressor* - a more complex sample demonstrating how to create
    multiband compressor algorihtm with dsp++

Building
--------
The only supported general-purpose solution for now is [CMake](https://cmake.org/). 
In the past there
were included project files for IDEs like Visual Studio, Eclipse and XCode, but
maintaining them was painful and didn't allow for using CI servers easily.

There's also possibility to build dsp++ for Android, using
[CrystaX NDK](https://www.crystax.net/android/ndk)  and
ndk-build tools.

Prerequisites
-------------
The dsp++ library has the following dependencies:

 *  *Boost* library - currently it's impossible to build nor use dsp++ without
    Boost. On Windows you'll probably use
    [binary distribution](https://sourceforge.net/projects/boost/files/boost-binaries/).
 *  *libsndfile* - on non-Windows systems just use package manager to install
    this in standard location and it should be automagickally found by CMake. On
    Windows you'll probably want to install from
    [prebuilt binaries](http://www.mega-nerd.com/libsndfile/#Download).
 *  *fftw3* (optional) - on non-Windows system install as usual. On Windows
    download [prebuild binaries](http://www.fftw.org/install/windows.html) and extract it into `%ProgramFiles%`.
