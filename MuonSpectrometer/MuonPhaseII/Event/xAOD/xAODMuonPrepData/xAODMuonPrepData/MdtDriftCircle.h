/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTMDRIFTCIRCLE_H
#define XAODMUONPREPDATA_MDTMDRIFTCIRCLE_H

#include "xAODMuonPrepData/MdtDriftCircleFwd.h"
#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"
#include "xAODCore/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(xAOD::MdtDriftCircle_v1, xAOD::UncalibratedMeasurement_v1);
// Set up a CLID for the class:

CLASS_DEF(xAOD::MdtDriftCircle, 92538182, 1)
#endif  