/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_COMBINEDMUONSTRIP_H
#define XAODMUONPREPDATA_COMBINEDMUONSTRIP_H

#include "xAODMuonPrepData/CombinedMuonStripFwd.h"
#include "xAODMuonPrepData/versions/CombinedMuonStrip_v1.h"
#include "xAODCore/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(xAOD::CombinedMuonStrip_v1, xAOD::UncalibratedMeasurement_v1);
// Set up a CLID for the class:

CLASS_DEF( xAOD::CombinedMuonStrip , 201321965 , 1 );
#endif  