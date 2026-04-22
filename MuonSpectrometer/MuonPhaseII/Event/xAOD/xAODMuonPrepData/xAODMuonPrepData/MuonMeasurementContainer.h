/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MUONMEASUREMENTCONTAINER_H
#define XAODMUONPREPDATA_MUONMEASUREMENTCONTAINER_H

#include "xAODMuonPrepData/MuonMeasurement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace xAOD{
   using MuonMeasurementContainer_v1 = DataVector<MuonMeasurement_v1>;
   using MuonMeasurementContainer = MuonMeasurementContainer_v1;
}

CLASS_DEF( xAOD::MuonMeasurementContainer , 1221753506 , 1 )
#endif