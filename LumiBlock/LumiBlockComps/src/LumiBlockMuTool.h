/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LumiBlockMuTool
   @brief Tool to provide interactions per crossing (mu) from reading values stored in DB
   @brief Implementing ILumiBlockMuTool interface
   @author E.Torrence
**/

#ifndef LUMIBLOCKCOMPS_LumiBlockMuTool_H
#define LUMIBLOCKCOMPS_LumiBlockMuTool_H

#include "LumiBlockComps/ILumiBlockMuTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include <string>

// Usually this will be called to retrieve the mu value directly from EventInfo
// For data which doesn't have this (or to write it there in the first place)
// this can be configured to read the mu value from COOL instead.
//
class LumiBlockMuTool: public extends<AthAlgTool, ILumiBlockMuTool> {

 public:
  // Use base class constructor
  using base_class::base_class;

  // ---------------- user interface -------------------

  // Return interactions per crossing (mu) averaged over all BCIDs in physics bunch group
  virtual float averageInteractionsPerCrossing(const EventContext& ctx) const override final;

  // Return interactions per crossing (mu) for this specific BCID
  virtual float actualInteractionsPerCrossing(const EventContext& ctx) const override final;

  // Functions
  virtual StatusCode initialize() override;

 private:
  SG::ReadHandleKey<xAOD::EventInfo>       m_eventInfoKey{this
      ,"EventInfoKey"
      ,"EventInfo"
      ,"RHK for EventInfo"};

  SG::ReadDecorHandleKey<xAOD::EventInfo>  m_rdhkActMu {this
      ,"actualInteractionsPerCrossingKey"
      ,m_eventInfoKey, "actualInteractionsPerCrossing"
      ,"Decoration for Actual Interaction Per Crossing"};

  SG::ReadDecorHandleKey<xAOD::EventInfo>  m_rdhkAveMu {this
      ,"averageInteractionsPerCrossingKey"
      ,m_eventInfoKey, "averageInteractionsPerCrossing"
      ,"Decoration for Average Interaction Per Crossing"};
};


#endif
