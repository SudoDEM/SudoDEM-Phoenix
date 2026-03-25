#include <sudodem/lib/base/Singleton.hpp>

#ifdef _WIN32
    SUDODEM_SINGLETON_API std::mutex singleton_constructor_mutex;
#endif