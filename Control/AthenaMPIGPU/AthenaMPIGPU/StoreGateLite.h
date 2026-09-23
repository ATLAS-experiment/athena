// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHENAMPIGPU_STOREGATELITE_H
#define ATHENAMPIGPU_STOREGATELITE_H

// Local include(s)

// Athena include(s)

// Gaudi include(s)

// System include(s)
#include <cstddef>
#include <memory_resource>

namespace RemoteCall {

/// Lightweight event store for the remote GPU server.
class StoreGateLite {
 public:
  // Public interface

 private:
  std::size_t m_eventNumber{};
  std::pmr::memory_resource* m_deviceResource{};
};

}  // namespace RemoteCall

#endif  // ATHENAMPIGPU_STOREGATELITE_H
