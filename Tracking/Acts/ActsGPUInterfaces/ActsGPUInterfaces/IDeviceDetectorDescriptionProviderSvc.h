/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUINTERFACES_IDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H
#define ACTSGPUINTERFACES_IDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H

// Framework include(s).
#include "GaudiKernel/IService.h"
#include "Identifier/Identifier.h"

// System include(s).
#include <cstdint>
#include <unordered_map>

namespace ActsTrk {

/// Interface for services providing an Acts device/GPU detector description.
class IDeviceDetectorDescriptionProviderSvc : virtual public IService {
 public:
  /// Declare the interface ID for this service.
  DeclareInterfaceID(IDeviceDetectorDescriptionProviderSvc, 1, 0);

  /// Get a LUT from Detray to Athena identifiers.
  virtual const std::unordered_map<uint64_t, Identifier>& detrayToAthenaMap()
      const = 0;
  /// Get a LUT from Athena to Detray identifiers.
  virtual const std::unordered_map<Identifier, uint64_t>& athenaToDetrayMap()
      const = 0;

};  // class IDeviceDetectorDescriptionProviderSvc

}  // namespace ActsTrk

#endif  // ACTSGPUINTERFACES_IDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H
