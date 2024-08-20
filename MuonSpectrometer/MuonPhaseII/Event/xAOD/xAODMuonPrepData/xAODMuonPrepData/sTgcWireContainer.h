/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCWIRECONTAINER_H
#define XAODMUONPREPDATA_STGCWIRECONTAINER_H

#include "xAODMuonPrepData/sTgcWireHitFwd.h"
#include "xAODMuonPrepData/sTgcWireHit.h"
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"

namespace xAOD{
   using sTgcWireContainer_v1 = DataVector<sTgcWireHit_v1>;
   using sTgcWireContainer = sTgcWireContainer_v1;
}
CLASS_DEF( xAOD::sTgcWireContainer , 1281983002 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
