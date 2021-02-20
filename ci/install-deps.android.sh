set -e -o pipefail
trap 'set +e +o pipefail +x' EXIT RETURN
set -x

export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${HOME}/Android/Sdk}"
"${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin/sdkmanager" --install --channel=1 \
    'cmake;3.18.1'

function sdk_component_path() {
    local component="$1"
    echo -n "${ANDROID_SDK_ROOT}/"
    "${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin/sdkmanager" --list_installed | \
        grep "${component}" | \
        tail -n1 | \
        awk -F\| '{ gsub(/[ ]+/, "", $4); print $4 }'
}

export NDK=$( sdk_component_path ndk)
export CMAKE=$( sdk_component_path 'cmake;3.18.1' )/bin/cmake

mkdir -p built-deps
export BUILT_DEPS_DIR=$( cd built-deps && pwd )

pushd built-deps
echo "*" > .gitignore
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/boost-1.74.0.tar.bz2
tar xf boost-1.74.0.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/fftw-3.3.9.tar.bz2
tar xf fftw-3.3.9.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/libsndfile-1.0.31.tar.bz2
tar xf libsndfile-1.0.31.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/flac-1.3.3.tar.bz2
tar xf flac-1.3.3.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/libogg-1.3.4.tar.bz2
tar xf libogg-1.3.4.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/libvorbis-1.3.7.tar.bz2
tar xf libvorbis-1.3.7.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/opus-1.3.1.tar.bz2
tar xf opus-1.3.1.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/speex-1.2.0.tar.bz2
tar xf speex-1.2.0.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/mpg123-1.26.4.tar.bz2
tar xf mpg123-1.26.4.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/lame-3.100.tar.bz2
tar xf lame-3.100.tar.bz2 --strip-components=1
popd
