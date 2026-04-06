#ifndef THIRD_PARTY_SNAPPY_CONFIG_DISPATCH_H_
#define THIRD_PARTY_SNAPPY_CONFIG_DISPATCH_H_

// Platform detection and config dispatch header
// This header includes the appropriate platform-specific config

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32_CONFIG)
  // Windows build
  #include "config-win32.h"
#else
  // Linux/Unix build (default)
  #include "config-linux.h"
#endif

#endif  // THIRD_PARTY_SNAPPY_CONFIG_DISPATCH_H_
