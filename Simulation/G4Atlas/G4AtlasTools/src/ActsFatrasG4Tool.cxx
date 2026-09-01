/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include <random>

// Amg includes
// These need to go before ActsFatras to not break definitions
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitives.h"

// class header
#include "ActsFatrasG4Tool.h"

// CLHEP
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandomEngine.h"

//ATLAS includes
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "MCTruth/TrackHelper.h"

// ACTS
#include "ActsGeometry/ActsDetectorElement.h"
#include "TrkSurfaces/Surface.h"
#include "Acts/ActsVersion.hpp"
#include <Acts/Utilities/StringHelpers.hpp>

// Geant4
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Region.hh"
#include "G4VPhysicalVolume.hh"

using namespace Acts::UnitLiterals;


StatusCode ActsFatrasG4Tool::initialize() {
  // print version name
  ATH_MSG_INFO(name() << " initialize start" );
  ATH_MSG_INFO("ActsFatrasG4Tool updated with ACTS version: v"
    << Acts::VersionMajor << "." << Acts::VersionMinor << "."
    << Acts::VersionPatch << " [" << Acts::CommitHash.value_or("unknown hash") << "]");

  // setup logger
  m_logger = makeActsAthenaLogger(this, std::string("ActsFatras"),std::string("ActsFatrasG4Tool"));

  // retrieve tracking geo tool
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
  ATH_MSG_DEBUG("Tracking geometry OK");  
  ATH_CHECK(m_ctxProvider.initialize());
  

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
  ATH_MSG_DEBUG("Random number services OK");

  // Get the Pixel Identifier-helper, must be in initialize() after calling detector store
  ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
  // Get the SCT Identifier-helper
  ATH_CHECK(detStore()->retrieve(m_sctIdHelper, "SCT_ID"));
  ATH_MSG_DEBUG("Pixel and SCT identifiers OK");

  return StatusCode::SUCCESS;
}

StatusCode ActsFatrasG4Tool::configureSimulator()
{
    ATH_MSG_INFO("Configuring Fatras simulator...");
    // protection against multiple calls
    if (m_simulator) {
      ATH_MSG_DEBUG("Simulator already configured — skipping");
      return StatusCode::SUCCESS;
    }

    // construct the ACTS simulator
    // Magnetic field
    m_bField = std::make_shared<ATLASMagneticFieldWrapper>();
    // Navigator (needs tracking geometry ready)
    m_navigator = std::make_unique<Navigator>(Navigator::Config{ m_trackingGeometry }, m_logger);
    // Steppers
    ChargedStepper chargedStepper(m_bField);
    NeutralStepper neutralStepper;
    // Propagators
    ChargedPropagator chargedPropagator(std::move(chargedStepper), *m_navigator, m_logger);
    NeutralPropagator neutralPropagator(std::move(neutralStepper), *m_navigator, m_logger);
    // Single-particle simulations

    // Single particle simulations 
    ChargedSimulation simCharged(std::move(chargedPropagator), m_logger);
    NeutralSimulation simNeutral(std::move(neutralPropagator), m_logger);

    // construct the ACTS simulator/dispatcher
    m_simulator = std::make_unique<Simulation>(std::move(simCharged), std::move(simNeutral));

    // Acts propagater options (charged) particles
    m_simulator->charged.maxStepSize             = m_maxStepSize;
    m_simulator->charged.maxStep                 = m_maxStep;
    m_simulator->charged.pathLimit               = m_pathLimit;
    m_simulator->charged.maxRungeKuttaStepTrials = m_maxRungeKuttaStepTrials;
    m_simulator->charged.loopProtection          = m_loopProtection;
    m_simulator->charged.loopFraction            = m_loopFraction;
    m_simulator->charged.targetTolerance         = m_tolerance;
    m_simulator->charged.stepSizeCutOff          = m_stepSizeCutOff;

    // Create interaction list
    ATH_MSG_VERBOSE(name() << " Min pT for interaction " << m_interact_minPt * Acts::UnitConstants::MeV << " MeV");
    m_simulator->charged.interactions = ActsFatras::makeStandardChargedElectroMagneticInteractions(m_interact_minPt * Acts::UnitConstants::MeV);

    ATH_MSG_VERBOSE("Simulator configured:");
    ATH_MSG_VERBOSE("  maxStepSize = " << m_maxStepSize);
    ATH_MSG_VERBOSE("  maxStep     = " << m_maxStep);
    ATH_MSG_INFO("Fatras simulator configured successfully.");

    return StatusCode::SUCCESS;
}

