// Copyright 2011 Google Inc. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Windows type stubs for the open-source version of Snappy.

#ifndef THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_WIN32_H_
#define THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_WIN32_H_

#include <cstddef>
#include <cstdint>

#define SNAPPY_MAJOR 1
#define SNAPPY_MINOR 2
#define SNAPPY_PATCHLEVEL 2
#define SNAPPY_VERSION \
    ((SNAPPY_MAJOR << 16) | (SNAPPY_MINOR << 8) | SNAPPY_PATCHLEVEL)

namespace snappy {

// Windows doesn't have sys/uio.h, so define iovec structure
struct iovec {
  void* iov_base;
  size_t iov_len;
};

}  // namespace snappy

#endif  // THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_WIN32_H_
