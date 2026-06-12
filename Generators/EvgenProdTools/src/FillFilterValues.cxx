/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

#ifdef HEPMC3
StatusCode FillFilterValues::execute(const EventContext& ctx) {
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
  
  SG::WriteDecorHandle<xAOD::EventInfo,float> dec_filtHT(m_mcFilterHTKey, ctx);
  if (event_const()->attribute<HepMC3::DoubleAttribute>(HepMCStr::filterHT) != NULL){
     std::shared_ptr<HepMC3::DoubleAttribute>  fHT =   event_const()->attribute<HepMC3::DoubleAttribute>(HepMCStr::filterHT); 
     double fHT_double = fHT->value();
     dec_filtHT(0) = fHT_double;
  }

  SG::WriteDecorHandle<xAOD::EventInfo,float> dec_filtMET(m_mcFilterMETKey, ctx);
  if (event_const()->attribute<HepMC3::DoubleAttribute>(HepMCStr::filterMET) != NULL){
    std::shared_ptr<HepMC3::DoubleAttribute>  fMET =   event_const()->attribute<HepMC3::DoubleAttribute>(HepMCStr::filterMET);
    double fMET_double = fMET->value();
    dec_filtMET(0) = fMET_double;
  }
 
  // Post-hoc debug printouts
  ATH_MSG_DEBUG("Copied HepMC filter values to EventInfo");

  return StatusCode::SUCCESS;
}
#else
StatusCode FillFilterValues::execute(const EventContext& /*ctx*/) {
  return StatusCode::SUCCESS;
}
#endif

#endif

