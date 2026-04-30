//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_IMEMORYRESOURCETOOL_H
#define ATHDEVICEINTERFACES_IMEMORYRESOURCETOOL_H

// Framework include(s).
#include "GaudiKernel/IAlgTool.h"

// Local include(s).
#include "AthDeviceInterfaces/IMemoryResourceProvider.h"

namespace AthDevice {

/// Interface for a tool that provides a "memory resource"
class IMemoryResourceTool : virtual public IAlgTool,
                            virtual public IMemoryResourceProvider {

 public:
  /// Declare the interface that the tool will implement
  DeclareInterfaceID(IMemoryResourceTool, 1, 0);

  /// Destructor
  virtual ~IMemoryResourceTool() = default;

};  // class IMemoryResourceTool

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_IMEMORYRESOURCETOOL_H
