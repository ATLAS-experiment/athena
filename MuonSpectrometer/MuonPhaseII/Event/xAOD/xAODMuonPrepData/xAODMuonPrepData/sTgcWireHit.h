/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCWIREHIT_H
#define XAODMUONPREPDATA_STGCWIREHIT_H

#include "xAODMuonPrepData/versions/sTgcWireHit_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcStrip
   typedef sTgcWireHit_v1 sTgcWireHit;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcWireHit , 73038134 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
