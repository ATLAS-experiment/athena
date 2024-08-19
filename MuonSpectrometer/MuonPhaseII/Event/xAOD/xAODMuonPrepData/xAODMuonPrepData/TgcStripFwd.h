/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_TgcStripFWD_H
#define XAODMUONPREPDATA_TgcStripFWD_H
#include "AthContainers/DataVector.h"

/** @brief Forward declaration of the xAOD::TgcStrip */
namespace xAOD{
   class UncalibratedMeasurement_v1;
   class TgcStrip_v1;
   using TgcStrip = TgcStrip_v1;

   class TgcStripAuxContainer_v1;
   using TgcStripAuxContainer = TgcStripAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::TgcStrip_v1, xAOD::UncalibratedMeasurement_v1);

namespace xAOD{
   using TgcStripContainer_v1 = DataVector<TgcStrip_v1>;
   using TgcStripContainer = TgcStripContainer_v1;
}
#endif
