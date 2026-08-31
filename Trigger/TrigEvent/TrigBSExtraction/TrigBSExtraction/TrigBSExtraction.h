/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBSEXTRACTION_TRIGBSEXTRACTION_H
#define TRIGBSEXTRACTION_TRIGBSEXTRACTION_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include "TrigNavigation/Navigation.h"
#include "TrigSteeringEvent/HLTResult.h"


/**
 * @brief Top algorithms which unpacks objects from BS and places them in SG.
 */
class TrigBSExtraction : public AthAlgorithm {
public:
  TrigBSExtraction(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

private:
  /**
   @brief method which does loop over objects
   @param  ctx       EventContext
   @param  navTool   navigation tool
   @param  key       Input key of the HLTResult
   @param  keyOu     Output key of the HLTResult
   */
  StatusCode repackFeaturesToSG (const EventContext& ctx,
                                 HLT::Navigation& navTool,
                                 const SG::ReadHandleKey<HLT::HLTResult>& key,
                                 SG::WriteHandleKey<HLT::HLTResult>& keyOut);

  ToolHandle<HLT::Navigation> m_navTool{this, "Navigation", "HLT::Navigation/Navigation",
                                        "Navigation tool for HLT result"};

  SG::ReadHandleKey<HLT::HLTResult> m_hltResultKeyIn{
    this, "HLTResultKeyIn", "HLTResult_HLT_BS", "Input key for HLT result"};
  SG::WriteHandleKey<HLT::HLTResult> m_hltResultKeyOut{
    this, "HLTResultKeyOut", "HLTResult_HLT", "Output key for HLT result"};

  SG::ReadHandleKeyArray<HLT::HLTResult> m_dataScoutingKeysIn{
    this, "DSResultKeysIn", {}, "Input keys for DataScouting HLT results"};
  SG::WriteHandleKeyArray<HLT::HLTResult> m_dataScoutingKeysOut{
    this, "DSResultKeysOut", {}, "Output keys for DataScouting HLT results"};
};


#endif // TRIGBSEXTRACTION_TRIGBSEXTRACTION_H
