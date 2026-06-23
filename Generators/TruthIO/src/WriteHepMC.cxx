/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthIO/WriteHepMC.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/WriterAsciiHepMC2.h"
#include "HepMC3/ReaderFactory.h"
// Additional includes for dealing with event numbers
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

WriteHepMC::WriteHepMC(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
{
  declareProperty("OutputFile", m_outfile="events.hepmc");
  declareProperty("Precision", m_precision=8);
  declareProperty("Format", m_format="hepmc3");
  declareProperty("Units", m_units="MEVMM");
}


StatusCode WriteHepMC::initialize() {
  CHECK(GenBase::initialize());
  if (m_units.size() != 5) {
     return StatusCode::FAILURE;
  }
  m_momentumunit = HepMC3::Units::momentum_unit(m_units.substr(0,3));
  m_lengthunit = HepMC3::Units::length_unit(m_units.substr(3,2));
  if (m_format == "hepmc2" || m_format == "ascii") {
    auto writer = std::make_shared<HepMC3::WriterAsciiHepMC2>(m_outfile);
    writer->set_precision(m_precision);
    m_hepmcio = writer;
    
  }
  if (m_format == "hepmc3"  || m_format == "asciiv3") {
    auto writer = std::make_shared<HepMC3::WriterAscii>(m_outfile);
    writer->set_precision(m_precision);
    m_hepmcio = writer;
  }
  return StatusCode::SUCCESS;
}


StatusCode WriteHepMC::execute(const EventContext& ctx) {
  // Just write out the first (i.e. signal) event in the collection
  HepMC3::GenEvent ev (*event_const(ctx));
  ev.set_units(m_momentumunit,m_lengthunit);
  if (ev.event_number()==1){
    // Get the event number. Full fall back - just set it to 1.
    int event_number = 1;
    // First attempt: xAOD::EventInfo (new-style EVNT)
    SG::ReadHandle<xAOD::EventInfo> mc_ei{"McEventInfo", ctx};
    if (!mc_ei.isValid()){
      // Second attempt: old-style EventInfo
      SG::ReadHandle<EventInfo> og_mc_ei{"McEventInfo", ctx};
      if (!og_mc_ei.isValid()){
        // Give up and let people know that we fell through
        ATH_MSG_WARNING("No McEventInfo found in SG - no event numbers available");
      } else {
        // Second one hit - get the event number from the OG event info
        event_number = og_mc_ei->event_ID()->event_number();
      }
    } else {
      // First one hit - get the event number from the xAOD event info
      event_number = mc_ei->eventNumber();
    }
    ev.set_event_number(event_number);
  }
  m_hepmcio->write_event(ev);
  return StatusCode::SUCCESS;
}
