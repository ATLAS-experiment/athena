// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHHIPINTERFACES_ISTREAMSVC_H
#define ATHHIPINTERFACES_ISTREAMSVC_H

// Local include(s).
#include "AthHIPInterfaces/IStreamProvider.h"

// Gaudi include(s).
#include "GaudiKernel/IService.h"

namespace AthHIP {

/// Interface for services providing HIP streams to (reentrant) algorithms
class IStreamSvc : public virtual IService, public virtual IStreamProvider {

 public:
  /// Declare the interface ID
  DeclareInterfaceID(AthHIP::IStreamSvc, 1, 0);

};  // class IStreamSvc

}  // namespace AthHIP

#endif  // ATHHIPINTERFACES_ISTREAMSVC_H
