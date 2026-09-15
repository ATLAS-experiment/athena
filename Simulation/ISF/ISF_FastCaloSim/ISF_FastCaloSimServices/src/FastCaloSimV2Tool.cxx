/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "FastCaloSimV2Tool.h"


// FastCaloSim includes
#include "FastCaloSim/Core/TFCSParametrizationBase.h"
#include "FastCaloSim/Core/TFCSSimulationState.h"
#include "FastCaloSim/Core/TFCSTruthState.h"
#include "FastCaloSim/Core/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"

// Geant4 track construction
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
#include "G4Track.hh"
#include "G4FieldTrack.hh"
#include "G4ThreeVector.hh"
#include <memory>
#include <vector>

#include "AthenaKernel/RNGWrapper.h"

//!
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictParser/IdDictParser.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/TileID.h"
//!

// StoreGate
#include "StoreGate/StoreGateSvc.h"

#include "ISF_Event/ISFTruthIncident.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArReadoutGeometry/FCALDetectorManager.h"

#include "PathResolver/PathResolver.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "TFile.h"
#include <fstream>

using std::abs;
using std::atan2;

namespace {
  /// Build a Geant4 track from a FastCaloSim truth state.
  std::unique_ptr<G4Track> makeG4Track(const TFCSTruthState& truth) {
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    if (!particleTable) return nullptr;
    G4ParticleDefinition* particle = particleTable->FindParticle(truth.pdgid());
    if (!particle) return nullptr;
    const G4ThreeVector momentum(truth.Px(), truth.Py(), truth.Pz());
    auto* dynamicParticle = new G4DynamicParticle(particle, momentum);
    const G4ThreeVector position(truth.vertex().X(), truth.vertex().Y(),
                                 truth.vertex().Z());
    return std::make_unique<G4Track>(dynamicParticle, /*time=*/0.0, position);
  }
}

/** Constructor **/
ISF::FastCaloSimV2Tool::FastCaloSimV2Tool( const std::string& type, const std::string& name,  const IInterface* parent)
  : BaseSimulatorTool(type, name, parent)
{
}

/** framework methods */
StatusCode ISF::FastCaloSimV2Tool::initialize()
{
  ATH_CHECK( BaseSimulatorTool::initialize() );

  ATH_CHECK( m_caloCellMakerToolsSetup.retrieve() );
  ATH_MSG_DEBUG( "Successfully retrieve CaloCellMakerTools: " << m_caloCellMakerToolsSetup );
  ATH_CHECK( m_caloCellMakerToolsRelease.retrieve() );

  ATH_CHECK(m_rndmGenSvc.retrieve());

  m_doPunchThrough = not m_punchThroughTool.empty();
  if (m_doPunchThrough) {
    ATH_CHECK(m_punchThroughTool.retrieve());
  }

  ATH_CHECK(m_truthRecordSvc.retrieve());

  ATH_CHECK(m_FastCaloSimParametrizationTool.retrieve());

  // Workers cannot create the shared transport world.
  ATH_CHECK(m_FastCaloSimParametrizationTool->initializeTransportGeometry());

  ATH_CHECK( detStore()->retrieve(m_caloCellID, "CaloCell_ID") );

  // Output data handle
  ATH_CHECK( m_caloCellKey.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode ISF::FastCaloSimV2Tool::setupEventST()
{
  ATH_MSG_DEBUG ("setupEventST");

  m_theContainer = std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS);
  m_theContainerPtr = m_theContainer.get();
  ATH_CHECK(evtStore()->record(std::move(m_theContainer), m_caloCellsOutputName));
  // NB: m_theContainer is now nullptr
  const EventContext& ctx = Gaudi::Hive::currentContext();
  return this->commonSetup(ctx);
}

StatusCode ISF::FastCaloSimV2Tool::setupEvent(const EventContext& ctx)
{
  ATH_MSG_DEBUG ("setupEvent");

  m_theContainer = std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS);
  m_theContainerPtr = m_theContainer.get();
  return this->commonSetup(ctx);
}

