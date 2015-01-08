CXX=g++
CFLAGS=-DNDEBUG -I../dsp++/include -std=gnu++11 -O2
CXXFLAGS=$(CFLAGS)

SRC=src/adaptfilt_test.cpp src/execution_timer.cpp \
	src/fft_test.cpp src/filter_design_test.cpp src/filter_test.cpp src/intmath_test.cpp \
	src/lattice_test.cpp src/levinson_test.cpp src/loudness_test.cpp src/lpc_test.cpp \
	src/main.cpp src/mean_test.cpp src/overlap_add_test.cpp src/pow2_test.cpp \
	src/resample_test.cpp src/simd_test.cpp src/window_test.cpp src/xcorr_test.cpp

OBJ = $(patsubst %.cpp,%.o, $(SRC))

LIBS = -lcppunit -L../dsp++ -ldsp++

all: dsp++-test

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

dsp++-test: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJ)


