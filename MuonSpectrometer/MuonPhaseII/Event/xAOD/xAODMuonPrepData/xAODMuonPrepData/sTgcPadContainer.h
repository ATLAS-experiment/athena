/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_STGCPADCONTAINER_H
#define XAODMUONPREPDATA_STGCPADCONTAINER_H

#include "xAODMuonPrepData/sTgcPadHitFwd.h"
#include "xAODMuonPrepData/sTgcPadHit.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"
// Set up a CLID for the class:

namespace xAOD{
   using sTgcPadContainer_v1 = DataVector<sTgcPadHit_v1>;
   using sTgcPadContainer = sTgcPadContainer_v1;
}

CLASS_DEF( xAOD::sTgcPadContainer , 1310723933 , 1 )
#endif