StatusCode ActsFatrasG4Tool::initializePhysics(){
  ATH_MSG_INFO("ActsFatrasG4Tool::initializePhysics() called");
  // make simulator
  ATH_CHECK( configureSimulator());
  return StatusCode::SUCCESS;
}


std::vector<ActsFatras::Particle> ActsFatrasG4Tool::buildActsInputFromG4(const G4Track& track)
{
  TrackHelper helper(&track);

  int barcode = helper.GetBarcode();
  if (barcode == 0){
    barcode = track.GetTrackID();
    ATH_MSG_DEBUG("Track barcode is 0, using TrackID " << barcode
                    << " PDG=" << track.GetDefinition()->GetPDGEncoding()
                    << " Ekin=" << track.GetKineticEnergy()
                  );
  }

  ActsFatras::Barcode fatrasBarcode = ActsFatras::Barcode().withVertexPrimary(0).withParticle(barcode);
  auto fatrasPDG      = static_cast<Acts::PdgParticle>(track.GetDefinition()->GetPDGEncoding());
  double fatrasCharge = track.GetDefinition()->GetPDGCharge();
  double fatrasMass   = track.GetDefinition()->GetPDGMass(); // already MeV

  ActsFatras::Particle particle(fatrasBarcode, fatrasPDG, fatrasCharge, fatrasMass);

  // ---- Direction magnitude ----
  particle.setDirection(track.GetMomentum().x(), track.GetMomentum().y(), track.GetMomentum().z());

  // ---- Momentum magnitude ----
  double momentumMeV = track.GetMomentum().mag();  // already MeV
  particle.setAbsoluteMomentum(momentumMeV);

  // ---- Unit direction ----
  G4ThreeVector momDir = track.GetMomentumDirection(); // already unit
  particle.setDirection(momDir.x(), momDir.y(), momDir.z());

  // ---- 4-Position (mm, ns already) ----
  particle.setPosition4(Acts::Vector4(
      track.GetPosition().x(),
      track.GetPosition().y(),
      track.GetPosition().z(),
      track.GetGlobalTime()));

  ATH_MSG_DEBUG("Converted G4->Acts:"
      << " p(MeV)=" << momentumMeV
      << " mass(MeV)=" << fatrasMass);

    return { particle };
}

void ActsFatrasG4Tool::handleSimulationFailures(const Acts::Result<ActsFatras::SingleParticleSimulationResult>& result)
{
  if (!result.ok()) {
    ATH_MSG_ERROR("Fatras simulation failed: " << result.error().message());
  }
}

StatusCode ActsFatrasG4Tool::applyPrimaryUpdate(
    const G4Track&,
    const std::vector<ActsFatras::Particle>& simulatedFinalParticles,
    G4FastStep& fastStep)
{
  if (simulatedFinalParticles.empty()){return StatusCode::SUCCESS;}

  const auto& primary = simulatedFinalParticles.front();

  if (!primary.isAlive()) { // if not alive, kill the track in G4
    fastStep.KillPrimaryTrack();
    return StatusCode::SUCCESS;
  }

  double p = primary.absoluteMomentum(); // MeV
  double m = primary.mass();             // MeV

  double energy = std::sqrt(p*p + m*m); // total energy in MeV
  double kinetic = (energy - m) * CLHEP::MeV; // convert back to kinetic energy in MeV for G4

  fastStep.ProposePrimaryTrackFinalKineticEnergy(kinetic);

  fastStep.ProposePrimaryTrackFinalMomentumDirection(
    G4ThreeVector(primary.direction().x(), primary.direction().y(), primary.direction().z()).unit()
  );

  fastStep.ProposePrimaryTrackFinalPosition(
    G4ThreeVector(primary.position().x(), primary.position().y(), primary.position().z())
  );
  return StatusCode::SUCCESS;
}