StatusCode ISF::FastCaloSimV2Tool::commonSetup(const EventContext& ctx)
{
  // Set the RNGs to use for this event. We need to reset it for MT jobs
  // because of the mismatch between Gaudi slot-local and G4 thread-local RNG.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this, m_randomEngineName);
  rngWrapper->setSeed( m_randomEngineName, ctx );

  for (const ToolHandle<ICaloCellMakerTool>& tool : m_caloCellMakerToolsSetup)
    {
      std::string chronoName=this->name()+"_"+ tool.name();
      if (m_chrono) m_chrono->chronoStart(chronoName);
      StatusCode sc = tool->process(m_theContainerPtr, ctx);
      if (m_chrono) {
        m_chrono->chronoStop(chronoName);
        ATH_MSG_DEBUG( "Chrono stop : delta " << m_chrono->chronoDelta (chronoName,IChronoStatSvc::USER) * CLHEP::microsecond / CLHEP::second << " second " );
      }

      if (sc.isFailure())
        {
          ATH_MSG_ERROR( "Error executing tool " << tool.name() );
          return StatusCode::FAILURE;
        }
    }

  return StatusCode::SUCCESS;
}

StatusCode ISF::FastCaloSimV2Tool::releaseEvent(const EventContext& ctx)
{
  ATH_MSG_VERBOSE( "FastCaloSimV2Tool " << name() << " releaseEvent() " );
  // Run the version of releaseEvent that returns the output collection
  // Run the normal method
  ATH_CHECK(this->releaseEventST());
  if ( m_theContainer ) {

    // Record with WriteHandle
    SG::WriteHandle< CaloCellContainer > caloCellHandle( m_caloCellKey, ctx );
    ATH_CHECK( caloCellHandle.record( std::move( m_theContainer ) ) );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode ISF::FastCaloSimV2Tool::releaseEventST()
{
  ATH_MSG_VERBOSE("release Event");
  const EventContext& ctx = Gaudi::Hive::currentContext();

  //run release tools in a loop
  for (const ToolHandle<ICaloCellMakerTool>& tool : m_caloCellMakerToolsRelease)
    {
      ATH_MSG_VERBOSE( "Calling tool " << tool.name() );

      ATH_CHECK(tool->process(m_theContainerPtr, ctx));
    }

  return StatusCode::SUCCESS;

}

/** Simulation Call */
StatusCode ISF::FastCaloSimV2Tool::simulate(const EventContext& ctx, ISF::ISFParticle& isfp, ISFParticleContainer& secondaries, McEventCollection*)
{

  ATH_MSG_VERBOSE("NEW PARTICLE! FastCaloSimV2Tool called with ISFParticle: " << isfp);

  Amg::Vector3D particle_position =  isfp.position();
  Amg::Vector3D particle_direction(isfp.momentum().x(),isfp.momentum().y(),isfp.momentum().z());

  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this, m_randomEngineName);

  //Don't simulate particles with total energy below 10 MeV
  if(isfp.ekin() < 10) {
    ATH_MSG_VERBOSE("Skipping particle with Ekin: " << isfp.ekin() <<" MeV. Below the 10 MeV threshold.");
    return StatusCode::SUCCESS;
  }

  TFCSTruthState truth;
  truth.SetPtEtaPhiM(particle_direction.perp(), particle_direction.eta(), particle_direction.phi(), isfp.mass());
  truth.set_pdgid(isfp.pdgCode());
  truth.set_vertex(particle_position[Amg::x], particle_position[Amg::y], particle_position[Amg::z]);

  /// for anti protons and anti neutrons the kinetic energy should be
  /// calculated as Ekin = E() + M() instead of E() - M()
  /// this is achieved by setting an Ekin offset of 2*M() to the truth state
  if(isfp.pdgCode() == -2212 || isfp.pdgCode() == -2112) {
    truth.set_Ekin_off(2*isfp.mass());
    ATH_MSG_VERBOSE("Found anti-proton/neutron, setting Ekin offset in TFCSTruthState.");
  }

  // Use the same transport and extrapolation as the Geant4 fast-sim model.
  std::unique_ptr<G4Track> g4track = makeG4Track(truth);
  if (!g4track) {
    ATH_MSG_WARNING("Could not build G4Track for pdgid " << isfp.pdgCode()
                    << ", skipping FastCaloSim simulation for this particle");
    return StatusCode::SUCCESS;
  }
  TFCSExtrapolationState extrapol;
  const std::vector<G4FieldTrack> caloSteps = m_FastCaloSimParametrizationTool->transport(*g4track);
  m_FastCaloSimParametrizationTool->extrapolate(extrapol, &truth, caloSteps);

  ATH_MSG_DEBUG(" particle: " << isfp.pdgCode() << " Ekin: " << isfp.ekin() << " position eta: " << particle_position.eta() << " direction eta: " << particle_direction.eta() << " position phi: " << particle_position.phi() << " direction phi: " << particle_direction.phi());

  //only simulate if extrapolation to calo surface succeeded
  if(extrapol.IDCaloBoundary_eta() != -999){
    CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);
    TFCSSimulationState simulstate(rndmEngine);

    // ATH_MSG_WARNING("Event number for this event: " << ctx.evt());
    simulstate.setAuxInfo<int>("EventNr"_FCShash, ctx.evt() );
    
    if (m_FastCaloSimParametrizationTool->simulate(simulstate, &truth, &extrapol) != FCSSuccess) {
      ATH_MSG_ERROR("FastCaloSim simulation call failed");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Energy returned: " << simulstate.E());
    ATH_MSG_VERBOSE("Energy fraction for layer: ");
    for (int s = 0; s < CaloCell_ID_FCS::MaxSample; s++)
    ATH_MSG_VERBOSE(" Sampling " << s << " energy " << simulstate.E(s));

    // FastCaloSim uses compact identifiers, not calorimeter cell hashes.
    for(const auto& iter : simulstate.cells()) {
      const IdentifierHash cellHash =
        m_caloCellID->calo_cell_hash(Identifier(iter.first));
      CaloCell* theCell =
        static_cast<CaloCell*>(m_theContainerPtr->findCell(cellHash));
      if (!theCell) {
        ATH_MSG_WARNING("Skipping simulated energy deposit in unknown cell id " << iter.first);
        continue;
      }
      theCell->addEnergy(iter.second);
    }

    //now perform punch through
    if (m_doPunchThrough) {
      int process = 201;
      // call punch-through simulation
      const ISF::ISFParticleVector *someSecondaries = m_punchThroughTool->computePunchThroughParticles(isfp, simulstate, rndmEngine);

      if (someSecondaries && !someSecondaries->empty()) {
        //Record truth incident for created punch through particles
        ISF::ISFTruthIncident truth( const_cast<ISF::ISFParticle&>(isfp),
                                     *someSecondaries,
                                     process,
                                     isfp.nextGeoID(),  // inherits from the parent
                                     ISF::fKillsPrimary);

        m_truthRecordSvc->registerTruthIncident( truth, true );
        // At this point we need to update the properties of the
        // ISFParticles produced in the interaction
        truth.updateChildParticleProperties();

        for (auto *secondary : *someSecondaries) {
          if (secondary->getTruthBinding()) {
            secondaries.push_back(secondary);
          }
          else {
            ATH_MSG_WARNING("Secondary particle created by PunchThroughTool not written out to truth.\n Parent (" << isfp << ")\n Secondary (" << *secondary <<")");
            delete secondary;
          }
        }
        delete someSecondaries;
      }
    }
    simulstate.DoAuxInfoCleanup();
  }
  else ATH_MSG_DEBUG("Skipping simulation as extrapolation to ID-Calo boundary failed.");


  return StatusCode::SUCCESS;

}
