/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CopyMcEventCollection.h"
#include "AtlasHepMC/HeavyIon.h"

CopyMcEventCollection::CopyMcEventCollection(const std::string &name, ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode CopyMcEventCollection::initialize()
{
  ATH_MSG_DEBUG("Initializing...");

  // Check and initialize keys
  ATH_CHECK( m_bkgInputKey.initialize(!m_bkgInputKey.key().empty()) );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_bkgInputKey);
  ATH_CHECK( m_signalInputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_signalInputKey);
  ATH_CHECK( m_outputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputKey);

  ATH_CHECK( m_eventInfoKey.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode CopyMcEventCollection::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("execute() begin");

  // Reading the input containers
  ATH_MSG_VERBOSE("Retrieving input containers");

  const McEventCollection *bkgContainerPtr = nullptr;
  if (!m_bkgInputKey.key().empty()) {
    SG::ReadHandle<McEventCollection> bkgContainer(m_bkgInputKey, ctx);
    if (!bkgContainer.isValid()) {
      ATH_MSG_ERROR("Could not get background McEventCollection container " << bkgContainer.name() << " from store " << bkgContainer.store());
      return StatusCode::FAILURE;
    }
    bkgContainerPtr = bkgContainer.cptr();

    ATH_MSG_DEBUG("Found background McEventCollection container " << bkgContainer.name() << " in store " << bkgContainer.store());
  }

  SG::ReadHandle<McEventCollection> signalContainer(m_signalInputKey, ctx);
  if (!signalContainer.isValid()) {
    ATH_MSG_ERROR("Could not get signal McEventCollection container " << signalContainer.name() << " from store " << signalContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found signal McEventCollection container " << signalContainer.name() << " in store " << signalContainer.store());

  // Creating output RDO container
  SG::WriteHandle<McEventCollection> outputContainer(m_outputKey, ctx);
  ATH_CHECK(outputContainer.record(std::make_unique<McEventCollection>()));
  if (!outputContainer.isValid()) {
    ATH_MSG_ERROR("Could not record output McEventCollection container " << outputContainer.name() << " to store " << outputContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output McEventCollection container " << outputContainer.name() << " in store " << outputContainer.store());

  unsigned int droppedSeparatorGenEvents{0};
#ifdef HEPMC3
  int backupPileUpType{0};
  int backupBunchCrossingTime{0};
#endif
  // Copy signal GenEvents
  for (McEventCollection::const_iterator it = signalContainer->begin(); it != signalContainer->end(); ++it) {
#ifdef HEPMC3
    if ( (*it)->event_number() == -1 ) {
      ++droppedSeparatorGenEvents;
      ATH_MSG_VERBOSE("Signal: Skipping a separator GenEvent. " << droppedSeparatorGenEvents << " skipped so far.");
      backupBunchCrossingTime+=25; // NB This is a bit of a hack, but better than having everything in-time
      continue;
    }
#endif
    HepMC::GenEvent* copiedEvent = new HepMC::GenEvent(**it);
    HepMC::fillBarcodesAttribute(copiedEvent);
#ifdef HEPMC3
    auto bunchCrossingTime = (*it)->attribute<HepMC3::IntAttribute>("BunchCrossingTime");
    if (bunchCrossingTime) {
      copiedEvent->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(bunchCrossingTime->value()));
    }
    else {
      copiedEvent->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(backupBunchCrossingTime));
    }
    auto pileupType = (*it)->attribute<HepMC3::IntAttribute>("PileUpType");
    if (pileupType) {
      copiedEvent->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(pileupType->value()));
    }
    else {
      copiedEvent->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(backupPileUpType));
      if (backupPileUpType == 0) { backupPileUpType = 1; } // ignore the possibility for cavern background for this back-up case
    }
#endif
    if (!copiedEvent->heavy_ion() && (*it)->heavy_ion()) {
      // It should be clarified if we want to get a copy or the
      // content.
#ifdef HEPMC3
      HepMC::GenHeavyIonPtr hinew=std::make_shared<HepMC::GenHeavyIon>(*((*it)->heavy_ion()));
      copiedEvent->set_heavy_ion(std::move(hinew));
#else
      copiedEvent->set_heavy_ion(*((*it)->heavy_ion()));
#endif
    }
    outputContainer->push_back(copiedEvent);
  }

#ifdef HEPMC3
  backupBunchCrossingTime = 0;
#endif
  // Copy background GenEvents if configured
  if (!m_bkgInputKey.key().empty()) {
    McEventCollection::const_iterator it = bkgContainerPtr->begin();
    if (m_removeBkgHardScatterTruth.value()) {
      // Do not copy the single particle neutrino GenEvent.
      ++it;
    }
    for ( ; it != bkgContainerPtr->end(); ++it) {
#ifdef HEPMC3
      if ( (*it)->event_number() == -1 ) {
        ++droppedSeparatorGenEvents;
        ATH_MSG_VERBOSE("Background: Skipping a separator GenEvent. " << droppedSeparatorGenEvents << " skipped so far.");
        backupBunchCrossingTime+=25; // NB This is a bit of a hack, but better than having everything in-time
        continue;
      }
#endif
      HepMC::GenEvent* copiedEvent = new HepMC::GenEvent(**it);
      HepMC::fillBarcodesAttribute(copiedEvent);
#ifdef HEPMC3
      auto bunchCrossingTime = (*it)->attribute<HepMC3::IntAttribute>("BunchCrossingTime");
      if (bunchCrossingTime) {
        copiedEvent->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(bunchCrossingTime->value()));
      }
      else {
        copiedEvent->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(backupBunchCrossingTime));
      }
      auto pileupType = (*it)->attribute<HepMC3::IntAttribute>("PileUpType");
      if (pileupType) {
        copiedEvent->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(pileupType->value()));
      }
      else {
        copiedEvent->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(backupPileUpType));
        if (backupPileUpType == 0) { backupPileUpType = 1; } // ignore the possibility for cavern background for this back-up case
      }
#endif
      if (!copiedEvent->heavy_ion() && (*it)->heavy_ion()) {
        // It should be clarified if we want to get a copy or the
        // content.
#ifdef HEPMC3
        HepMC::GenHeavyIonPtr hinew=std::make_shared<HepMC::GenHeavyIon>(*((*it)->heavy_ion()));
        copiedEvent->set_heavy_ion(std::move(hinew));
#else
        copiedEvent->set_heavy_ion(*((*it)->heavy_ion()));
#endif
      }
      outputContainer->push_back(copiedEvent);
    }
  }
  ATH_MSG_VERBOSE("output size: " << outputContainer->size() << ", Signal Input size: " << signalContainer->size() << ", Bkg Input size: " << bkgContainerPtr->size() << ", dropped neutrino GenEvents: " << (m_removeBkgHardScatterTruth ? 1 : 0) << " , dropped Separator GenEvents: " << droppedSeparatorGenEvents);
  // dump McEventCollection in debug mode to confirm everything is as expected
  if (msgLvl(MSG::DEBUG)) {
    if (!outputContainer->empty()) {
      ATH_MSG_DEBUG("McEventCollection contents:");
      for (const HepMC::GenEvent *event : *outputContainer) {
        ATH_MSG_DEBUG("  GenEvent #" << event->event_number() << ", signal_process_id=" << HepMC::signal_process_id(event));
      }
    }
  }

  ATH_MSG_DEBUG("execute() end");
  return StatusCode::SUCCESS;
}
