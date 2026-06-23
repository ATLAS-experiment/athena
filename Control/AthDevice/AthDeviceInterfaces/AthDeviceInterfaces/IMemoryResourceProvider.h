//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_IMEMORYRESOURCEPROVIDER_H
#define ATHDEVICEINTERFACES_IMEMORYRESOURCEPROVIDER_H

// System include(s).
#include <memory_resource>

namespace AthDevice {

/// Interface for a component that provides a "memory resource"
class IMemoryResourceProvider {

 public:
  /// Destructor
  virtual ~IMemoryResourceProvider() = default;

  /// Get the provided @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mr() const = 0;

};  // class IMemoryResourceProvider

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_IMEMORYRESOURCEPROVIDER_H
