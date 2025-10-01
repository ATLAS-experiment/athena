/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthIO/WriteHepMC.h"
#ifdef HEPMC3
#include "HepMC3/WriterAscii.h"
#endif

WriteHepMC::WriteHepMC(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
{
  declareProperty("OutputFile", m_outfile="events.hepmc");
  declareProperty("Precision", m_precision=8);
  declareProperty("Format", m_format="hepmc2");
  declareProperty("Units", m_units="GEVMM");
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
  m_hepmcio->write_event(ev);
#else
  HepMC::GenEvent ev (*event_const());
  ev.use_units(m_momentumunit,m_lengthunit);
  m_hepmcio->write_event(&ev);
#endif
  return StatusCode::SUCCESS;
}
