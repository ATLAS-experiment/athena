// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHCUDAINTERFACES_ISTREAMTOOL_H
#define ATHCUDAINTERFACES_ISTREAMTOOL_H

// Local include(s).
#include "AthCUDAInterfaces/IStreamProvider.h"

// Gaudi include(s).
#include "GaudiKernel/IAlgTool.h"

namespace AthCUDA {

/// Interface for tools providing CUDA streams to (reentrant) algorithms
class IStreamTool : public virtual IAlgTool, public virtual IStreamProvider {

 public:
  /// Declare the interface ID
  DeclareInterfaceID(AthCUDA::IStreamTool, 1, 0);

};  // class IStreamTool

}  // namespace AthCUDA

#endif  // ATHCUDAINTERFACES_ISTREAMTOOL_H
