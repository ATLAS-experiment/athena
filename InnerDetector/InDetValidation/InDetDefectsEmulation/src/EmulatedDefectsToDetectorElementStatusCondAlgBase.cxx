/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EmulatedDefectsToDetectorElementStatusCondAlgBase.h"

namespace InDet {
   EmulatedDefectsToDetectorElementStatusCondAlgBase::EmulatedDefectsToDetectorElementStatusCondAlgBase(const std::string& name, ISvcLocator* pSvcLocator)
      : ::AthReentrantAlgorithm(name, pSvcLocator)
   {
   }

   StatusCode EmulatedDefectsToDetectorElementStatusCondAlgBase::initialize()
   {
      ATH_CHECK( m_writeKey.initialize());
      return StatusCode::SUCCESS;
   }

   StatusCode EmulatedDefectsToDetectorElementStatusCondAlgBase::finalize()
   {
      return StatusCode::SUCCESS;
   }
}
