/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "Photospp_i/Photospp_i.h"

#include "Photos/Photos.h"

#ifdef HEPMC3
#include "Photos/PhotosHepMC3Event.h"
namespace Photospp {
using PhotosHepMCEvent=PhotosHepMC3Event;
}
#else
#include "Photos/PhotosHepMCEvent.h"
#endif
#include "Photos/Log.h"

#include "GeneratorObjects/McEventCollection.h"
#include "AthenaKernel/errorcheck.h"

#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/RNGWrapper.h"
#include "TruthUtils/ParticleConstants.h"

using namespace Photospp;

// Pointer to random engine
CLHEP::HepRandomEngine*  Photospp_i::p_rndmEngine = nullptr;

void ignore_unused_variable_warning(int *) {}

extern "C" double phoranc_(int *idum) {
    ignore_unused_variable_warning(idum);
    return CLHEP::RandFlat::shoot(Photospp_i::p_rndmEngine);
}

////////////////////////////////////////////////////////////////////////////////
Photospp_i::Photospp_i(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator) {
}

////////////////////////////////////////////////////////////////////////////////
StatusCode Photospp_i::initialize() {
    ATH_MSG_DEBUG("Photospp_i initializing");

    ATH_CHECK(m_rndmSvc.retrieve());
    p_rndmEngine = getRandomEngineDuringInitialize("PHOTOSPP_INIT", m_randomSeed, m_dsid);

    if(!m_delayInitialisation) setupPhotos();

    return StatusCode::SUCCESS;
}

void Photospp_i::setupPhotos() {

    Photos::initialize();
    Photos::setAlphaQED(m_alphaQED);
    Photos::setInterference(true);
    Photos::setCorrectionWtForW(true);
    Photos::maxWtInterference(m_maxWtInterference);
    Photos::setMeCorrectionWtForW(m_WMECorrection);
    Photos::setMeCorrectionWtForZ(m_ZMECorrection);
    Photos::setPairEmission(m_photonSplitting);
    Photos::forceMassFrom4Vector(true);
    Photos::forceMassFromEventRecord(13);
    Photos::forceMassFromEventRecord(15);
    Photos::forceMass(11, ParticleConstants::electronMassInMeV); // The assumption that units are MEV will be checked later
    Photos::forceMassFromEventRecord(211);
    Photos::setTopProcessRadiation(false);
    Photos::createHistoryEntries(m_createHistory, 3);

    if(m_exponentiation) {
        Photos::setExponentiation(true);
    } else {
        Photos::setInfraredCutOff(0.01);
        Photos::setDoubleBrem(true);
        Photos::setQuatroBrem(false);
        phokey.iexp = 0;
    }

    // over-ride default IR cutoff if user has set a specific value
    if(m_infraRedCutOff > 0.) {
        Photos::setInfraredCutOff(m_infraRedCutOff);
    }

    Photospp::Log::LogWarning(false);
    Photospp::Photos::setStopAtCriticalError(m_stopCritical);

    Photos::iniInfo();

    return;
}


void Photospp_i::reseedRandomEngine(const std::string& streamName, const EventContext& ctx)
{
  long seeds[7];
  ATHRNG::calculateSeedsMC21(seeds, streamName,  ctx.eventID().event_number(), m_dsid, m_randomSeed);
  p_rndmEngine->setSeeds(seeds, 0); // NOT THREAD-SAFE
}


CLHEP::HepRandomEngine* Photospp_i::getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset,
                                                             const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  rngWrapper->setSeed( streamName, ctx.slot(), randomSeedOffset, ctx.eventID().run_number() );
  return rngWrapper->getEngine(ctx);
}


CLHEP::HepRandomEngine* Photospp_i::getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun, unsigned int lbn) const
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


StatusCode Photospp_i::execute() {

    // initialise if not done already

    if(m_delayInitialisation) {
        setupPhotos();
        m_delayInitialisation = false;
    }

    //Re-seed the random number stream
    const EventContext& ctx = Gaudi::Hive::currentContext();
    reseedRandomEngine("PHOTOSPP", ctx);

    // Get the event collection
    // FIXME should be using Read/WriteHandles here
    McEventCollection* eventCollection = nullptr;
    StatusCode sc = evtStore()->retrieve(eventCollection, m_genEventKey);
    if (sc.isFailure() || eventCollection == 0) {
        ATH_MSG_ERROR("Unable to retrieve event collection from StoreGate with key " << m_genEventKey);
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_DEBUG("Retrieved event collection from StoreGate with key " << m_genEventKey);
    }

    // Get the event most recently added to the event collection
    HepMC::GenEvent *event = eventCollection->back();
    if(event == 0) {
        ATH_MSG_ERROR("Photospp_i received a null HepMC event");
        return StatusCode::FAILURE;
    }
    switch(event->momentum_unit()) {
#ifdef HEPMC3
    case HepMC3::Units::MomentumUnit::GEV:
        Photos::setMomentumUnit(Photos::GEV);
        Photos::forceMass(11, ParticleConstants::electronMassInMeV/1000.); // Convert MeV to GeV
        break;
    case HepMC3::Units::MomentumUnit::MEV:
        Photos::setMomentumUnit(Photos::MEV);
        Photos::forceMass(11, ParticleConstants::electronMassInMeV);
        break;
#else
    case HepMC::Units::GEV:
        Photos::setMomentumUnit(Photos::GEV);
        Photos::forceMass(11, ParticleConstants::electronMassInMeV/1000.); // Convert MeV to GeV
        break;
    case HepMC::Units::MEV:
        Photos::setMomentumUnit(Photos::MEV);
        Photos::forceMass(11, ParticleConstants::electronMassInMeV);
        break;
#endif
    default:
        ATH_MSG_ERROR("Photospp_i received a event with unknown units.");
        Photos::setMomentumUnit(Photos::DEFAULT_MOMENTUM);
        break;
    };
    PhotosHepMCEvent photosEvent(event);
    photosEvent.process();

    return StatusCode::SUCCESS;
}
