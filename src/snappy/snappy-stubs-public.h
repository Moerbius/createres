// Platform-specific snappy-stubs-public.h dispatch
// This header includes the appropriate platform-specific stubs

#ifndef THIRD_PARTY_SNAPPY_STUBS_PUBLIC_DISPATCH_H_
#define THIRD_PARTY_SNAPPY_STUBS_PUBLIC_DISPATCH_H_

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32_CONFIG)
  // Windows build
  #include "snappy-stubs-public-win32.h"
#else
  // Linux/Unix build (default)
  #include "snappy-stubs-public-linux.h"
#endif

#endif  // THIRD_PARTY_SNAPPY_STUBS_PUBLIC_DISPATCH_H_
