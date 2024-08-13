/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTTWINDRIFTCIRCLEAUXCONTAINER_H
#define XAODMUONPREPDATA_MDTTWINDRIFTCIRCLEAUXCONTAINER_H

#include "xAODMuonPrepData/versions/MdtTwinDriftCircleAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MdtDriftCircle
typedef MdtTwinDriftCircleAuxContainer_v1 MdtTwinDriftCircleAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MdtTwinDriftCircleAuxContainer , 1307363727 , 1 )
#endif 
