dsp++ library
===============
This is README file for dsp++ repository, whose master copy is located at
[Bitbucket](https://bitbucket.org/andrzejc/dsp.git).

Builds:

 *  Ubuntu [![Build Status](https://drone.io/bitbucket.org/andrzejc/dsp/status.png)](https://drone.io/bitbucket.org/andrzejc/dsp/latest)
 *  Windows #WIP
 *  MacOS X #WIP

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
The only supported general-purpose solution for now is CMake. In the past there
were included project files for IDEs like Visual Studio, Eclipse and XCode, but
maintaining them was painful and didn't allow for using CI servers easily.

There's also possibility to build dsp++ for Android, using CrystaX NDK and
ndk-build tools. This is not integrated with CMake (YET).
See dsp++/android/README.

Prerequisites
-------------
The dsp++ library has the following dependencies:

 *  *Boost* library - currently it's impossible to build nor use dsp++ without
    Boost. On Windows you'll probably use
    [binary distribution](https://sourceforge.net/projects/boost/files/boost-binaries/),
    in that case add `BOOST_ROOT` environment variable pointing at the Boost
    installation dir and the rest should be automagickally done by CMake
    FindBoost macro (with a little help).
 *  *libsndfile* - on non-Windows systems just use package manager to install
    this in standard location and it should be automagickally found by CMake. On
    Windows you'll probably want to install from
    [prebuilt binaries](http://www.mega-nerd.com/libsndfile/#Download). In such case,
    define `LIBSNDFILE_ROOT` environment variable, and install 32-bit binary in
    `%LIBSNDFILE_ROOT%\x86` and 64-bit one in `%LIBSNDFILE_ROOT%\x64`.
 *  *fftw3* (optional) - on non-Windows system install as usual. On Windows
    download [prebuild binaries](http://www.fftw.org/install/windows.html) and
    use `FFTW3_ROOT` just like with `LIBSNDFILE_ROOT` above (inluding x86/x64
    subdirs). You will also need to generate import libs the way it's described
    under the link above.