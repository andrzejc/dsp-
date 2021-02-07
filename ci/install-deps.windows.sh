choco install \
    openssl.light \
    doxygen.install

case "${TARGET_PLATFORM}" in
*64)
    winstall-exe https://sourceforge.net/projects/boost/files/boost-binaries/1.72.0/boost_1_72_0-msvc-14.1-64.exe f49cd0c11976a9bb4e5edd3e8111e61fd0b859ac /VERYSILENT
    winstall-libsndfile "${TARGET_PLATFORM}" http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28-w64-setup.exe 3783e513d735d1526f19a32a63991026
    winstall-fftw3 "${TARGET_PLATFORM}" ftp://ftp.fftw.org/pub/fftw/fftw-3.3.5-dll64.zip 409f4a5272506eb7422d265a68a02deeefcb5c17
    winstall-mpg123 "${TARGET_PLATFORM}" https://www.mpg123.de/download/win64/1.26.4/mpg123-1.26.4-x86-64.zip 3aabbb3856ef4cc746dc4865d50d25e67169c882
    winstall-lame "${TARGET_PLATFORM}" https://github.com/andrzejc/libmp3lame-windows-release/releases/download/3.100/lame-3.100-win-amd64.zip b1286ef97251d1f64e1f47a87cf462eb84447561
    ;;
*in32|*86)
    winstall-exe https://sourceforge.net/projects/boost/files/boost-binaries/1.72.0/boost_1_72_0-msvc-14.1-32.exe 121e069354cc792a3b1bb42fe7285e5380beea45 /VERYSILENT
    winstall-libsndfile "${TARGET_PLATFORM}" http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28-w32-setup.exe 443a2a2890969778e8f9fe6a146c0595
    winstall-fftw3 "${TARGET_PLATFORM}" ftp://ftp.fftw.org/pub/fftw/fftw-3.3.5-dll32.zip b19d875b07a0e4ac8251b0aa072321582374e000
    winstall-mpg123 "${TARGET_PLATFORM}" https://www.mpg123.de/download/win32/1.26.4/mpg123-1.26.4-x86.zip a6add8a796db082e970bfdb01f6ba550f774268d
    winstall-lame "${TARGET_PLATFORM}" https://github.com/andrzejc/libmp3lame-windows-release/releases/download/3.100/lame-3.100-win-x86.zip 609bcfd655c24b7a2dfbb73c6e564bd23fb6d09c
    ;;
*)
    >&2 echo "Installation of libsndfile for target '${TARGET_PLATFORM}' is not supported"
    return 1 ;;
esac
