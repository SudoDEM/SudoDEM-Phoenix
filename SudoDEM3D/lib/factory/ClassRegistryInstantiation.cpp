/*************************************************************************
*  Explicit instantiation for ClassRegistry singleton
*  This file provides the explicit template instantiation needed for
*  Singleton<ClassRegistry>::self to be exported from the library.
*************************************************************************/

#include <sudodem/lib/factory/ClassRegistry.hpp>

// Explicit instantiation of the static member Singleton<ClassRegistry>::self BEFORE the class instantiation
// This must come before the class instantiation
template<> ClassRegistry* Singleton<ClassRegistry>::self = nullptr;

// Explicit template instantiation with default visibility
// This ensures the symbol is exported from libsudodem.dylib
template class Singleton<ClassRegistry>;
