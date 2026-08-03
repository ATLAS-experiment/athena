//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_ICOPIESTOOL_H
#define ATHDEVICEINTERFACES_ICOPIESTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <memory>

namespace AthDevice {

/// Interface for a tool that provides host and device copy objects
///
/// A given @c vecmem::copy object is always for one specific type of copy.
/// It can be for performing host<->device copies to a specific type of device,
/// in synchronous or asynchronous ways. Or it can just be for host<->host
/// copies.
///
/// Components performing copies between the host and the device can generally
/// be configured in 2 ways:
///  - To use a separate host and device memory resource, and perform explicit
///    copies between allocations from the two.
///  - To use a single memory resource that is accessible from both the host
///    and the device. Skipping an explicit host<->device copy.
///
/// In both of those cases, the component generally needs access to both a
/// "host" and a "device" copy object. It needs a "host" version in both cases
/// to properly set up / handle buffers allocated in host accessible memory.
///
/// Long story short, components that need to perform host<->device copies, need
/// to make use of this interface.
///
class ICopiesTool : virtual public IAlgTool {

 public:
  /// Declare the interface that the tool will implement
  DeclareInterfaceID(ICopiesTool, 1, 0);

  /// Destructor
  virtual ~ICopiesTool() = default;

  /// Get the "host" copy object
  virtual std::shared_ptr<const vecmem::copy> hostCopy(
      const EventContext& ctx) const = 0;

  /// Get the "device" copy object
  virtual std::shared_ptr<const vecmem::copy> deviceCopy(
      const EventContext& ctx) const = 0;

};  // class ICopiesTool

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_ICOPIESTOOL_H
