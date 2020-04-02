#include "test_utility.h"

#if defined(DSPXX_OS_FAMILY_UNIX) || defined(DSPXX_OS_MACOSX)
#include <unistd.h>
#endif

#include <system_error>

namespace dsp { namespace test {

temp_file::temp_file() {
#if defined(DSPXX_OS_FAMILY_UNIX) || defined(DSPXX_OS_MACOSX)
    const char* dir = std::getenv("TMPDIR");
    if (dir == nullptr) {
  #ifdef P_tmpdir
        dir = P_tmpdir;
  #else
        dir = "/tmp";
  #endif
    }
    std::strncat(name, dir, L_tmpnam);
    std::strncat(name, "/dspxxXXXXXX", L_tmpnam);
    int fd = ::mkstemp(name);
    if (fd == -1) {
        throw std::system_error{errno, std::generic_category()};
    }
    ::close(fd);
#else
    std::tmpnam(name);
#endif
}

}}
