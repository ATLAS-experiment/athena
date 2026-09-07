// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHHIPINTERFACES_ISTREAMTOOL_H
#define ATHHIPINTERFACES_ISTREAMTOOL_H

// Local include(s).
#include "AthHIPInterfaces/IStreamProvider.h"

// Gaudi include(s).
#include "GaudiKernel/IAlgTool.h"

namespace AthHIP {

/// Interface for tools providing HIP streams to (reentrant) algorithms
class IStreamTool : public virtual IAlgTool, public virtual IStreamProvider {

 public:
  /// Declare the interface ID
  DeclareInterfaceID(AthHIP::IStreamTool, 1, 0);

};  // class IStreamTool

}  // namespace AthHIP

#endif  // ATHHIPINTERFACES_ISTREAMTOOL_H
