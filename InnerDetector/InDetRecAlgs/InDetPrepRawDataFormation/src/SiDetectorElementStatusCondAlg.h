// -*- C++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCONDITIONSALGORITHMS_SIDETECTORELEMENTINFOCONDALG_H
#define PIXELCONDITIONSALGORITHMS_SIDETECTORELEMENTINFOCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "StoreGate/WriteHandleKey.h"

#include "InDetConditionsSummaryService/IDetectorElementStatusTool.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"


namespace InDet {
   class SiDetectorElementStatusCondAlg : public AthCondAlgorithm
   {
   public:
      SiDetectorElementStatusCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~SiDetectorElementStatusCondAlg() override = default;

      virtual StatusCode initialize() override final;
      virtual StatusCode execute(const EventContext& ctx) const override final;
      virtual StatusCode finalize() override final;

   private:
      ToolHandle <IDetectorElementStatusTool> m_condSummaryTool
         {this, "ConditionsSummaryTool", "", "Tool to retrieve e.g. Pixel or SCT Conditions summary"};
      SG::WriteCondHandleKey<InDet::SiDetectorElementStatus> m_writeKey
         {this, "WriteKey", "", "Key of output SiDetectorElementStatus data"};
      mutable std::atomic<unsigned int> m_maxSize{};
   };
}
#endif // PIXELCONDITIONSALGORITHMS_SIDETECTORELEMENTINFOALG_H
