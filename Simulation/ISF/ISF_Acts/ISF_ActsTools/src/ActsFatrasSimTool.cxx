/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <algorithm>
#include <random>

#include "ActsFatrasSimTool.h"
#include "Acts/ActsVersion.hpp"
#include <Acts/Utilities/StringHelpers.hpp>
#include "Acts/Definitions/PdgParticle.hpp"

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
  ATH_CHECK(m_ctxProvider.initialize());
  ATH_MSG_INFO("ISF::ActsFatrasSimTool update with ACTS version: v"
    << Acts::VersionMajor << "." << Acts::VersionMinor << "."
    << Acts::VersionPatch << " [" << Acts::CommitHash.value_or("unknown hash") << "]");
  // Retrieve particle filter
  ATH_CHECK(m_particleFilter.retrieve());
  ATH_MSG_INFO("Using particle filter: " << m_particleFilter.typeAndName());
  // setup logger
  m_logger = makeActsAthenaLogger(this, std::string("ActsFatras"),std::string("ActsFatrasSimTool"));

  // Geometry identifier service
  if ( !m_geoIDSvc.empty() && m_geoIDSvc.retrieve().isFailure()){
    ATH_MSG_FATAL ("Could not retrieve " << m_geoIDSvc);
    return StatusCode::FAILURE;
  }

  // retrive tracking geo tool
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();

  // Acts Extrapolator
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_MSG_INFO( "- ActsExtrapolationTool : " << m_extrapolationTool.typeAndName() );
  
  //retrive Magnetfield tool
  ATH_MSG_VERBOSE("Using ATLAS magnetic field service");
  
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
  // filter particles
  std::vector<ISFParticle*> selectedParticles;
  for (const auto isfp : particles) {
    if (!m_particleFilter.empty() && !m_particleFilter->passFilter(*isfp)) {
      ATH_MSG_VERBOSE("ISFParticle " << *isfp << " does not pass selection. Ignoring.");
      continue;
    }
    selectedParticles.push_back(isfp);
  }
  if (selectedParticles.empty()) {
    ATH_MSG_VERBOSE("No particles passed selection. Ignoring.");
    return StatusCode::SUCCESS;
  }
  // Set random seed for current event                            
  m_randomEngine->setSeed(m_randomEngineName, ctx);
  CLHEP::HepRandomEngine* randomEngine = m_randomEngine->getEngine(ctx);
  Generator generator(CLHEP::RandFlat::shoot(randomEngine->flat()));
  ATH_MSG_VERBOSE(name() << " RNG seed " << CLHEP::RandFlat::shoot(randomEngine->flat()));
  ATH_MSG_VERBOSE(name() << " received vector of size "
               << selectedParticles.size() << " particles for simulation.");

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

  // Construct the ACTS propagator for starting surface check
  auto surfaceCheckPropagator = ChargedPropagator(chargedStepper, navigator);
  
  // get Geo and Mag map
  ATH_MSG_VERBOSE(name() << " Getting per event Geo and Mag map");
  auto mctx = m_ctxProvider.getMagneticFieldContext(ctx);
  auto anygctx = m_ctxProvider.getGeometryContext(ctx);
  // Loop over ISFParticleVector and process each separately
  ATH_MSG_VERBOSE(name() << " Processing particles in ISFParticleVector.");
  for (const auto isfp : selectedParticles) {
    // ====ACTSFatras Simulation====
    // //  
    // input/output particle and hits containers
    // Convert to ActsFatras::Particle
    // ISF: Energy, mass, and momentum are in MeV, position in mm
    // Acts: Energy, mass, and momentum are in GeV, position in mm
    ATH_MSG_DEBUG(name() << " Convert ISF::Particle(mass) " << isfp->id()<<"|" << *isfp<<"(" << isfp->mass() << ")");
    std::vector<ActsFatras::Particle> input = std::vector<ActsFatras::Particle>{
      ActsFatras::Particle(ActsFatras::Barcode().withVertexPrimary(0).withParticle(isfp->id()), static_cast<Acts::PdgParticle>(isfp->pdgCode()),
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
      if (itr->numberOfHits() > 0) {
        std::copy(hits.begin(), hits.begin()+itr->numberOfHits(), std::back_inserter(particle_hits));
        m_ActsFatrasWriteHandler->createHits(*isfp, m_trackingGeometry,particle_hits,m_pixelSiHits,m_sctSiHits);
      }
      // Process secondaries
      auto isKilled = !itr->isAlive();
      int maxGeneration = simulatedFinal.back().particleId().generation();
      ATH_MSG_DEBUG(name() << " maxGeneration: "<< maxGeneration);
      for (int gen = 0; gen <= maxGeneration; ++gen){
        ATH_MSG_DEBUG(name() << " start with generation "<< gen << "|" << maxGeneration << ": "<< *itr);
        auto vecsecisfp = std::make_unique<ISF::ISFParticleVector>();
        std::unique_ptr<ISF::ISFParticle> newisfp = nullptr;  // Boundary crossing particle
        
        while (itr != simulatedFinal.end() && static_cast<int>(itr->particleId().generation()) == gen) {
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
            auto secisfp = std::make_unique<ISF::ISFParticle>(pos,mom,mass,charge,pdgid,status,properTime,*isfp,id);
            secisfp->setNextGeoID(m_geoIDSvc->identifyNextGeoID(*secisfp));
            ATH_MSG_DEBUG(name() <<" secondaries particle (ACTS): "<<*itr<< "("<<itr->momentum()<<")|time "<<itr->time()<<"|process "<< getATLASProcessCode(itr->process()));
            ATH_MSG_DEBUG(name() <<" secondaries particle (ISF): pdg=" << secisfp->pdgCode() 
              << " pos=" << secisfp->position() << " mom=" << secisfp->momentum() 
              << " GeoID=" << m_geoIDSvc->identifyNextGeoID(*secisfp));
            vecsecisfp->push_back(secisfp.release());
          }
          else{
            // Primary particle handling
            ATH_MSG_DEBUG(name() <<" primary particle found with generation ("<< gen <<")");
            // After simulation, check particle's final state
            if (!isKilled) {
              auto fisfp = std::make_unique<ISF::ISFParticle>(*isfp);
              fisfp->updateMomentum(ActsTrk::convertMomFromActs(itr->fourMomentum()).first);
              fisfp->updatePosition(ActsTrk::convertPosFromActs(itr->fourPosition()).first);
              ATH_MSG_DEBUG(name() << " After simulation, primary particle state: " << *fisfp);
              if (!m_particleFilter.empty() && !m_particleFilter->passFilter(*fisfp)) {
                ATH_MSG_VERBOSE("ISFParticle" << fisfp << "  after simulation does not pass selection. Ignoring for boundary check.");
                continue;
              }
              ATH_MSG_DEBUG(name() << " [ISF] original GeoID: " <<  m_geoIDSvc->identifyGeoID(*isfp) 
                            << " new particle GeoID: " << m_geoIDSvc->identifyGeoID(*fisfp) 
                            << ", nextGeoID: " << m_geoIDSvc->identifyNextGeoID(*fisfp));

              // Use ActsExtrapolationTool
              ATH_MSG_DEBUG(name() << " Extrapolating using ActsExtrapolationTool");
              
              // Convert to ACTS BoundTrackParameters for extrapolation
              Acts::BoundTrackParameters startParams = Acts::BoundTrackParameters::createCurvilinear(
                  itr->fourPosition(), itr->direction(), itr->qOverP(), std::nullopt, itr->hypothesis());

              //=============== try if a starting surface exist before passing to Extrapolator ==
              auto do_exit_startsurface = checkStartSurface(mctx, anygctx, surfaceCheckPropagator, startParams);
              ATH_MSG_DEBUG(name() << " checkStartSurface returned: " << do_exit_startsurface);
              if (do_exit_startsurface) {
                ATH_MSG_DEBUG(name() << " Particle starts at a valid surface, doing extrapolation...");
              
                // Extrapolate and get propagation steps
                auto nextGeoID = AtlasDetDescr::fUndefinedAtlasRegion;
                Amg::Vector3D entryPos{Amg::Vector3D::Zero()};
                try {
                  auto stepsResult = m_extrapolationTool->propagationSteps(ctx, startParams, Acts::Direction::Forward());
                  auto steps = stepsResult.value().first;
                  ATH_MSG_DEBUG(name() << " Number of propagation steps: " << steps.size());
                  if (steps.size() != 0) {
                    for (const auto& step : steps) {
                      ATH_MSG_DEBUG(name() << " [Acts] Step at position " << step.position 
                                    << " (eta " << Acts::VectorHelpers::eta(step.position) 
                                    << ") with GeoID " << step.geoID);
                      entryPos = convertPos3FromActs(step.position);
                      nextGeoID = m_geoIDSvc->identifyGeoID(entryPos);
                      ATH_MSG_DEBUG(name() << " [Acts] GeoID from service: " << nextGeoID);
                      if (nextGeoID > AtlasDetDescr::fAtlasID) { // Valid boundary crossing
                        ATH_MSG_DEBUG(name() << " Boundary crossing detected at GeoID " << nextGeoID);
                        break;
                      }
                    }
                  } else {
                    ATH_MSG_WARNING(name() << " No propagation steps returned by ActsExtrapolationTool");
                  }
                }
                catch (const std::exception& e) {
                  ATH_MSG_WARNING(name() << " extrapolation [" << m_extrapolationTool.name() << "] failed: " << e.what() << "\nSkip boundary check for " << *fisfp);
                  break; // Skip boundary check for this particle and continue with next one
                }

                if (fisfp && nextGeoID > AtlasDetDescr::fAtlasID){
                  const auto mom = ActsTrk::convertMomFromActs(itr->fourMomentum()).first;
                  double mass = itr->mass() / Acts::UnitConstants::MeV;
                  double charge = itr->charge();
                  int pdgid = itr->pdg();
                  auto properTime = ActsTrk::timeToAthena(itr->time());
                  
                  // Create boundary crossing particle
                  newisfp = std::make_unique<ISF::ISFParticle>(entryPos, mom, mass, charge, pdgid, isfp->status(), properTime, *isfp, isfp->id(), isfp->barcode());
                  newisfp->setNextGeoID(nextGeoID);
                  ATH_MSG_DEBUG(name() << " Truthbinding of parent ISFParticle: " << (isfp->getTruthBinding() ? "exists" : "null"));
                  if (isfp->getTruthBinding()) {
                    ATH_MSG_DEBUG(name() << " Current GenParticle: " << isfp->getTruthBinding()->getCurrentGenParticle());
                  }
                  ATH_MSG_DEBUG(name() << " Created new ISFParticle at boundary with nextGeoID: " 
                                  << AtlasDetDescr::AtlasRegionHelper::getName(nextGeoID) 
                                  << "(" << nextGeoID << ")");
                }

                // Handle boundary crossing particle separately - DON'T add to vecsecisfp yet
                if (newisfp && nextGeoID > AtlasDetDescr::fAtlasID) {
                  ATH_MSG_DEBUG(name() << " [ISF] Processing boundary particle with nextGeoID: " 
                                << AtlasDetDescr::AtlasRegionHelper::getName(newisfp->nextGeoID()) 
                                << "(" << newisfp->nextGeoID() << ")");
                  
                  // Identify Entrylayer
                  ISF::EntryLayer entryLayer = ISF::fUnsetEntryLayer;
                  
                  switch(nextGeoID) {
                    case AtlasDetDescr::fAtlasCalo:
                      entryLayer = ISF::fAtlasCaloEntry;
                      ATH_MSG_DEBUG("Particle crossing to Calorimeter");
                      break;
                    case AtlasDetDescr::fAtlasMS:
                      entryLayer = ISF::fAtlasMuonEntry;
                      ATH_MSG_DEBUG("Particle crossing to Muon System");
                      break;
                    default:
                      ATH_MSG_DEBUG("Particle at unspecified boundary");
                      break;
                  }
                  
                  if (entryLayer != ISF::fUnsetEntryLayer) {
                        vecsecisfp->push_back(newisfp.release()); // Add boundary crossing particle to secondaries vector
                  } else {
                    ATH_MSG_WARNING("Invalid entry layer for boundary particle");
                  }
                }
              }
              else {
                ATH_MSG_DEBUG(name() << " No starting surface found, skipping boundary check and extrapolation.");
              }
            } // end of !isKilled
          } // end of primary vs secondary
          ++itr;
        } // end of while loop over particles in generation

        // Process truth for this generation
        if (!vecsecisfp->empty()) {
          // Determine process code and geoID based on whether we have boundary crossing
          int processCode = 0;
          AtlasDetDescr::AtlasRegion geoID = AtlasDetDescr::fAtlasID;
          auto isParentKilled = ISF::fPrimarySurvives;
          
          if (newisfp) {
            // Boundary crossing - use boundary info
            processCode = 91;  // Boundary crossing has no process
            geoID = newisfp->nextGeoID();
            isParentKilled = ISF::fKillsPrimary;
          } else {
            // Regular secondaries - use process from last particle
            processCode = getATLASProcessCode((itr-1)->process());
            geoID = (isfp->nextGeoID() <= AtlasDetDescr::fUndefinedAtlasRegion) ? 
                    AtlasDetDescr::fAtlasID : isfp->nextGeoID();
            isParentKilled = isKilled && gen==maxGeneration ? ISF::fKillsPrimary : ISF::fPrimarySurvives;
          }
          
          ISF::ISFTruthIncident truth(*isfp,
                                      *vecsecisfp,
                                      processCode,
                                      geoID,
                                      isParentKilled);
          
          ATH_MSG_DEBUG(name() << " Truth incident parentPt2(MinPt2) " << truth.parentPt2() <<" (100 MeV)");
          ATH_MSG_DEBUG(name() << " Truth incident ChildPt2(MinPt2) " << truth.childrenPt2Pass(300) <<" (300 MeV)");
          m_truthRecordSvc->registerTruthIncident(truth,  true);
          truth.updateParentAfterIncidentProperties();
          truth.updateChildParticleProperties();          
          for (auto *secisfp : *vecsecisfp){
            if (secisfp->getTruthBinding()) {
              secondaries.push_back(secisfp);
              ATH_MSG_DEBUG(name() << " Secondary particle written out to truth.\n Parent (" 
                              << *isfp << ")\n Secondary (" << *secisfp <<")");

              ATH_MSG_DEBUG("Secondary particle push back to ISF, TruthBinding: " << secisfp->getTruthBinding()->getCurrentGenParticle() << " (current) | " << secisfp->getTruthBinding()->getPrimaryGenParticle() << " (primary) | " << secisfp->getTruthBinding()->getGenerationZeroGenParticle() << " (zero)");
              if (secisfp->getTruthBinding()->getCurrentGenParticle() != nullptr) ATH_MSG_DEBUG("Secondary particle GenParticle EndVertex: " << (secisfp->getTruthBinding()->getCurrentGenParticle()->end_vertex() ? HepMC::barcode(secisfp->getTruthBinding()->getCurrentGenParticle()->end_vertex()) : 1));
            } else {
              ATH_MSG_WARNING("Secondary particle not written out to truth.\n Parent (" 
                              << *isfp << ")\n Secondary (" << *secisfp <<")");
              delete secisfp; // Clean up particles without truth binding
            }
          }
        }
      } // end of generation loop
    } // end of !simulatedFinal.empty()
    ATH_MSG_VERBOSE(name() << " No. of secondaries: " << secondaries.size());
    ATH_MSG_DEBUG(name() << " End of particle " << HepMC::uniqueID(isfp));

    std::vector<ActsFatras::Particle>().swap(input);
    std::vector<ActsFatras::Particle>().swap(simulatedInitial);
    std::vector<ActsFatras::Particle>().swap(simulatedFinal);
    std::vector<ActsFatras::Hit>().swap(hits);
  } // end of isfp loop
  return StatusCode::SUCCESS;
}

