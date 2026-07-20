//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_IMEMORYRESOURCESTOOL_H
#define ATHDEVICEINTERFACES_IMEMORYRESOURCESTOOL_H

// Framework include(s).
#include "GaudiKernel/IAlgTool.h"

// System include(s).
#include <memory_resource>

namespace AthDevice {

/// Interface for a tool that provides a main/device and a host memory resource
///
/// In many situations, when a components needs to interact with a "device",
/// it needs to manage memory both on the device and on the host. This interface
/// is meant to simplify the setup for such components.
///
/// The API follows the logic that was adopted by Acts/traccc. That when a
/// piece of code needs both a "device"/"main" and a "host" memory resource, the
/// "device"/"main" one would absolutely need to exist. But the "host" one is
/// technically optional. Such that if there is no "host" memory resource
/// set up, that means that the "device"/"main" memory resource is one that's
/// accessible both from the host and the device. With no explicit copies
/// necessary bbetween the two.
///
class IMemoryResourcesTool : virtual public IAlgTool {

 public:
  /// Declare the interface that the tool will implement
  DeclareInterfaceID(IMemoryResourcesTool, 1, 0);

  /// Destructor
  virtual ~IMemoryResourcesTool() = default;

  /// Get the "main" / "device" @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mainMR() const = 0;

  /// Get the "host" (accessible) @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource* hostMR() const = 0;

};  // class IMemoryResourcesTool

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_IMEMORYRESOURCESTOOL_H
