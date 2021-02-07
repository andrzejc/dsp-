
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
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/mpg123-1.26.4.tar.bz2
tar xf mpg123-1.26.4.tar.bz2 --strip-components=1
wget -c -nv https://android-deps.s3-us-west-1.amazonaws.com/lame-3.100.tar.bz2
tar xf lame-3.100.tar.bz2 --strip-components=1
popd
