HOMEBREW_NO_AUTO_UPDATE=1 \
HOMEBREW_NO_INSTALL_CLEANUP=1 \
    brew install \
        abseil \
        ccache \
        cmake \
        libsndfile \
        fftw \
        boost \
        doxygen \
        graphviz

export PATH="/usr/local/opt/ccache/libexec:$PATH"
