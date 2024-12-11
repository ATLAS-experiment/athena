/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeneratorPhysValEventInfo_GeneratorEventInfo_H
#define GeneratorPhysValEventInfo_GeneratorEventInfo_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandle.h"

namespace GeneratorPhysVal
{
  class GeneratorEventInfo : public PlotBase
  {
  public:

    GeneratorEventInfo(PlotBase* pParent, const std::string& sDir, const std::string& sType = "");

    void check_eventNumber(SG::ReadHandle<xAOD::EventInfo> evt);
    int check_mcChannelNumber(SG::ReadHandle<xAOD::EventInfo> evt, int ref);

    TH1* m_event_number = nullptr;
    TH1* m_mc_ChannelNumber = nullptr;
    
  private:
    std::string m_sType = "";
  };
}

#endif
