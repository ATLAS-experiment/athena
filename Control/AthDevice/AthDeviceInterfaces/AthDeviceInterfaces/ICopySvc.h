//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_ICOPYSVC_H
#define ATHDEVICEINTERFACES_ICOPYSVC_H

// Framework include(s).
#include "GaudiKernel/IService.h"

// Local include(s).
#include "AthDeviceInterfaces/ICopyProvider.h"

namespace AthDevice {

/// Interface for a service that provides a "copy object"
class ICopySvc : virtual public IService, virtual public ICopyProvider {

 public:
  /// Declare the interface that the service will implement
  DeclareInterfaceID(ICopySvc, 1, 0);

  /// Destructor
  virtual ~ICopySvc() = default;

};  // class ICopySvc

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_ICOPYSVC_H
