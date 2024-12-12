/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorEventInfo.h"

namespace GeneratorPhysVal {
GeneratorEventInfo::GeneratorEventInfo(PlotBase* pParent, const std::string& sDir,
                                       const std::string& sType)
    : PlotBase(pParent, sDir), m_sType(sType) {

  m_event_number = Book1D("is_event_number", "is_event_number", 2, -0.5, 1.5);
  m_mc_ChannelNumber =
      Book1D("is_mc_ChannelNumber", "is_mc_ChannelNumber", 2, -0.5, 1.5);
}
void GeneratorEventInfo::check_eventNumber(SG::ReadHandle<xAOD::EventInfo> evt) {
  const auto eventNumber = evt->eventNumber();
  m_event_number->Fill(eventNumber > 0 ? 1.0 : 0.0);
}

int GeneratorEventInfo::check_mcChannelNumber(SG::ReadHandle<xAOD::EventInfo> evt,
                                              int ref) {
  int mcChannelNumber = evt->mcChannelNumber();
  if (ref == 0) {
    ref = mcChannelNumber;
    m_mc_ChannelNumber->Fill(1.);
  } else {
    m_mc_ChannelNumber->Fill(mcChannelNumber == ref ? 1.0 : 0.0);
  }
  return ref;
}
}  // namespace GeneratorPhysVal
