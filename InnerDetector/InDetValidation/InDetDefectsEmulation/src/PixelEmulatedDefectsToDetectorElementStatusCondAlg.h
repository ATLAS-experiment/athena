/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_PIXELEMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H
#define INDET_PIXELEMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALG_H

#include "EmulatedDefectsToDetectorElementStatusCondAlg.h"
#include "PixelEmulatedDefects.h"
#include "PixelReadoutGeometry/PixelDetectorElementStatus.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace InDet {

   class PixelEmulatedDefectsToDetectorElementStatusCondAlg;
   namespace details {
      template <>
      struct EmulatedDefectsToDetectorElementStatusTraits<PixelEmulatedDefectsToDetectorElementStatusCondAlg> {
         using T_ConcreteDetectorElementStatusType  = PixelDetectorElementStatus;
         using T_EmulatedDefects  = PixelEmulatedDefects;
         using T_ModuleHelper  = PixelModuleHelper;
         using KEY_TYPE = PixelEmulatedDefects::KEY_TYPE;
         static constexpr unsigned int CHIP_MASK_IDX = 2;
      };
   }

   class PixelEmulatedDefectsToDetectorElementStatusCondAlg
      : public EmulatedDefectsToDetectorElementStatusCondAlg<PixelEmulatedDefectsToDetectorElementStatusCondAlg>
   {
   public:
      using EmulatedDefectsToDetectorElementStatusCondAlg<PixelEmulatedDefectsToDetectorElementStatusCondAlg>::EmulatedDefectsToDetectorElementStatusCondAlg;

   };
}
#endif
