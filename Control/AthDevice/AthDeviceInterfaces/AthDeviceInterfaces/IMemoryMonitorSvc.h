//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_IMEMORYMONITORSVC_H
#define ATHDEVICEINTERFACES_IMEMORYMONITORSVC_H

// Framework include(s).
#include "GaudiKernel/IService.h"

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <cstddef>
#include <string>

namespace AthDevice {

/// Interface to a service that provides "device memory monitoring"
class IMemoryMonitorSvc : virtual public IService {

 public:
  /// Declare the interface that the service will implement
  DeclareInterfaceID(IMemoryMonitorSvc, 1, 0);

  /// Destructor
  virtual ~IMemoryMonitorSvc() = default;

  /// @name Memory allocation monitoring
  /// @{

  /// Notify the service that a memory allocation has been made
  virtual void memoryAllocated(const std::string& component,
                               const std::string& device,
                               std::size_t bytes) = 0;
  /// Notify the service that a memory deallocation has been made
  virtual void memoryDeallocated(const std::string& component,
                                 const std::string& device,
                                 std::size_t bytes) = 0;

  /// @}

  /// @name Memory copy monitoring
  /// @{

  /// Notify the service that a memory copy has been made
  virtual void memoryCopied(const std::string& component,
                            vecmem::copy::type::copy_type type,
                            std::size_t bytes) = 0;

  /// @}

};  // class IMemoryMonitorSvc

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_IMEMORYMONITORSVC_H
