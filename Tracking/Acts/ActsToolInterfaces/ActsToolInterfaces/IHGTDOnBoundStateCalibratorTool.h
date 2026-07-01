/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTOOLINTERFACES_IHGTDONBOUNDSTATECALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IHGTDONBOUNDSTATECALIBRATORTOOL_H

#include "IOnBoundStateCalibratorTool.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

namespace ActsTrk {
   using HGTDOnBoundStateCalibratorBase = OnBoundStateCalibratorBase<xAOD::HGTDCluster,3>;

   class IHGTDOnBoundStateCalibratorTool : public IOnBoundStateCalibratorTool<xAOD::HGTDCluster,3> {
   public:
      DeclareInterfaceID(IHGTDOnBoundStateCalibratorTool,1,0);
   };
   namespace traits {
      template <>
      struct Calibrator<xAOD::HGTDCluster,3> {
         using ToolInterface = IHGTDOnBoundStateCalibratorTool;
      };
   }
}

#endif
