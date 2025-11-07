/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "EvgenProdTools/FillFilterValues.h"
#include "StoreGate/WriteDecorHandle.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventType.h"

FillFilterValues::FillFilterValues(const std::string& name, ISvcLocator* svcLoc)
  : GenBase(name, svcLoc)
{
}

StatusCode FillFilterValues::initialize()
{
  ATH_CHECK(GenBase::initialize());
  ATH_CHECK(m_mcFilterHTKey.initialize());
  ATH_CHECK(m_mcFilterMETKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode FillFilterValues::execute() {
#ifdef HEPMC3
  // Check that the collection isn't empty
  const size_t nEvents = events_const()->size();
  if (nEvents == 0) {
    ATH_MSG_WARNING("McEventCollection is empty");
    return StatusCode::SUCCESS;
  }
  // Get the event info/type object to be filled
  const EventInfo* pInputEvt(nullptr);
  CHECK(evtStore()->retrieve(pInputEvt));
  assert(pInputEvt);

  // write filter values into xAOD::EventInfo
  
  SG::WriteDecorHandle<xAOD::EventInfo,float> dec_filtHT(m_mcFilterHTKey);
  if (event_const()->attribute<HepMC3::DoubleAttribute>("filterHT") != NULL){
     std::shared_ptr<HepMC3::DoubleAttribute>  fHT =   event_const()->attribute<HepMC3::DoubleAttribute>("filterHT"); 
     double fHT_double = fHT->value();
     dec_filtHT(0) = fHT_double;
  }

  SG::WriteDecorHandle<xAOD::EventInfo,float> dec_filtMET(m_mcFilterMETKey);
  if (event_const()->attribute<HepMC3::DoubleAttribute>("filterMET") != NULL){
    std::shared_ptr<HepMC3::DoubleAttribute>  fMET =   event_const()->attribute<HepMC3::DoubleAttribute>("filterMET");
    double fMET_double = fMET->value();
    dec_filtMET(0) = fMET_double;
  }
 
  // Post-hoc debug printouts
  ATH_MSG_DEBUG("Copied HepMC filter values to EventInfo");

#endif
  return StatusCode::SUCCESS;
}

#endif

