PREFIX=/usr/local
CXX=g++
CFLAGS=-DDSPXX_EXPORTS -DNDEBUG -fvisibility=hidden -I./include -fPIC -std=gnu++11 -O2
CXXFLAGS=$(CFLAGS)

LIBS = -lfftw3 -lfftw3_threads -lfftw3f -lfftw3f_threads -lfftw3l -lfftw3l_threads \
	-lsndfile

ifeq ($(OS),Windows_NT)
EXT := .dll
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
EXT := .dylib
CFLAGS += -I/opt/local/include
LIBS += -L/opt/local/lib 
else
EXT := .so
endif
endif

PRODUCT = libdsp++$(EXT)

SRC=src/debug.cpp src/fft.cpp src/filter.cpp src/fixed.cpp src/flt_biquad.cpp \
	src/flt_fs.cpp src/flt_iir.cpp src/flt_pm.cpp src/resample.cpp src/sample.cpp \
	src/simd.cpp src/vectmath.cpp src/zeropole.cpp src/arch/x86/cpu_x86.cpp \
	src/arch/x86/sse.cpp src/arch/x86/sse3.cpp src/arch/x86/sse41.cpp \
	src/fftw/traits.cpp src/mkfilter/mkfilter.cpp src/remez/remez.cpp \
	src/rpoly/rpoly.cpp src/snd/format.cpp src/snd/io.cpp src/snd/loudness.cpp

OBJ = $(patsubst %.cpp,%.o, $(SRC))

all: $(PRODUCT)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

src/arch/x86/sse.o: src/arch/x86/sse.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -msse

src/arch/x86/sse3.o: src/arch/x86/sse3.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -msse3

src/arch/x86/sse41.o: src/arch/x86/sse41.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -msse4.1

$(PRODUCT): $(OBJ)
	$(CXX) -shared -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean docs install uninstall clean-docs clean-all all

docs: doxyfile
	doxygen doxyfile

uninstall:
	rm -f $(PREFIX)/lib/$(PRODUCT)
	rm -rf $(PREFIX)/include/dsp++
	rm -rf $(PREFIX)/share/doc/dsp++/html

install: all docs
	cp $(PRODUCT) $(PREFIX)/lib/$(PRODUCT)
	cp -r include/dsp++ $(PREFIX)/include/dsp++
	mkdir -p $(PREFIX)/share/doc/dsp++
	cp -r doc/html $(PREFIX)/share/doc/dsp++/html

clean:
	rm -f $(OBJ)

clean-docs:
	rm -rf doc

clean-all: clean clean-docs
	rm -f $(PRODUCT)

	

