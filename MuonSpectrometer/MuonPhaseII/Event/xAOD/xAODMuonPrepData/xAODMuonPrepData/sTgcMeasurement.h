/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCMEASUREMENT_H
#define XAODMUONPREPDATA_STGCMEASUREMENT_H

#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcMeasurement
   typedef sTgcMeasurement_v1 sTgcMeasurement;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcMeasurement , 72401404 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
