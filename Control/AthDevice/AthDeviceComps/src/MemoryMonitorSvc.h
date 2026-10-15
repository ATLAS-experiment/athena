//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MEMORYMONITORSVC_H
#define ATHDEVICECOMPS_MEMORYMONITORSVC_H

// Framework include(s).
#include "AthenaBaseComps/AthService.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryMonitorSvc.h"

// System include(s).
#include <map>
#include <mutex>

namespace AthDevice {

/// Service providing "device memory monitoring"
class MemoryMonitorSvc : public extends<AthService, IMemoryMonitorSvc> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthService
  /// @{

  /// Finalize the service
  virtual StatusCode finalize() override;

  /// @}

  /// @}

  /// @name Function(s) inherited from @c IMemoryMonitorSvc
  /// @{

  /// Notify the service that a memory allocation has been made
  virtual void memoryAllocated(const std::string& component,
                               const std::string& device,
                               std::size_t bytes) override;

  /// Notify the service that a memory deallocation has been made
  virtual void memoryDeallocated(const std::string& component,
                                 const std::string& device,
                                 std::size_t bytes) override;

  /// Notify the service that a memory copy has been made
  virtual void memoryCopied(const std::string& component,
                            vecmem::copy::type::copy_type type,
                            std::size_t bytes) override;

  /// @}

 private:
  /// @name Objects collecting the monitoring data
  /// @{

  /// Key used in the memory allocation/deallocation monitoring maps
  struct MemoryAllocKey {
    /// Athena component making the memory (de)allocation
    std::string component;
    /// Name/identifier of the device on which the (de)allocation was made
    std::string device;

    /// Comparison operator, to make the type usable as a key in a map
    bool operator<(const MemoryAllocKey& other) const {
      if (device != other.device) {
        return device < other.device;
      } else {
        return component < other.component;
      }
    }
  };

  /// All the memory allocations
  std::map<MemoryAllocKey, std::size_t> m_allocations;
  /// Mutex for @c m_allocations
  std::mutex m_allocationsMutex;
  /// All the memory deallocations
  std::map<MemoryAllocKey, std::size_t> m_deallocations;
  /// Mutex for @c m_deallocations
  std::mutex m_deallocationsMutex;

  /// Key used in the memory copy monitoring map
  struct MemoryCopyKey {
    /// Athena component making the memory copy
    std::string component;
    /// Type of the memory copy
    vecmem::copy::type::copy_type type;

    /// Comparison operator, to make the type usable as a key in a map
    bool operator<(const MemoryCopyKey& other) const {
      if (component != other.component) {
        return component < other.component;
      } else {
        return type < other.type;
      }
    }
  };

  /// All the memory copies
  std::map<MemoryCopyKey, std::size_t> m_copies;
  /// Mutex for @c m_copies
  std::mutex m_copiesMutex;

  /// @}

};  // class MemoryMonitorSvc

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MEMORYMONITORSVC_H
