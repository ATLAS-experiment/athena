//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_ICOPYTOOL_H
#define ATHDEVICEINTERFACES_ICOPYTOOL_H

// Framework include(s).
#include "GaudiKernel/IAlgTool.h"

// Local include(s).
#include "AthDeviceInterfaces/ICopyProvider.h"

namespace AthDevice {

/// Interface for a tool that provides a "copy object"
class ICopyTool : virtual public IAlgTool, virtual public ICopyProvider {

 public:
  /// Declare the interface that the tool will implement
  DeclareInterfaceID(ICopyTool, 1, 0);

  /// Destructor
  virtual ~ICopyTool() = default;

};  // class ICopyTool

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_ICOPYTOOL_H
