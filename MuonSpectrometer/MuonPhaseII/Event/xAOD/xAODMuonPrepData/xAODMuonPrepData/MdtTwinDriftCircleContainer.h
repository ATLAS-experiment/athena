/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MDTTWINDRIFTCIRCLECONTAINER_H
#define XAODMUONPREPDATA_MDTTWINDRIFTCIRCLECONTAINER_H

#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/versions/MdtTwinDriftCircleContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the MdtTwinDriftCirleContainer
typedef MdtTwinDriftCircleContainer_v1 MdtTwinDriftCircleContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MdtTwinDriftCircleContainer , 1134789784 , 1 )
#endif