/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <algorithm>
#include <random>

#include "ActsFatrasSimTool.h"
#include "Acts/ActsVersion.hpp"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandomEngine.h"

#include "TruthUtils/MagicNumbers.h"
#include "ISF_Event/ISFTruthIncident.h"

using namespace Acts::UnitLiterals;

ISF::ActsFatrasSimTool::ActsFatrasSimTool(const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent)
    : BaseSimulatorTool(type, name, parent) {}

ISF::ActsFatrasSimTool::~ActsFatrasSimTool() {}

StatusCode ISF::ActsFatrasSimTool::initialize() {
  ATH_CHECK(BaseSimulatorTool::initialize());
  ATH_MSG_INFO("ISF::ActsFatrasSimTool update with ACTS version: v"
    << Acts::VersionMajor << "." << Acts::VersionMinor << "."
    << Acts::VersionPatch << " [" << Acts::CommitHash << "]");
  // Retrieve particle filter
  if (!m_particleFilter.empty()) ATH_CHECK(m_particleFilter.retrieve());

  // setup logger
  m_logger = makeActsAthenaLogger(this, std::string("ActsFatras"),std::string("ActsFatrasSimTool"));

  // retrive tracking geo tool
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  m_trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  
  //retrive Magnetfield tool
  ATH_MSG_VERBOSE("Using ATLAS magnetic field service");
  ATH_CHECK( m_fieldCacheCondObjInputKey.initialize());

  // Random number service
  if (m_rngSvc.retrieve().isFailure()) {
    ATH_MSG_FATAL("Could not retrieve " << m_rngSvc);
    return StatusCode::FAILURE;
  }
  // Get own engine with own seeds
  m_randomEngine = m_rngSvc->getEngine(this, m_randomEngineName.value());
  if (!m_randomEngine) {
    ATH_MSG_FATAL("Could not get random engine '" << m_randomEngineName.value() << "'");
    return StatusCode::FAILURE;
  }

  // ISF truth service
  ATH_CHECK (m_truthRecordSvc.retrieve());
  ATH_MSG_DEBUG( "- Using ISF TruthRecordSvc : " << m_truthRecordSvc.typeAndName() );
  return StatusCode::SUCCESS;
}

StatusCode ISF::ActsFatrasSimTool::simulate(const EventContext& ctx,
  ISFParticle& isp, ISFParticleContainer& secondaries,
  McEventCollection* mcEventCollection) {
  ATH_MSG_VERBOSE("Particle " << isp << " received for simulation.");
  // Check if particle passes filter, if there is one
  if (!m_particleFilter.empty() && !m_particleFilter->passFilter(isp)) {
    ATH_MSG_VERBOSE("ISFParticle " << isp << " does not pass selection. Ignoring.");
    return StatusCode::SUCCESS;
  }
  // Process ParticleState from particle stack
  // Wrap the input ISFParticle in an STL vector with size of 1
  const ISF::ISFParticleVector ispVector(1, &isp);
  ATH_CHECK(this->simulateVector(ctx, ispVector, secondaries, mcEventCollection));
  ATH_MSG_VERBOSE("Simulation done");
  return StatusCode::SUCCESS;
}

