/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_TGCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_TGCSTRIPCONTAINER_H

#include "xAODMuonPrepData/TgcStripFwd.h"
#include "xAODMuonPrepData/TgcStrip.h"

namespace xAOD{
   using TgcStripContainer_v1 = DataVector<TgcStrip_v1>;
   using TgcStripContainer = TgcStripContainer_v1;
}
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::TgcStripContainer , 1245357318 , 1 )

#endif