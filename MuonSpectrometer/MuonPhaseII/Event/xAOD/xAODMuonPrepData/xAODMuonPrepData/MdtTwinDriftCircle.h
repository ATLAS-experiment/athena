/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTTWINMEASUREMENT_H
#define XAODMUONPREPDATA_MDTTWINMEASUREMENT_H

#include "xAODMuonPrepData/MdtTwinDriftCircleFwd.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/versions/MdtTwinDriftCircle_v1.h"
#include "xAODCore/CLASS_DEF.h"

DATAVECTOR_BASE(xAOD::MdtTwinDriftCircle_v1, xAOD::MdtDriftCircle_v1);
// Set up a CLID for the class:
CLASS_DEF( xAOD::MdtTwinDriftCircle , 71190612 , 1 )
#endif