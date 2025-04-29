/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_STRIPEMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H
#define INDET_STRIPEMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H

#include "EmulatedDefectsToDetectorElementStatusCondAlg.h"
#include "StripEmulatedDefects.h"
#include "SCT_ReadoutGeometry/SCT_DetectorElementStatus.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace InDet {

   class StripEmulatedDefectsToDetectorElementStatusCondAlg;
   namespace details {
      template <>
      struct EmulatedDefectsToDetectorElementStatusTraits<StripEmulatedDefectsToDetectorElementStatusCondAlg> {
         using T_ConcreteDetectorElementStatusType  = SCT_DetectorElementStatus;
         using T_EmulatedDefects  = StripEmulatedDefects;
         using T_ModuleHelper  = StripModuleHelper;
         using KEY_TYPE = StripEmulatedDefects::KEY_TYPE;
         // No chip defects
         static constexpr unsigned int CHIP_MASK_IDX = StripModuleHelper::N_MASKS;
      };
   }

   class StripEmulatedDefectsToDetectorElementStatusCondAlg
      : public EmulatedDefectsToDetectorElementStatusCondAlg<StripEmulatedDefectsToDetectorElementStatusCondAlg>
   {
   public:
      using EmulatedDefectsToDetectorElementStatusCondAlg<StripEmulatedDefectsToDetectorElementStatusCondAlg>::EmulatedDefectsToDetectorElementStatusCondAlg;
   };
}
#endif
