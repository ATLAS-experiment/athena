// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHCUDAINTERFACES_ISTREAMSVC_H
#define ATHCUDAINTERFACES_ISTREAMSVC_H

// Local include(s).
#include "AthCUDAInterfaces/IStreamProvider.h"

// Gaudi include(s).
#include "GaudiKernel/IService.h"

namespace AthCUDA {

/// Interface for services providing CUDA streams to (reentrant) algorithms
class IStreamSvc : public virtual IService, public virtual IStreamProvider {

 public:
  /// Declare the interface ID
  DeclareInterfaceID(AthCUDA::IStreamSvc, 1, 0);

};  // class IStreamSvc

}  // namespace AthCUDA

#endif  // ATHCUDAINTERFACES_ISTREAMSVC_H
