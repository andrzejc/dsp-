TARGET_ARCH_ABI := armeabi-v7a-hard
LOCAL_PATH := $(call my-dir)
DSPXX_LOCAL_PATH := $(LOCAL_PATH)
DEPS := $(LOCAL_PATH)/../deps
FFTW3_DIR := $(DEPS)/fftw3-android
LIBSNDFILE_DIR := $(DEPS)/libsndfile-android

include $(FFTW3_DIR)/jni/Android.mk
include $(LIBSNDFILE_DIR)/jni/Android.mk
LOCAL_PATH := $(DSPXX_LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -DDSPXX_EXPORTS -DNDEBUG -fPIC -fvisibility=hidden -DDSP_FFTW_HAVE_LONG_DOUBLE=0 -DDSP_FFTW_HAVE_QUAD=0
LOCAL_CPPFLAGS := -std=gnu++11 
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_SHARED_LIBRARIES := pthread boost_atomic_shared sndfile
LOCAL_STATIC_LIBRARIES := fftw3 fftw3f

SRC := ../../src

LOCAL_SRC_FILES := $(SRC)/arch/arm/cpu_arm.cpp \
  $(SRC)/debug.cpp $(SRC)/fft.cpp $(SRC)/filter.cpp $(SRC)/fixed.cpp $(SRC)/flt_biquad.cpp \
	$(SRC)/flt_fs.cpp $(SRC)/flt_iir.cpp $(SRC)/flt_pm.cpp $(SRC)/resample.cpp $(SRC)/sample.cpp \
	$(SRC)/simd.cpp $(SRC)/vectmath.cpp $(SRC)/zeropole.cpp $(SRC)/arch/x86/cpu_x86.cpp \
	$(SRC)/arch/x86/sse.cpp $(SRC)/arch/x86/sse3.cpp $(SRC)/arch/x86/sse41.cpp \
	$(SRC)/mkfilter/mkfilter.cpp $(SRC)/remez/remez.cpp \
	$(SRC)/rpoly/rpoly.cpp $(SRC)/snd/format.cpp $(SRC)/snd/io.cpp $(SRC)/snd/loudness.cpp \
	$(SRC)/fftw/traits.cpp 

LOCAL_MODULE := dsp++
include $(BUILD_SHARED_LIBRARY)

$(call import-module,boost/1.57.0)
