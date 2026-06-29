/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthIO/HepMCReadFromFile.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/HEPEVT_Wrapper.h"
#include "AtlasHepMC/GenCrossSection.h" 
#include "AtlasHepMC/ReaderFactory.h" 
#include "GaudiKernel/DataSvc.h"

#include "StoreGate/StoreGateSvc.h"


HepMCReadFromFile::HepMCReadFromFile(const std::string& name, ISvcLocator* pSvcLocator) :
  GenBase(name, pSvcLocator)
{
  declareProperty("InputFile", m_input_file="events.hepmc");
  m_event_number = 0;
  m_sum_xs = 0;
}


StatusCode HepMCReadFromFile::initialize() {
  CHECK(GenBase::initialize());
  
  // Initialize input file and event number
  m_hepmcio = HepMC3::deduce_reader(m_input_file);
  m_event_number = 0;
  return StatusCode::SUCCESS;
}


StatusCode HepMCReadFromFile::execute(const EventContext& /*ctx*/) {

  McEventCollection* mcEvtColl = nullptr;

  if ( evtStore()->contains<McEventCollection>(m_mcEventKey) && evtStore()->retrieve(mcEvtColl, m_mcEventKey).isSuccess() ) {
    if (msgLvl(MSG::VERBOSE)) msg(MSG::VERBOSE) << "found an McEventCollecion in store" << endmsg;
  } else {
    // McCollection doesn't exist. Create it (empty)
    if (msgLvl(MSG::VERBOSE)) msg(MSG::VERBOSE) << "create new McEventCollecion in store" << endmsg;
    mcEvtColl = new McEventCollection;
    StatusCode status = evtStore()->record( mcEvtColl, m_mcEventKey );
    if (status.isFailure()) {
      msg(MSG::ERROR) << "Could not record McEventCollection" << endmsg;
      return status;
    }
  }
  HepMC3::GenEvent* evt = new HepMC3::GenEvent();
  if (m_hepmcio) {
    m_hepmcio->read_event(*evt);
    if (!evt->run_info()) evt->set_run_info(m_hepmcio->run_info());
    ++m_event_number;
    evt->set_event_number(m_event_number);
    evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
    mcEvtColl->push_back(evt);
    const auto cs = evt->cross_section();
    double xs = 0;
    if ( cs ){
       xs=cs->xsec();
    }
    m_sum_xs = m_sum_xs+xs; 

  }
  return StatusCode::SUCCESS;
}

StatusCode HepMCReadFromFile::finalize() {

 if (m_sum_xs >0)  std::cout << "MetaData: cross-section (nb)= " << m_sum_xs/(1000*m_event_number) <<std::endl;
      
  return StatusCode::SUCCESS;
}  





