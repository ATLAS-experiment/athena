/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MUONMEASUREMENT_H
#define XAODMUONPREPDATA_MUONMEASUREMENT_H

#include "xAODMuonPrepData/MuonMeasurementFwd.h"
#include "xAODMuonPrepData/versions/MuonMeasurement_v1.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "AthContainers/DataVector.h"
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"

DATAVECTOR_BASE(xAOD::MuonMeasurement_v1, xAOD::UncalibratedMeasurement_v1);

CLASS_DEF( xAOD::MuonMeasurement , 187147070 , 1 )


#endif