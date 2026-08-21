/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file HGTD_ConditionsAlgorithms/HGTD_AlignCondAlg.h
 * @author Fatima Bendebba
 * @date June, 2026
 * @brief Conditions algorithm producing the HGTD GeoAlignmentStore.
 */

#ifndef HGTD_CONDITIONSALGORITHMS_HGTDALIGNCONDALG_H
#define HGTD_CONDITIONSALGORITHMS_HGTDALIGNCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "DetDescrConditions/AlignableTransformContainer.h"

class HGTD_DetectorManager;

class HGTD_AlignCondAlg : public AthCondAlgorithm {
public:
  HGTD_AlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~HGTD_AlignCondAlg() override = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  SG::WriteCondHandleKey<GeoAlignmentStore> m_writeKey{
      this, "WriteKey", "HGTDAlignmentStore", "HGTD alignment store"};

  StringProperty m_detManagerName{
      this, "DetManagerName", "HGTD", "Name of the DetectorManager to retrieve"};

  const HGTD_DetectorManager* m_detManager{nullptr};

  SG::ReadCondHandleKey<AlignableTransformContainer> m_readKey{
      this,
      "ReadKey",
      "/HGTD/Align",
      "HGTD alignment folder"
  };

};

#endif
