/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_STGCMEASCONTAINER_H
#define XAODMUONPREPDATA_STGCMEASCONTAINER_H

#include "xAODMuonPrepData/sTgcMeasurment.h"
#include "xAODMuonPrepData/versions/sTgcMeasContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Define the version of the pixel cluster container
   typedef sTgcMeasContainer_v1 sTgcMeasContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcMeasContainer , 1285126101 , 1 )
#endif