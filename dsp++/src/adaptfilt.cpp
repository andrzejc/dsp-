#include <dsp++/adaptfilt.h>

dsp::filter_adapt_lms<float> lms(256, 0.1f);
dsp::filter_adapt_nlms<float> nlms(256, 0.1f);
