//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MONITOREDCOPY_H
#define ATHDEVICECOMPS_MONITOREDCOPY_H

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryMonitorSvc.h"

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <functional>
#include <memory>

namespace AthDevice {

/// @c vecmem::copy object reporting about its operations
class MonitoredCopy : public vecmem::copy {

 public:
  /// Constructor
  MonitoredCopy(std::shared_ptr<const vecmem::copy> upstream,
                IMemoryMonitorSvc& svc, const std::string& component,
                const std::string& flavor);
  /// Destructor
  virtual ~MonitoredCopy();

 private:
  /// @name Function(s) inherited from @c vecmem::copy
  /// @{

  /// Perform a "low level" memory copy
  virtual void do_copy(std::size_t size, const void* from, void* to,
                       type::copy_type cptype) const override;
  /// Perform a "low level" memory filling operation
  virtual void do_memset(std::size_t size, void* ptr, int value) const override;

  /// @}

  /// @name Variables used to monitor the memory allocations/deallocations
  /// @{

  /// The upstream copy object to which the copy operations are forwarded
  std::shared_ptr<const vecmem::copy> m_upstream;
  /// The service used to monitor the memory allocations/deallocations
  std::reference_wrapper<IMemoryMonitorSvc> m_svc;
  /// The name of the component that is using this memory resource
  std::string m_component;
  /// The flavor/type of the copy object
  std::string m_flavor;

  /// @}

};  // class MonitoredCopy

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MONITOREDCOPY_H
