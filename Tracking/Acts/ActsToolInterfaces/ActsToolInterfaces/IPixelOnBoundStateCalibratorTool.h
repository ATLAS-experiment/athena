/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTOOLINTERFACES_IPIXELONBOUNDSTATECALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IPIXELONBOUNDSTATECALIBRATORTOOL_H

#include "IOnBoundStateCalibratorTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

namespace ActsTrk {
   using PixelOnBoundStateCalibratorBase = OnBoundStateCalibratorBase<xAOD::PixelCluster,2>;

   class IPixelOnBoundStateCalibratorTool : public IOnBoundStateCalibratorTool<xAOD::PixelCluster,2> {
   public:
      DeclareInterfaceID(IPixelOnBoundStateCalibratorTool,1,0);
   };

   namespace traits {
      template <>
      struct Calibrator<xAOD::PixelCluster,2> {
         using ToolInterface = IPixelOnBoundStateCalibratorTool;
      };
   }
}

#endif
