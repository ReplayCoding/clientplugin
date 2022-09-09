//===- llvm/Support/LEB128.h - [SU]LEB128 utility functions -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <cstdint>

/// Utility function to decode a ULEB128 value.
inline uint64_t decodeULEB128(const uint8_t* p,
                              unsigned* n = nullptr,
                              const uint8_t* end = nullptr,
                              const char** error = nullptr) {
  const uint8_t* orig_p = p;
  uint64_t Value = 0;
  unsigned Shift = 0;
  if (error)
    *error = nullptr;
  do {
    if (p == end) {
      if (error)
        *error = "malformed uleb128, extends past end";
      if (n)
        *n = (unsigned)(p - orig_p);
      return 0;
    }
    uint64_t Slice = *p & 0x7f;
    if ((Shift >= 64 && Slice != 0) || Slice << Shift >> Shift != Slice) {
      if (error)
        *error = "uleb128 too big for uint64";
      if (n)
        *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value += Slice << Shift;
    Shift += 7;
  } while (*p++ >= 128);
  if (n)
    *n = (unsigned)(p - orig_p);
  return Value;
}

/// Utility function to decode a SLEB128 value.
inline int64_t decodeSLEB128(const uint8_t* p,
                             unsigned* n = nullptr,
                             const uint8_t* end = nullptr,
                             const char** error = nullptr) {
  const uint8_t* orig_p = p;
  int64_t Value = 0;
  unsigned Shift = 0;
  uint8_t Byte;
  if (error)
    *error = nullptr;
  do {
    if (p == end) {
      if (error)
        *error = "malformed sleb128, extends past end";
      if (n)
        *n = (unsigned)(p - orig_p);
      return 0;
    }
    Byte = *p;
    uint64_t Slice = Byte & 0x7f;
    if ((Shift >= 64 && Slice != (Value < 0 ? 0x7f : 0x00)) ||
        (Shift == 63 && Slice != 0 && Slice != 0x7f)) {
      if (error)
        *error = "sleb128 too big for int64";
      if (n)
        *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value |= Slice << Shift;
    Shift += 7;
    ++p;
  } while (Byte >= 128);
  // Sign extend negative numbers if needed.
  if (Shift < 64 && (Byte & 0x40))
    Value |= (-1ULL) << Shift;
  if (n)
    *n = (unsigned)(p - orig_p);
  return Value;
}

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t Value);

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t Value);