StatusCode ISF::ActsFatrasSimTool::simulateVector(
                                                  const EventContext& ctx,
                                                  const ISFParticleVector& particles,
                                                  ISFParticleContainer& secondaries,
                                                  McEventCollection* /*mcEventCollection*/, McEventCollection *) {

  m_randomEngine->setSeed(m_randomEngineName, ctx);
  CLHEP::HepRandomEngine* randomEngine = m_randomEngine->getEngine(ctx);
  Generator generator(CLHEP::RandFlat::shoot(randomEngine->flat()));
  ATH_MSG_VERBOSE(name() << " RNG seed " << CLHEP::RandFlat::shoot(randomEngine->flat()));
  ATH_MSG_VERBOSE(name() << " received vector of size "
               << particles.size() << " particles for simulation.");

  // construct the ACTS simulator
  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometry }, m_logger);
  auto bField = std::make_shared<ATLASMagneticFieldWrapper>();
  auto chargedStepper = ChargedStepper(std::move(bField));
  auto neutralStepper = NeutralStepper();
  auto chargedPropagator = ChargedPropagator(chargedStepper, navigator, m_logger);
  auto neutralPropagator = NeutralPropagator(neutralStepper, navigator, m_logger);
  ChargedSimulation simulatorCharged(std::move(chargedPropagator), m_logger);
  NeutralSimulation simulatorNeutral(std::move(neutralPropagator), m_logger);
  Simulation simulator=Simulation(std::move(simulatorCharged),std::move(simulatorNeutral));
  ATH_MSG_VERBOSE(name() << " Min pT for interaction " << m_interact_minPt * Acts::UnitConstants::MeV << " GeV");
  // Acts propagater options
  simulator.charged.maxStepSize = m_maxStepSize;
  simulator.charged.maxStep = m_maxStep;
  simulator.charged.pathLimit = m_pathLimit;
  simulator.charged.maxRungeKuttaStepTrials = m_maxRungeKuttaStepTrials;
  simulator.charged.loopProtection = m_loopProtection;
  simulator.charged.loopFraction = m_loopFraction;
  simulator.charged.targetTolerance = m_tolerance;
  simulator.charged.stepSizeCutOff = m_stepSizeCutOff;
  // Create interaction list
  simulator.charged.interactions = ActsFatras::makeStandardChargedElectroMagneticInteractions(m_interact_minPt * Acts::UnitConstants::MeV);
  // get Geo and Mag map
  ATH_MSG_VERBOSE(name() << " Getting per event Geo and Mag map");
  Acts::MagneticFieldContext mctx = getMagneticFieldContext(ctx);
  const ActsGeometryContext& gctx = m_trackingGeometryTool->getNominalGeometryContext();
  auto anygctx = gctx.context();
  // Loop over ISFParticleVector and process each separately
  ATH_MSG_VERBOSE(name() << " Processing particles in ISFParticleVector.");
  // For sihit creation
  SiHitCollection pixelSiHits;
  SiHitCollection sctSiHits;
  for (const auto isfp : particles) {
    // ====ACTSFatras Simulation====
    // //  
    // input/output particle and hits containers
    // Convert to ActsFatras::Particle
    // ISF: Energy, mass, and momentum are in MeV, position in mm
    // Acts: Energy, mass, and momentum are in GeV, position in mm
    ATH_MSG_DEBUG(name() << " Convert ISF::Particle(mass) " << isfp->id()<<"|" << isfp<<"(" << isfp->mass() << ")");
    std::vector<ActsFatras::Particle> input = std::vector<ActsFatras::Particle>{
      ActsFatras::Particle(ActsFatras::Barcode().setVertexPrimary(0).setParticle(isfp->id()), static_cast<Acts::PdgParticle>(isfp->pdgCode()),
                           isfp->charge(),isfp->mass() * Acts::UnitConstants::MeV)
        .setDirection(Acts::makeDirectionFromPhiEta(isfp->momentum().phi(), isfp->momentum().eta()))
        .setAbsoluteMomentum(isfp->momentum().mag() * Acts::UnitConstants::MeV)
        .setPosition4(ActsTrk::convertPosToActs(isfp->position(), isfp->timeStamp()))};
    ATH_MSG_DEBUG(name() << " Propagating ActsFatras::Particle  vertex|particle|generation|subparticle, " << input[0]);
    std::vector<ActsFatras::Particle> simulatedInitial;
    std::vector<ActsFatras::Particle> simulatedFinal;
    std::vector<ActsFatras::Hit> hits;
    // simulate
    auto result=simulator.simulate(anygctx, mctx, generator, input, simulatedInitial, simulatedFinal, hits);
    auto simulatedFailure=result.value();
    if (simulatedFailure.size()>0){
      for (const auto& simfail : simulatedFailure){
        auto errCode = Acts::make_error_code(Acts::PropagatorError(simfail.error.value()));
        ATH_MSG_WARNING(name() << " Particle id " <<simfail.particle.particleId()<< ": fail to be simulated during Propagation: " << errCode.message());
        ATH_MSG_WARNING(name() << " Particle vertex|particle|generation|subparticle"<<simfail.particle << " starts from position" << Acts::toString(simfail.particle.position()) << " and direction " << Acts::toString(simfail.particle.direction()));
        return StatusCode::SUCCESS;
      }
    }

    ATH_MSG_DEBUG(name() << " initial particle " << simulatedInitial[0]);
    ATH_MSG_DEBUG(name() << " ActsFatras simulator hits: " << hits.size());
    int i = 0;
    for (const auto& hit : hits) {
      ATH_MSG_DEBUG(name() << " hit pos: " << hit.position() );
      ++i;
      if (i>5) break;
    }
    ATH_MSG_DEBUG(name() << " No. of particles after ActsFatras simulator: " << simulatedFinal.size());
    if (!simulatedFinal.empty()){
      ATH_MSG_DEBUG(name() << " start procesing secondaries");
      auto itr = simulatedFinal.begin();
      // Save hits of isfp
      std::vector<ActsFatras::Hit> particle_hits;
      std::copy(hits.begin(), hits.begin()+itr->numberOfHits(), std::back_inserter(particle_hits));
      m_ActsFatrasWriteHandler->createHits(*isfp, m_trackingGeometry,particle_hits,m_pixelSiHits,m_sctSiHits);
      // Process secondaries
      auto isKilled = !itr->isAlive();
      int maxGeneration = (simulatedFinal.back()).particleId().generation();
      ATH_MSG_DEBUG(name() << " maxGeneration: "<< maxGeneration);
      for (int gen = 0; gen <= maxGeneration; ++gen){
        ATH_MSG_DEBUG(name() << " start with genration "<< gen << "|" << maxGeneration << ": "<< *itr);
        auto vecsecisfp = std::make_unique<ISF::ISFParticleVector>();
        while (static_cast<int>(itr->particleId().generation()) == gen){
          ATH_MSG_DEBUG(name() << " genration "<< gen << "|" << maxGeneration << ": "<< *itr);
          if(itr->isSecondary()){
            // convert final particles to ISF::particle
            const auto pos = ActsTrk::convertPosFromActs(itr->fourPosition()).first;
            const auto mom = ActsTrk::convertMomFromActs(itr->fourMomentum()).first;
            double mass = itr->mass() / Acts::UnitConstants::MeV;
            double charge = itr->charge();
            int pdgid = itr->pdg();
            auto properTime = ActsTrk::timeToAthena(itr->time());
            const int status = 1 + HepMC::SIM_STATUS_THRESHOLD;
            const int id = HepMC::UNDEFINED_ID;
            auto secisfp = new ISF::ISFParticle (pos,mom,mass,charge,pdgid,status,properTime,*isfp,id);
            ATH_MSG_DEBUG(name() <<" secondaries particle (ACTS): "<<*itr<< "("<<itr->momentum()<<")|time "<<itr->time()<<"|process "<< getATLASProcessCode(itr->process()));
            ATH_MSG_DEBUG(name() <<" secondaries particle (ISF): " << *secisfp << " time "<<secisfp->timeStamp());
            vecsecisfp->push_back(secisfp);
          }
          else{
            ATH_MSG_DEBUG(name() <<" primary particle found with generation "<< gen);
          }
          ++itr;
        }
        if (!vecsecisfp->empty()) {
          ISF::ISFTruthIncident truth(*isfp,
                                      *vecsecisfp,
                                      getATLASProcessCode((itr-1)->process()),
                                      isfp->nextGeoID(),
                                      isKilled&&gen==maxGeneration ? ISF::fKillsPrimary : ISF::fPrimarySurvives
                                    );
          ATH_MSG_DEBUG(name() << " Truth incident parentPt2(MinPt2) " << truth.parentPt2() <<" (100 MeV)");
          ATH_MSG_DEBUG(name() << " Truth incident ChildPt2(MinPt2) " << truth.childrenPt2Pass(300) <<" (300 MeV)");
          m_truthRecordSvc->registerTruthIncident(truth,  true);
          truth.updateParentAfterIncidentProperties();
          truth.updateChildParticleProperties();
          for (auto *secisfp : *vecsecisfp){
            if (secisfp->getTruthBinding()) {
                secondaries.push_back(secisfp);
            }
            else {
                ATH_MSG_WARNING("Secondary particle not written out to truth.\n Parent (" << isfp << ")\n Secondary (" << *secisfp <<")");
            }
          } // end of truth binding 
        }// end of store truth bind secondaries
      } 
    }// end of secondaries
    ATH_MSG_VERBOSE(name() << " No. of secondaries: " << secondaries.size());
    ATH_MSG_DEBUG(name() << " End of particle " << isfp->barcode());
    m_ActsFatrasWriteHandler->createHits(*isfp, m_trackingGeometry,hits,pixelSiHits,sctSiHits);

    std::vector<ActsFatras::Particle>().swap(input);
    std::vector<ActsFatras::Particle>().swap(simulatedInitial);
    std::vector<ActsFatras::Particle>().swap(simulatedFinal);
    std::vector<ActsFatras::Hit>().swap(hits);
  } // end of isfp loop
  return StatusCode::SUCCESS;
}

Acts::MagneticFieldContext ISF::ActsFatrasSimTool::getMagneticFieldContext(const EventContext& ctx) const {
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
  if (!readHandle.isValid()) {
    ATH_MSG_ERROR(name() + ": Failed to retrieve magnetic field condition data " + m_fieldCacheCondObjInputKey.key() + ".");
  }
  else ATH_MSG_DEBUG(name() << "retrieved magnetic field condition data "<< m_fieldCacheCondObjInputKey.key());
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

  return Acts::MagneticFieldContext(fieldCondObj);
}
