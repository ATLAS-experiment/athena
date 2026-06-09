/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorModules/GenModule.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/Incident.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include <fstream>


GenModule::GenModule(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
{
  m_mkMcEvent = true;
}


StatusCode GenModule::initialize() {
  // Base class initializations
  CHECK(GenBase::initialize());
  // Get the random number service
  CHECK(m_rndmSvc.retrieve());
  // Get the incident service
  CHECK(m_incidentSvc.retrieve());
  CHECK(genInitialize());
  CHECK(genuserInitialize());
  return StatusCode::SUCCESS;
}


CLHEP::HepRandomEngine* GenModule::getRandomEngine(const std::string& streamName,
                                                             const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  return rngWrapper->getEngine(ctx);
}


CLHEP::HepRandomEngine* GenModule::getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset,
                                                             const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  rngWrapper->setSeed( streamName, ctx.slot(), randomSeedOffset, ctx.eventID().run_number() );
  return rngWrapper->getEngine(ctx);
}


CLHEP::HepRandomEngine* GenModule::getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun, unsigned int lbn) const
{
  const size_t slot=0;
  EventContext ctx;
  ctx.setSlot( slot );
  ctx.setEventID (EventIDBase (conditionsRun,
               EventIDBase::UNDEFEVT,  // event
               EventIDBase::UNDEFNUM,  // timestamp
               EventIDBase::UNDEFNUM,  // timestamp ns
               lbn));
  Atlas::setExtendedEventContext(ctx,
                                 Atlas::ExtendedEventContext( evtStore()->hiveProxyDict(),
                                                              conditionsRun) );
  return getRandomEngine(streamName, randomSeedOffset, ctx);
}


StatusCode GenModule::execute(const EventContext& /*ctx*/) {
  // Examples of how to retrieve the random number engine for a given
  // stream.
  // NB getRandomEngine should only be called once per event for a
  // given stream, as it causes the stream to be re-seeded each time
  // it is called.

  // Example 1 - seeded based on the current event number (+ slot, run, streamName)
  //CLHEP::HepRandomEngine* rndmEngine = this->getRandomEngine("MyStream", ctx);

  // Example 2 - seeded based on the m_randomSeed property (+ slot, run, streamName)
  //CLHEP::HepRandomEngine* rndmEngine = this->getRandomEngine("MyStream", m_randomSeed.value(), ctx);

  // Call the code that generates an event
  CHECK(this->callGenerator());

  // Create the MC event and send the GeneratorEvent stored in it to fillEvt
  HepMC::GenEvent* evt = HepMC::newGenEvent(1,1);
  CHECK(this->fillEvt(evt));
  HepMC::fillBarcodesAttribute(evt);

  // Add the event to the MC event collection
  if (events()) {
    // If this is an "afterburner" generator, replace the last event rather than add a new one
    /// @todo Remove hard-coded alg name checking (already incomplete)
    if (m_isAfterburner.value() || name() == "Tauola" || name() == "Photos") {
      events()->pop_back();
    }
    // Add the event to the end of the collection
    events()->push_back(evt);
    ATH_MSG_DEBUG("MC event added to McEventCollection");

    // remove the empty event in case of ParticleDecayer
    if (name() == "ParticleDecayer") {
      events()->pop_back();
    }
  }

  // Call the incident service to notify that an event has been made
  m_incidentSvc->fireIncident( Incident(name(), "McEventGenerated") );
  return StatusCode::SUCCESS;
}
