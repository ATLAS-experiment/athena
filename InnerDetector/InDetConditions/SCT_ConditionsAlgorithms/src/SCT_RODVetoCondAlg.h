// -*- C++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_RODVetoCondAlg.h
 *
 * @brief Header file for the SCT_RODVetoCondAlg class 
 *  in package SCT_ConditionsAlgorithms
 *
 * @author Susumu Oda
 **/

#ifndef SCT_CONDITIONSALGORITHMS_RODVETOCONDALG_H
#define SCT_CONDITIONSALGORITHMS_RODVETOCONDALG_H 

// Athena
#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "SCT_Cabling/ISCT_CablingTool.h"
#include "SCT_ConditionsData/IdentifierSet.h"
#include "StoreGate/WriteCondHandleKey.h"

// STL
#include <string>
#include <vector>

// Forward declarations
class SCT_ID;

/// Algorithm needs to show calling the SCT_RODVeto to exclude bad components
class SCT_RODVetoCondAlg : public AthCondAlgorithm {
 public:
  SCT_RODVetoCondAlg(const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~SCT_RODVetoCondAlg() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

 private:
  ToolHandle<ISCT_CablingTool> m_cabling{this, "SCT_CablingTool", "SCT_CablingTool", "Tool to retrieve SCT Cabling"};
  const SCT_ID* m_pHelper{nullptr};
  SG::WriteCondHandleKey<IdentifierSet> m_badIds{this, "BadModuleIds", "BadSCTModuleIds_RODVeto", "Write key for bad module identifiers"};
  UnsignedIntegerArrayProperty m_badRODElementsInput{this, "BadRODIds", {}, "Input list of RODs to be vetoed"};
}; //end of class

#endif // SCT_RODVetoCondAlg_H
