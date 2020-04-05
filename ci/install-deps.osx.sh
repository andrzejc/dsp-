HOMEBREW_NO_AUTO_UPDATE=1 \
HOMEBREW_NO_INSTALL_CLEANUP=1 \
    brew install \
        ccache \
        cmake \
        ninja \
        mpg123 \
        lame \
        libsndfile \
        fdk-aac \
        fftw \
        doxygen \
        graphviz

        # boost \

export PATH="/usr/local/opt/ccache/libexec:$PATH"
