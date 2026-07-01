/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_TGCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_TGCSTRIPCONTAINER_H


#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"

namespace xAOD{
   using TgcStripContainer_v1 = DataVector<TgcStrip_v1>;
   using TgcStripContainer = TgcStripContainer_v1;
}
// Set up a CLID for the class:

CLASS_DEF( xAOD::TgcStripContainer , 1245357318 , 1 )

#endif