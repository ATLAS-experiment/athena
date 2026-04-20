/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_STGCMEASCONTAINER_H
#define XAODMUONPREPDATA_STGCMEASCONTAINER_H


#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"
namespace xAOD{    
   using sTgcMeasContainer_v1 = DataVector<sTgcMeasurement_v1>;
   using sTgcMeasContainer = sTgcMeasContainer_v1;
}
// Set up a CLID for the class:

CLASS_DEF( xAOD::sTgcMeasContainer , 1285126101 , 1 )
#endif