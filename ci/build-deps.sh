
DEPS_DIR=$( cd deps && pwd )

mkdir -p build/deps/install
DEPS_INSTALL_DIR=$( cd build/deps/install && pwd )

if [[ "${TRAVIS_OS_NAME:-}" == osx ]]; then
    function cd {
        set +e && builtin cd "$@" && set -e
    }
    function pushd {
        set +e && builtin pushd "$@" && set -e
    }
    function popd {
        set +e && builtin popd "$@" && set -e
    }
fi

mkdir -p build/deps/googletest
pushd build/deps/googletest
cmake ${TARGET_PLATFORM:+-A ${TARGET_PLATFORM}} \
    ${CMAKE_GENERATOR:+"-DCMAKE_GENERATOR=${CMAKE_GENERATOR}"} \
    "-DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_DIR}" \
    -Dgtest_force_shared_crt=ON \
    -DBUILD_SHARED_LIBS=OFF \
    "${DEPS_DIR}/googletest"
cmake --build . --config RelWithDebInfo --target install
popd

mkdir -p build/deps/abseil-cpp
pushd build/deps/abseil-cpp
cmake ${TARGET_PLATFORM:+-A ${TARGET_PLATFORM}} \
    ${CMAKE_GENERATOR:+"-DCMAKE_GENERATOR=${CMAKE_GENERATOR}"} \
    "-DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_DIR}" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    "${DEPS_DIR}/abseil-cpp"
cmake --build . --config RelWithDebInfo --target install
popd
