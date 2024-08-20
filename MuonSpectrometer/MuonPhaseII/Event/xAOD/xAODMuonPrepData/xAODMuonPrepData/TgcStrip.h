/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_TGCSTRIP_H
#define XAODMUONPREPDATA_TGCSTRIP_H

#include "xAODMuonPrepData/TgcStripFwd.h"
#include "xAODMuonPrepData/versions/TgcStrip_v1.h"
#include "AthContainers/DataVector.h"
#include "xAODCore/CLASS_DEF.h"
DATAVECTOR_BASE(xAOD::TgcStrip_v1, xAOD::UncalibratedMeasurement_v1);

// Set up a CLID for the class:
CLASS_DEF(xAOD::TgcStrip, 46677082, 1)

#endif  // XAODMUONPREPDATA_TGCSTRIP_H