StatusCode ActsFatrasG4Tool::spawnSecondaries(
    const std::vector<ActsFatras::Particle>& simulatedFinal,
    const G4Track&,
    G4FastStep& fastStep)
{
    for (size_t i = 1; i < simulatedFinal.size(); ++i) {
      const auto& sec = simulatedFinal[i];

      G4ParticleDefinition* def = G4ParticleTable::GetParticleTable()->FindParticle(sec.pdg());

      if (!def){continue;}

      G4ThreeVector dir(sec.direction().x(), sec.direction().y(), sec.direction().z());
      dir = dir.unit();
      double momentumMeV = sec.absoluteMomentum();

      G4DynamicParticle dyn(def, dir, momentumMeV);
      fastStep.CreateSecondaryTrack(
          dyn,
          G4ThreeVector(sec.position().x(),
                        sec.position().y(),
                        sec.position().z()),
          sec.time(),
          true   // is local time
      );
    }
    return StatusCode::SUCCESS;
}

void ActsFatrasG4Tool::debugTrackRegion(const G4FastTrack& fastTrack) const
{
    const G4Track& track = *fastTrack.GetPrimaryTrack();

    ATH_MSG_DEBUG("=========== FATRAS REGION DEBUG ===========");
    // FastSim envelope solid
    const G4VSolid* envelope = fastTrack.GetEnvelopeSolid();
    if (envelope) {ATH_MSG_DEBUG("FastSim envelope solid: " << envelope->GetName());} 
    else {ATH_MSG_DEBUG("FastSim envelope solid: NONE");}

    // Physical volume
    const G4VPhysicalVolume* pv = track.GetVolume();
    if (pv) {
        ATH_MSG_DEBUG("Physical volume: " << pv->GetName());
        const G4LogicalVolume* lv = pv->GetLogicalVolume();
        if (lv) {
            const G4Region* region = lv->GetRegion();
            if (region) {ATH_MSG_DEBUG("Region: " << region->GetName());} 
            else {ATH_MSG_DEBUG("Region: NONE");}
        }
    } else {
        ATH_MSG_DEBUG("Physical volume: NONE");
    }

    const G4ThreeVector& pos = track.GetPosition();
    double r = std::sqrt(pos.x()*pos.x() + pos.y()*pos.y());

    ATH_MSG_DEBUG("Position (mm): (" << pos.x()/CLHEP::mm << ", " << pos.y()/CLHEP::mm << ", " << pos.z()/CLHEP::mm << ")");
    ATH_MSG_DEBUG("R (mm): " << r / CLHEP::mm);
    ATH_MSG_DEBUG("Ekin (MeV): " << track.GetKineticEnergy()/CLHEP::MeV);
    ATH_MSG_DEBUG("PDG: " << track.GetDefinition()->GetPDGEncoding());
    ATH_MSG_DEBUG("===========================================");
}

