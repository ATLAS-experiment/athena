/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTOOLINTERFACES_ISTRIPONBOUNDSTATECALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPONBOUNDSTATECALIBRATORTOOL_H

#include "IOnBoundStateCalibratorTool.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

namespace ActsTrk {
   using StripOnBoundStateCalibratorBase = OnBoundStateCalibratorBase<xAOD::StripCluster,1>;

   class IStripOnBoundStateCalibratorTool : public IOnBoundStateCalibratorTool<xAOD::StripCluster,1> {
   public:
      DeclareInterfaceID(IStripOnBoundStateCalibratorTool,1,0);
   };

   namespace traits {
      template <>
      struct Calibrator<xAOD::StripCluster,1> {
         using ToolInterface = IStripOnBoundStateCalibratorTool;
      };
   }
}

#endif
