// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_EMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALGBASE_H
#define INDET_EMULATEDDEFECTSTODETECTORELEMENTSTATUSCONDALGBASE_H

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "StoreGate/WriteHandleKey.h"

#include "InDetConditionsSummaryService/IDetectorElementStatusTool.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"


namespace InDet {
   class EmulatedDefectsToDetectorElementStatusCondAlgBase : public AthCondAlgorithm
   {
   public:
      EmulatedDefectsToDetectorElementStatusCondAlgBase(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~EmulatedDefectsToDetectorElementStatusCondAlgBase() override = default;

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

   protected:
      SG::WriteCondHandleKey<InDet::SiDetectorElementStatus> m_writeKey
         {this, "WriteKey", "", "Key of output SiDetectorElementStatus data"};
   };
}
#endif // PIXELCONDITIONSALGORITHMS_SIDETECTORELEMENTINFOALG_H
