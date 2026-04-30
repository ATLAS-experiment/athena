//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_IMEMORYRESOURCESVC_H
#define ATHDEVICEINTERFACES_IMEMORYRESOURCESVC_H

// Framework include(s).
#include "GaudiKernel/IService.h"

// Local include(s).
#include "AthDeviceInterfaces/IMemoryResourceProvider.h"

namespace AthDevice {

/// Interface for a service that provides a "memory resource"
class IMemoryResourceSvc : virtual public IService,
                           virtual public IMemoryResourceProvider {

 public:
  /// Declare the interface that the service will implement
  DeclareInterfaceID(IMemoryResourceSvc, 1, 0);

  /// Destructor
  virtual ~IMemoryResourceSvc() = default;

};  // class IMemoryResourceSvc

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_IMEMORYRESOURCESVC_H
