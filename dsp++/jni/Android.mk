TARGET_ARCH_ABI := armeabi-v7a
LOCAL_PATH := $(call my-dir)
NDK_PROJECT_PATH := $(LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE := sndfile
LOCAL_SRC_FILES := $(LOCAL_PATH)/deps/libsndfile-android/obj/local/$(TARGET_ARCH_ABI)/libsndfile.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/deps/libsndfile-android/jni
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -DDSP_FFTW_DISABLED=1 -DDSPXX_EXPORTS -DNDEBUG -fPIC -fvisibility=hidden 
LOCAL_CPPFLAGS := -std=gnu++11 
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_SHARED_LIBRARIES := pthread boost_atomic_shared sndfile

SRC := $(LOCAL_PATH)/../src

LOCAL_SRC_FILES := $(SRC)/arch/arm/cpu_arm.cpp \
  $(SRC)/debug.cpp $(SRC)/fft.cpp $(SRC)/filter.cpp $(SRC)/fixed.cpp $(SRC)/flt_biquad.cpp \
	$(SRC)/flt_fs.cpp $(SRC)/flt_iir.cpp $(SRC)/flt_pm.cpp $(SRC)/resample.cpp $(SRC)/sample.cpp \
	$(SRC)/simd.cpp $(SRC)/vectmath.cpp $(SRC)/zeropole.cpp $(SRC)/arch/x86/cpu_x86.cpp \
	$(SRC)/arch/x86/sse.cpp $(SRC)/arch/x86/sse3.cpp $(SRC)/arch/x86/sse41.cpp \
	$(SRC)/fftw/traits.cpp $(SRC)/mkfilter/mkfilter.cpp $(SRC)/remez/remez.cpp \
	$(SRC)/rpoly/rpoly.cpp $(SRC)/snd/format.cpp $(SRC)/snd/io.cpp $(SRC)/snd/loudness.cpp \

# include $(BUILD_STATIC_LIBRARY)

# include $(CLEAR_VARS)
LOCAL_MODULE := dsp++
# LOCAL_STATIC_LIBRARIES := dsp++-static
# LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

include $(BUILD_SHARED_LIBRARY)

$(call import-module,boost/1.57.0)
