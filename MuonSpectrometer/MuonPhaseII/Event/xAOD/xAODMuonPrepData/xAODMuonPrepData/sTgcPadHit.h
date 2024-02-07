/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCPADHIT_H
#define XAODMUONPREPDATA_STGCPADHIT_H

#include "xAODMuonPrepData/versions/sTgcPadHit_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcStrip
   typedef sTgcPadHit_v1 sTgcPadHit;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcPadHit , 28752257 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
