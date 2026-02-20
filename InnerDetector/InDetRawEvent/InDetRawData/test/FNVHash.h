/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef _FNVHASH_H_
#define _FNVHASH_H_
#include <span>

// Computes a FNV-0 hash for a data stream to test whether
// two or more data streams are likely identical.
// See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
struct FNVHash {
    unsigned int FNV_prime = 12289;
    unsigned int offset_basis = 0;
    unsigned int counter = 0;

   unsigned int value() const { return offset_basis; }
   operator unsigned int() const { return value(); }

   template <typename T>
   unsigned int add(const T &data) {
      return addData( std::span(reinterpret_cast< const char *>(&data), sizeof(T) ));
   }
   unsigned int addData(std::span<const char> data) {
      char last_char=0;
      for (unsigned int i=0; i < data.size(); ++i) {
        char the_char=data[i];
        if (the_char==last_char) continue;
        last_char=the_char;

        offset_basis = (offset_basis * FNV_prime) & 0xffffffff;
        offset_basis = offset_basis ^ the_char;
        ++counter;
      }
      return offset_basis;
   }
};
#endif
