/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_COMBINEDMUONSTRIPCONTAINER_H
#define XAODMUONPREPDATA_COMBINEDMUONSTRIPCONTAINER_H

#include "xAODMuonPrepData/CombinedMuonStrip.h"

#include "xAODCore/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

namespace xAOD{
   using CombinedMuonStripContainer_v1 = DataVector<CombinedMuonStrip_v1>;
   using CombinedMuonStripContainer = CombinedMuonStripContainer_v1;
}
// Set up a CLID for the class:
CLASS_DEF( xAOD::CombinedMuonStripContainer , 1312941631 , 1 )
#endif  