StatusCode ActsFatrasG4Tool::runDebugInjection(
    const EventContext& ctx,
    const G4Track& track,
    const Acts::GeometryContext& anygctx,
    const Acts::MagneticFieldContext& mctx,
    Generator& generator,
    G4FastStep& fastStep)
{
  // NOTE: block below is only meant for DEBUGGING, 
  // Retained here for future debugging purposes
  // -- It injects a muon particle in InDet, 
  // --- to test if FATRAS properly simulates it and produces/writes expected hits. 
  // DO NOT USE in actual production
  // PLEASE set m_debugInject in header or jobOption config to false for actual production use.

  ATH_MSG_DEBUG("### DEBUG INJECTION ACTIVE ###");

  // --- Build fake muon ---
  ActsFatras::Barcode bc = ActsFatras::Barcode().withVertexPrimary(0).withParticle(1);

  Acts::PdgParticle pdg = Acts::PdgParticle(13);  // mu-
  double charge = -1.0;
  double mass   = 105.7; // MeV

  ActsFatras::Particle fakeMuon(bc, pdg, charge, mass);

  fakeMuon.setPosition4({50., 0., 0., 0.});
  fakeMuon.setDirection(0., 1., 0.);
  fakeMuon.setAbsoluteMomentum(10000.0); // 10 GeV

  // create input particle with ActsFatras::Particle 
  std::vector<ActsFatras::Particle> inputParticle{ fakeMuon };
  // declare variables to be passed
  std::vector<ActsFatras::Particle> simulatedInitialParticles;
  std::vector<ActsFatras::Particle> simulatedFinalParticles;
  std::vector<ActsFatras::Hit> hits;

  if (!m_simulator) {
    ATH_MSG_ERROR("Simulator not configured");
    return StatusCode::FAILURE;
  }

  auto result = m_simulator->simulate(anygctx, mctx, generator, inputParticle, simulatedInitialParticles, simulatedFinalParticles, hits);

  if (!result.ok()) {
    ATH_MSG_ERROR("Fatras debug injection failed: " << result.error().message());
    fastStep.KillPrimaryTrack();
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Injected muon hits produced: " << hits.size());

  if (createHitsFromG4(ctx, track, *m_trackingGeometry, hits).isFailure()) {
    ATH_MSG_ERROR("Failed to write debug injected hits");
    fastStep.KillPrimaryTrack();
    return StatusCode::FAILURE;
  }

  fastStep.KillPrimaryTrack();
  // Do not call here below methods, only for simple injection:
  //   applyPrimaryUpdate()
  //   spawnSecondaries()
  //   buildActsInputFromG4()

  ATH_MSG_DEBUG("===================================");

  return StatusCode::SUCCESS;
}

void ActsFatrasG4Tool::simulateFatrasTrack(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  // Get primary track
  const G4Track& track = *fastTrack.GetPrimaryTrack();
  TrackHelper helper(&track);

  int trackID  = track.GetTrackID();
  int parentID = track.GetParentID();
  int barcode  = helper.GetBarcode();
  int pdg      = track.GetDefinition()->GetPDGEncoding();
  double ekin  = track.GetKineticEnergy() / CLHEP::MeV;

  ATH_MSG_DEBUG("==== G4 Track entering Fatras ====");
  ATH_MSG_DEBUG("TrackID  = " << trackID);
  ATH_MSG_DEBUG("ParentID = " << parentID);
  ATH_MSG_DEBUG("Barcode  = " << barcode);
  ATH_MSG_DEBUG("PDG      = " << pdg);
  ATH_MSG_DEBUG("Ekin(MeV)= " << ekin);
  ATH_MSG_DEBUG("===================================");

  // get EventContext
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // random seeds
  m_randomEngine->setSeed(m_randomEngineName, ctx);
  CLHEP::HepRandomEngine* randomEngine = m_randomEngine->getEngine(ctx);

  // RNG
  Generator generator(CLHEP::RandFlat::shoot(randomEngine->flat()));

  // get Mag field context, and Geo context
  ATH_MSG_VERBOSE(name() << " Getting per event Geo and Mag map");
  auto mctx = m_ctxProvider.getMagneticFieldContext(ctx);
  auto anygctx = m_ctxProvider.getGeometryContext(ctx);

  // Build Acts input from G4 track
  std::vector<ActsFatras::Particle> inputParticle;

  // Skip very low energy particles (e.g 1MeV)
  if (ekin < 1.0) { // in MeV
    ATH_MSG_DEBUG("Skipping and killing low energy track");
    fastStep.KillPrimaryTrack();
    return;
  }

  if (msgLvl(MSG::DEBUG)){ATH_MSG_DEBUG("m_debugInject runtime value = " << m_debugInject);}  
  if (m_debugInject) {
    // run the debug injection
    if (runDebugInjection(ctx, track, anygctx, mctx, generator, fastStep).isFailure()){
      ATH_MSG_DEBUG("runDebugInjection failed");
    }
    return; // return so we escape scope after run, and so we don't enter production loop below
  } else {
    // Actual production code: build input from G4 track
    inputParticle = buildActsInputFromG4(track);
  }

  // safeguard against geometry pointer returned null or undefined (usually with being outside defined geometry)
  Acts::Vector3 startPos(track.GetPosition().x(), track.GetPosition().y(), track.GetPosition().z());
  auto startVolume = m_trackingGeometry->resolveLowestTrackingVolume(anygctx, startPos);

  if(not startVolume.ok() or *startVolume == nullptr){
    ATH_MSG_DEBUG("Could not resolve the lowest tracking volume, skip FATRAS.");
    return;
  }

  // debug region
  if (msgLvl(MSG::DEBUG)) {
    const G4VSolid* envelope = fastTrack.GetEnvelopeSolid();
    if (envelope) {ATH_MSG_DEBUG("FastSim envelope solid name: "  << envelope->GetName());}
    const G4VPhysicalVolume* pv = track.GetVolume();
    if (pv) {
        const G4LogicalVolume* lv = pv->GetLogicalVolume();
        if (lv) {
            const G4Region* region = lv->GetRegion();
            if (region) {
                ATH_MSG_DEBUG("Region name: " << region->GetName());
            }
        }
    }
  }

  // declare variables to be passed
  std::vector<ActsFatras::Particle> simulatedInitialParticles;
  std::vector<ActsFatras::Particle> simulatedFinalParticles;
  std::vector<ActsFatras::Hit> hits;

  // safety: returns early if m_simulator not called in initializePhysics, or if failed for some reason
  if (!m_simulator) {
    ATH_MSG_ERROR("Simulator not configured");
    return;
  }

  // do the actual Fatras simulation  
  auto result = m_simulator->simulate(anygctx, mctx, generator, inputParticle, simulatedInitialParticles, simulatedFinalParticles, hits);
  if (!result.ok()) {
    const std::string& msg = result.error().message();
      // volume error
      if (msg.find("No Volume") != std::string::npos) {
        const G4VSolid* envelope = fastTrack.GetEnvelopeSolid();
        const G4ThreeVector& pos = track.GetPosition();
        double r = std::sqrt(pos.x()*pos.x() + pos.y()*pos.y());
        auto inside = envelope->Inside(pos);
        ATH_MSG_ERROR("G4 envelope Inside() = " << inside << ", " 
                                        << "(Position mm, R mm) = ((" 
                                        << pos.x()/CLHEP::mm << ", " 
                                        << pos.y()/CLHEP::mm << ", " 
                                        << pos.z()/CLHEP::mm << "), " 
                                        << r/CLHEP::mm       << ")"
                      );
        ATH_MSG_ERROR("ActsFatras simulation failed with 'No Volume' error: " << msg);
        if (msgLvl(MSG::DEBUG)) {
          const G4ThreeVector& mom = track.GetMomentumDirection();
          ATH_MSG_DEBUG("=== FATRAS NO VOLUME DEBUG ===");
          ATH_MSG_DEBUG("PDG      = " << track.GetDefinition()->GetPDGEncoding());
          ATH_MSG_DEBUG("Ekin MeV = " << track.GetKineticEnergy() / CLHEP::MeV);
          ATH_MSG_DEBUG("Position mm = (" << pos.x()/CLHEP::mm << ", " << pos.y()/CLHEP::mm << ", " << pos.z()/CLHEP::mm << ")");
          ATH_MSG_DEBUG("R (mm)   = " << r / CLHEP::mm);
          ATH_MSG_DEBUG("Z (mm)   = " << pos.z() / CLHEP::mm);
          ATH_MSG_DEBUG("Direction = (" << mom.x() << ", " << mom.y() << ", " << mom.z() << ")");
          ATH_MSG_DEBUG("==============================");          
        }
        ATH_MSG_DEBUG("Track outside Acts tracking geometry -> let G4 continue.");
        return;   // do not kill track, let to G4
      }
      // other kinds of error 
      else {
        // real fatal error
        ATH_MSG_ERROR("ActsFatras simulation failed: " << msg);
        fastStep.KillPrimaryTrack();
        return;
      }
  }

  // =========================================================
  // ======= FATRAS DEBUG PRINTING ===========================
  // =========================================================
  if (msgLvl(MSG::DEBUG)) {
    if (!simulatedInitialParticles.empty()) {
      const auto& initial = simulatedInitialParticles.front();
      double totalEdep = 0.;
      for (const auto& h : hits) {totalEdep += h.depositedEnergy() / Acts::UnitConstants::MeV;}
      ATH_MSG_DEBUG("===============================================");
      ATH_MSG_DEBUG("FATRAS TRACK DEBUG");
      ATH_MSG_DEBUG("PDG: " << static_cast<int>(initial.pdg()));
      ATH_MSG_DEBUG("Initial momentum [MeV]: " << initial.absoluteMomentum());
      ATH_MSG_DEBUG("Initial mass     [MeV]: " << initial.mass());
      ATH_MSG_DEBUG("Hits produced: " << hits.size());
      ATH_MSG_DEBUG("Final particles: " << simulatedFinalParticles.size());
      ATH_MSG_DEBUG("Total deposited energy [MeV]: " << totalEdep);
      ATH_MSG_DEBUG("===============================================");
    }

    // some sanity printing, prints simulatedInitialParticles[0] if it exists, and the number of hits
    if (!simulatedInitialParticles.empty()) {
      ATH_MSG_DEBUG(name() << " initial particle " << simulatedInitialParticles.front());
    }
    ATH_MSG_DEBUG(name() << " ActsFatras simulator hits: " << hits.size());
  }

  // Actual part to create hits: fill MT cache only of non-zero detector hits
  if (!hits.empty()) {
    if (createHitsFromG4(ctx, track, *m_trackingGeometry, hits).isFailure()) {
      ATH_MSG_ERROR("Failed to create hits");
      return;
    }
 }

  // Apply propagation result to G4
  if (applyPrimaryUpdate(track, simulatedFinalParticles, fastStep).isFailure()) {
    ATH_MSG_ERROR("Failed to apply primary update");
    return;
  }

  // Spawn secondaries in Geant4
  if (spawnSecondaries(simulatedFinalParticles, track, fastStep).isFailure()) {
    ATH_MSG_ERROR("Failed to spawn secondaries");
    return;
  }
}

// ====================================
// related to the hits  caches / saving
// ====================================
const std::vector<SiHit>& ActsFatrasG4Tool::getPixelHitsCache(const EventContext& ctx) const
{
  const EventCache* cache = m_eventCache.get(ctx);
  static const std::vector<SiHit> empty;
  if (!cache) { return empty; }
  return cache->pixelHits;
}

const std::vector<SiHit>& ActsFatrasG4Tool::getSCTHitsCache(const EventContext& ctx) const
{
  const EventCache* cache = m_eventCache.get(ctx);
  static const std::vector<SiHit> empty;
  if (!cache) { return empty; }
  return cache->sctHits;
}

void ActsFatrasG4Tool::clearCaches(const EventContext& ctx) const
{
  EventCache* cache = m_eventCache.get(ctx);
  if (!cache) { return; }
  cache->pixelHits.clear();
  cache->sctHits.clear();
}

ActsFatrasG4Tool::EventCache& ActsFatrasG4Tool::getCache(const EventContext& ctx) const {
  return *m_eventCache.get(ctx);
}
// ====================================

StatusCode ActsFatrasG4Tool::createHitsFromG4(
    const EventContext& ctx,
    const G4Track& track,
    const Acts::TrackingGeometry& trackingGeometry,
    const std::vector<ActsFatras::Hit>& hits
) const
{
  if (hits.empty()) {
    return StatusCode::SUCCESS;
  }

  // ============================================
  // Get per-slot event cache (MT safe)
  // ============================================
  EventCache& evtCache = getCache(ctx);

  // ============================================
  // Truth handling via TrackHelper (from G4Track case)
  // ============================================
  TrackHelper helper(&track);
  int barcode = helper.GetBarcode();

  // initialize partLink
  HepMcParticleLink partLink(HepMC::UNDEFINED_ID, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);

  if (barcode != 0 && barcode != HepMC::UNDEFINED_ID && barcode != HepMC::INVALID_PARTICLE_ID) {
    partLink = HepMcParticleLink(barcode, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
  }

  // ============================================
  // Loop over Fatras hits
  // ============================================
  for (const auto& hit : hits) {

    double energyDeposit = hit.depositedEnergy() / Acts::UnitConstants::MeV;
    double time          = ActsTrk::timeToAthena(hit.time());
    auto   geoID         = hit.geometryId();

    try {

      auto acts_surface = trackingGeometry.findSurface(geoID);
      if (!acts_surface) continue;

      const auto *acts_de = getActsDetectorElement(acts_surface);
      if (!acts_de) continue;

      const Trk::Surface& atlasSurface = acts_de->atlasSurface();
      Identifier hitId = atlasSurface.associatedDetectorElementIdentifier();

      const Trk::TrkDetElementBase* detBase = atlasSurface.associatedDetectorElement();
      const InDetDD::SiDetectorElement* siDet = dynamic_cast<const InDetDD::SiDetectorElement*>(detBase);

      if (!siDet) continue;

      // ============================================
      // Global -> local intersection
      // ============================================
      auto intersection = atlasSurface.globalToLocal(hit.position());
      if (!intersection) continue;

      double interX = (*intersection)(0);
      double interY = (*intersection)(1);
      double thickness = siDet->thickness();

      // ============================================
      // Direction handling, compute entry/exit positions
      // ============================================
      // get hit direction 
      const auto& actsDir = hit.direction();
      // Convert to Amg only when applying transform
      Amg::Vector3D amgDir(actsDir.x(), actsDir.y(), actsDir.z());

      // ATLAS surface transform using Amg
      const Amg::Transform3D surfaceTransform = atlasSurface.transform();
      const Amg::Transform3D invSurfaceTransform = surfaceTransform.inverse();
      Amg::Vector3D localDirAmg = invSurfaceTransform.linear() * amgDir;

      // calculate cosTheta using norm
      double norm = localDirAmg.norm();
      if (norm < 1e-9) continue;
      double cosTheta = localDirAmg.z() / norm;
      if (std::abs(cosTheta) < 1e-6) continue;

      // calculate local entry and exit in X and Y based on intersection and distance
      localDirAmg *= thickness / std::abs(cosTheta);

      int movingDir = localDirAmg.z() > 0. ? 1 : -1;

      double distX = localDirAmg.x();
      double distY = localDirAmg.y();
      
      double localEntryX = interX - 0.5 * distX;
      double localEntryY = interY - 0.5 * distY;
      double localExitX  = interX + 0.5 * distX;
      double localExitY  = interY + 0.5 * distY;

      // calculate hit transform with silicon detector using Amg
      const Amg::Transform3D& hitTransform = siDet->transformHit().inverse();
      // create entry and exit surfaces
      Amg::Vector3D surfaceEntry(localEntryX, localEntryY, -0.5 * movingDir * thickness);
      Amg::Vector3D surfaceExit(localExitX, localExitY, 0.5 * movingDir * thickness);

      Amg::Vector3D globalEntry = surfaceTransform * surfaceEntry;
      Amg::Vector3D globalExit  = surfaceTransform * surfaceExit;
      Amg::Vector3D localEntry  = hitTransform * globalEntry;
      Amg::Vector3D localExit   = hitTransform * globalExit;

      HepGeom::Point3D<double> entryHep(localEntry.x(), localEntry.y(), localEntry.z());
      HepGeom::Point3D<double> exitHep(localExit.x(), localExit.y(), localExit.z());

      bool isPixel = siDet->isPixel();

      // ============================================
      // Create SiHit
      // ============================================
      SiHit siHit(
          entryHep,
          exitHep,
          energyDeposit,
          time,
          partLink,
          isPixel ? 0 : 1,
          isPixel ? m_pixIdHelper->barrel_ec(hitId) : m_sctIdHelper->barrel_ec(hitId),
          isPixel ? m_pixIdHelper->layer_disk(hitId) : m_sctIdHelper->layer_disk(hitId),
          isPixel ? m_pixIdHelper->eta_module(hitId) : m_sctIdHelper->eta_module(hitId),
          isPixel ? m_pixIdHelper->phi_module(hitId) : m_sctIdHelper->phi_module(hitId),
          isPixel ? 0 : m_sctIdHelper->side(hitId)
        );

      // ============================================
      // Store into MT-safe cache
      // ============================================
      if (isPixel) {
        evtCache.pixelHits.emplace_back(std::move(siHit));
      } else {
        evtCache.sctHits.emplace_back(std::move(siHit));
      }
      ATH_MSG_VERBOSE(name() << " convert and store 1 hit, total " << evtCache.pixelHits.size() << " Pixel | " << evtCache.sctHits.size()   << " SCT hits stored.");
    }
    catch (const std::exception& e) {
      ATH_MSG_DEBUG(name() << "Can not find Acts Surface (" << e.what() << ")...Skip...");
      continue;
    }
  }
  ATH_MSG_VERBOSE("Cache now contains " << evtCache.pixelHits.size() << " pixel hits and " << evtCache.sctHits.size()   << " SCT hits");
  return StatusCode::SUCCESS;
}

StatusCode ActsFatrasG4Tool::finalize()
{
  ATH_MSG_DEBUG( "[ActsFatrasG4Tool] finalize() starting" );
  ATH_MSG_DEBUG( "[ActsFatrasG4Tool] finalize() successful" );
  return StatusCode::SUCCESS;
}
