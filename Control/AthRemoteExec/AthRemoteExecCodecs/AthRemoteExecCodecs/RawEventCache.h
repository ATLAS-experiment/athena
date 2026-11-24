/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCODECS_RAWEVENTCACHE_H
#define ATHREMOTEEXECCODECS_RAWEVENTCACHE_H

#include <AthenaKernel/CLASS_DEF.h>
#include <ByteStreamData/RawEvent.h>

#include <cstdint>
#include <utility>
#include <vector>

namespace AthRemoteExec {

/// Class that allows for safe holding of RawEvent data and pointer throughout whole remote execution. To be stored in
/// StoreGate via WriteHandle.
class RawEventCache {
public:
  /// @param words contiguous eformat FullEvent fragment
  explicit RawEventCache(std::vector<uint32_t>&& words) : m_words(std::move(words)), m_event(m_words.data()) {}

  RawEventCache(const RawEventCache&) = delete;
  RawEventCache& operator=(const RawEventCache&) = delete;
  RawEventCache(RawEventCache&&) = delete;
  RawEventCache& operator=(RawEventCache&&) = delete;

  const RawEvent* event() const { return &m_event; }

  const std::vector<uint32_t>& words() const { return m_words; }

private:
  std::vector<uint32_t> m_words;
  RawEvent m_event;
};

}  // namespace AthRemoteExec

CLASS_DEF(AthRemoteExec::RawEventCache, 108881988, 1)

#endif  // ATHREMOTEEXECCODECS_RAWEVENTCACHE_H
