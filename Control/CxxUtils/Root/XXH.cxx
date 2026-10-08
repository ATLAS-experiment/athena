/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
#include "CxxUtils/XXH.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

std::uint64_t xxh3::hash64(const void* data, std::size_t size) {
  return XXH3_64bits(data, size);
}
