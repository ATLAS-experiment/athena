/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthIO/WriteHepMC.h"
#ifdef HEPMC3
#include "HepMC3/WriterAscii.h"
#endif
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
  declareProperty("Format", m_format="hepmc2");
  declareProperty("Units", m_units="MEVMM");
}


StatusCode WriteHepMC::initialize() {
  CHECK(GenBase::initialize());
  if (m_units.size() != 5) {
     return StatusCode::FAILURE;
  }
#ifdef HEPMC3
  m_momentumunit = HepMC3::Units::momentum_unit(m_units.substr(0,3));
  m_lengthunit = HepMC3::Units::length_unit(m_units.substr(3,2));
  if (m_format == "hepmc2") {
    auto writer = new HepMC3::WriterAsciiHepMC2(m_outfile);
    writer->set_precision(m_precision);
    m_hepmcio.reset(writer);
  }
  if (m_format == "hepmc3") {
    auto writer = new HepMC3::WriterAscii(m_outfile);
    writer->set_precision(m_precision);
    m_hepmcio.reset(writer);
  }
#else
  m_momentumunit = (m_units.substr(0,3) == "MEV") ? HepMC::Units::MEV : HepMC::Units::GEV;
  m_lengthunit = (m_units.substr(3,2) == "CM") ? HepMC::Units::CM : HepMC::Units::MM;
  m_hepmcio.reset( new HepMC::IO_GenEvent(m_outfile) );
  m_hepmcio->precision(m_precision);
#endif
  return StatusCode::SUCCESS;
}


StatusCode WriteHepMC::execute() {
  // Just write out the first (i.e. signal) event in the collection
#ifdef HEPMC3
  HepMC3::GenEvent ev (*event_const());
  ev.set_units(m_momentumunit,m_lengthunit);
  if (ev.event_number()==1){
    // Get the event number. Full fall back - just set it to 1.
    int event_number = 1;
    // Grab the contact for the current thread so we can use ReadHandles nicely
    const EventContext& context = Gaudi::Hive::currentContext();
    // First attempt: xAOD::EventInfo (new-style EVNT)
    SG::ReadHandle<xAOD::EventInfo> mc_ei{"McEventInfo", context};
    if (!mc_ei.isValid()){
      // Second attempt: old-style EventInfo
      SG::ReadHandle<EventInfo> og_mc_ei{"McEventInfo", context};
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
#else
  HepMC::GenEvent ev (*event_const());
  ev.use_units(m_momentumunit,m_lengthunit);
  if (ev.event_number()==1){
    // Get the event number. Full fall back - just set it to 1.
    int event_number = 1;
    // Grab the contact for the current thread so we can use ReadHandles nicely
    const EventContext& context = Gaudi::Hive::currentContext();
    // First attempt: xAOD::EventInfo (new-style EVNT)
    SG::ReadHandle<xAOD::EventInfo> mc_ei{"McEventInfo", context};
    if (!mc_ei.isValid()){
      // Second attempt: old-style EventInfo
      SG::ReadHandle<EventInfo> og_mc_ei{"McEventInfo", context};
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
  m_hepmcio->write_event(&ev);
#endif
  return StatusCode::SUCCESS;
}
