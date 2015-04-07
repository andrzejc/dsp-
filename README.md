# dsp++ library

This is README file for dsp++ repository, whose master copy is located at https://bitbucket.org/andrzejc/dsp.git.

## Introduction
**dsp++ library** aims at delivering reusable digital signal processing algorithms to C++ projects. The library is written in modern C++ with extensive use of templates and STL constructs.

## Repo contents

The repository hosts several "projects", with dsp++ library being the main one and the others are used for testing some of the library features. The contents of the directories is as follows:

* compressor_demo - a sample project demonstrating how to create very simple compressor using dsp++
* dsp++ - the library project, probably the reason why you cloned this repo
* dsp++-test - library unit testing project using CppUnit, being updated when time allows
* filter_demo - a sample project demonstrating use of Matlab-designed filters with dsp++ filtering classes
* multiband_compressor - a more complex sample demonstrating how to create multiband compressor algorihtm with dsp++
* scratchbook - utility project for testing code snippets

## Building

The main development platform is Windows/Visual Studio 2013. You'll find VS2013 solution file which includes the above-mentioned projects (dsp++-vc12.sln). There's also dsp++.sln for vc10 which is updated ocassionally and outdated most of the time. The dsp++ library is being ocassionally tested & updated to build on:

* Windows/Visual Studio 2013 (main)
* Ubuntu (using hand-crafted Makefiles)
* Scientific Linux 7 (Makefiles)
* Mac OS X (Makefiles + MacPorts)
* Android (Android.mk)

The specific build instructions for platforms are in respective readme files in dsp++ directory.

At some point of time there were also created IDE projects for dsp++ for XCode (Mac OS X) and Eclipse (Windows/MinGW, Linux, Mac OS X). These are not being updated until need arises, so there's no guarantee any of these will work, but they may be used as a base.

There's also possibility to build dsp++ for Android, using CrystaX NDK. See dsp++/android/README.

## Prerequisites

The dsp++ library has the following dependencies:

* Boost library - currently it's impossible to build nor use dsp++ without Boost. On Windows you'll probably use binary distribution, in that case add BOOST_ROOT environment variable pointing at the Boost installation dir.
* libsndfile - (LIBSNDFILE_ROOT env var)
* fftw - (FFTW_ROOT env var)
* cppunit - only for dsp++-test project