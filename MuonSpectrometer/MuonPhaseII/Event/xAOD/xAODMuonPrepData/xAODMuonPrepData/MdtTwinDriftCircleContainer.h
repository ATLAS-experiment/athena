/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MDTTWINDRIFTCIRCLECONTAINER_H
#define XAODMUONPREPDATA_MDTTWINDRIFTCIRCLECONTAINER_H

#include "xAODMuonPrepData/MdtTwinDriftCircleFwd.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"


namespace xAOD{
   using MdtTwinDriftCircleContainer_v1 = DataVector<MdtTwinDriftCircle_v1>;
   using MdtTwinDriftCircleContainer = MdtTwinDriftCircleContainer_v1;
}
// Set up a CLID for the class:
CLASS_DEF( xAOD::MdtTwinDriftCircleContainer , 1134789784 , 1 )
#endif