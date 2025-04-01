/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Common file for a few small hashing helpers to store things in
// std::unordered_*

#ifndef FTAG_HASH_H
#define FTAG_HASH_H

namespace FlavorTagInference {
  template<typename T>
  std::size_t getHash(const T& obj) {
    return std::hash<T>{}(obj);
  }

  // stolen from https://stackoverflow.com/a/27952689
  //
  // it doesn't have to be good, this isn't called much
  //
  inline size_t combine( size_t lhs, size_t rhs ) {
    lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
    return lhs;
  }
}

#endif