bool ISF::ActsFatrasSimTool::checkStartSurface(const Acts::MagneticFieldContext& mctx,
                                        const Acts::GeometryContext& anygctx,
                                        const ChargedPropagator& chargedPropagator,
                                        const Acts::BoundTrackParameters& startParameters,
                                        Acts::Direction navDir /*= Acts::Direction::Forward()*/,
                                        double pathLimit /*= std::numeric_limits<double>::max()*/) const
{

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");
  using ActorList =
  Acts::ActorList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
  using PropagatorOptions = typename ChargedPropagator::template Options<ActorList>;

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " Setting up propagator options for start surface check.");
  PropagatorOptions options(anygctx, mctx);
  options.loopProtection = (Acts::VectorHelpers::perp(startParameters.momentum()) < 300 * 1_MeV);
  options.direction = navDir;
  options.pathLimit = pathLimit;

  // The state creation triggers the initial volume/surface lookup
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " Initializing propagator state with start parameters: position " 
              << startParameters.position(anygctx).transpose() << ", momentum " 
              << startParameters.momentum().transpose());
  auto state = chargedPropagator.makeState(options);
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " Created propagator state. Now initializing with start parameters.");
  auto initResult = chargedPropagator.initialize(state, startParameters);
  if (!initResult.ok()) {
    ATH_MSG_WARNING(name() << "::" << __FUNCTION__ << " Failed to initialize propagator state: " 
                << initResult.error().message());
    return false;
  }
  return true;
}
