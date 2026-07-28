/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimModel.h"

#include "FastCaloSim/Core/TFCSParametrizationBase.h"
#include "FastCaloSim/Core/TFCSSimulationState.h"
#include "FastCaloSim/Core/TFCSTruthState.h"
#include "FastCaloSim/Core/TFCSExtrapolationState.h"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"
#include "Randomize.hh"

#include "G4ParticleTable.hh"
#include "TruthUtils/HepMCHelpers.h"
#include "G4SDManager.hh"
#include "CaloCellContainerSD.h"

#include <cmath>
#include <cstdlib>

#undef FCS_DEBUG

FastCaloSimModel::FastCaloSimModel(const std::string& name,
                         G4Region* region,
                         const Gaudi::Property<std::string>& CaloCellContainerSDName,
                         const PublicToolHandle<IFastCaloSimParametrizationTool>& FastCaloSimParametrizationTool,
                         const PublicToolHandle<IPunchThroughSimWrapper>& PunchThroughSimWrapper,
                         const Gaudi::Property<bool>& doPunchThrough)

: G4VFastSimulationModel(name, region),
  m_CaloCellContainerSDName(CaloCellContainerSDName),
  m_FastCaloSimParametrizationTool(FastCaloSimParametrizationTool),
  m_PunchThroughSimWrapper(PunchThroughSimWrapper),
  m_doPunchThrough(doPunchThrough)
{
  // The transport world is shared, while each Geant4 thread needs its own
  // propagator. Both initialization calls are idempotent.
  if (m_FastCaloSimParametrizationTool->initializeTransportGeometry().isFailure()) {
    G4Exception("FastCaloSimModel", "FailedToInitializeTransportGeometry", FatalException,
                "FastCaloSimModel: Failed to initialize the particle-transport world volume.");
    std::abort();
  }
  if (m_FastCaloSimParametrizationTool->initializeTransportPropagator().isFailure()) {
    G4Exception("FastCaloSimModel", "FailedToInitializeTransportPropagator", FatalException,
                "FastCaloSimModel: Failed to initialize the particle-transport propagator.");
    std::abort();
  }
}

G4bool FastCaloSimModel::IsApplicable(const G4ParticleDefinition& particleType)
{
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();
  bool isElectron = &particleType == G4Electron::ElectronDefinition();
  bool isPositron = &particleType == G4Positron::PositronDefinition();
  bool isHadron   = MC::isHadron(particleType.GetPDGEncoding());

  bool isApplicable = isPhoton || isElectron || isPositron || isHadron;

  #ifdef FCS_DEBUG
    const std::string pName = particleType.GetParticleName();
    G4cout<< "[FastCaloSimModel::IsApplicable] Got " << pName <<G4endl;
    if(isApplicable) G4cout<<"[FastCaloSimModel::IsApplicable] APPLICABLE"<<G4endl;
    else G4cout<<"[FastCaloSimModel::IsApplicable] NOT APPLICABLE"<<G4endl;
  #endif


  return isApplicable;
}

G4bool FastCaloSimModel::ModelTrigger(const G4FastTrack& fastTrack)
{

  #ifdef FCS_DEBUG
    G4cout<<"[FastCaloSimModel::ModelTrigger] Got particle with "                                                      <<"\n"
                                    <<" pdg=" <<fastTrack.GetPrimaryTrack() -> GetDefinition()->GetPDGEncoding()  <<"\n"
                                    <<" Ekin="<<fastTrack.GetPrimaryTrack() -> GetKineticEnergy()                 <<"\n"
                                    <<" p="   <<fastTrack.GetPrimaryTrack() -> GetMomentum().mag()                <<"\n"
                                    <<" x="   <<fastTrack.GetPrimaryTrack() -> GetPosition().x()                  <<"\n"
                                    <<" y="   <<fastTrack.GetPrimaryTrack() -> GetPosition().y()                  <<"\n"
                                    <<" z="   <<fastTrack.GetPrimaryTrack() -> GetPosition().z()                  <<"\n"
                                    <<" r="   <<fastTrack.GetPrimaryTrack() -> GetPosition().perp()               <<"\n"
                                    <<" eta=" <<fastTrack.GetPrimaryTrack() -> GetMomentum().eta()                <<"\n"
                                    <<" phi=" <<fastTrack.GetPrimaryTrack() -> GetMomentum().phi()                <<"\n"
                                    <<G4endl;
  #endif


  // Match the 50 keV lower threshold used by the ISF implementation.
  if (fastTrack.GetPrimaryTrack() -> GetKineticEnergy() < 0.05) {
      #ifdef FCS_DEBUG
        G4cout<<"[FastCaloSimModel::ModelTrigger] Particle below 50 keV threshold. Passing to G4. "<<G4endl;
      #endif
    return false;
  }

  if (!passedIDCaloBoundary(fastTrack)) {
    #ifdef FCS_DEBUG
      G4cout<<"[FastCaloSimModel::ModelTrigger] Particle failed passedIDCaloBoundary z="<<fastTrack.GetPrimaryTrack() -> GetPosition().z()<<" r="<<fastTrack.GetPrimaryTrack() -> GetPosition().perp()<<G4endl;
    #endif
    return false;
  }

  float minEkinPions = 200;
  float minEkinOtherHadrons = 400;

  const G4ParticleDefinition * G4Particle = fastTrack.GetPrimaryTrack() -> GetDefinition();
  const float Ekin = fastTrack.GetPrimaryTrack() -> GetKineticEnergy();

  bool isPhoton    = G4Particle == G4Gamma::Definition();
  bool isElectron  = G4Particle == G4Electron::Definition();
  bool isPositron  = G4Particle == G4Positron::Definition();
  bool isPionPlus  = G4Particle == G4PionPlus::Definition();
  bool isPionMinus = G4Particle == G4PionMinus::Definition();

  if (isPhoton || isElectron || isPositron){
    #ifdef FCS_DEBUG
      G4cout<<"[FastCaloSimModel::ModelTrigger] Model triggered"<<G4endl;
    #endif
    return true;
  }

  if (isPionPlus || isPionMinus){
    bool passMinEkinPions = Ekin > minEkinPions;

    #ifdef FCS_DEBUG
      if(passMinEkinPions) G4cout<<"[FastCaloSimModel::ModelTrigger] Model triggered"<<G4endl;
      else G4cout<<"[FastCaloSimModel::ModelTrigger] Pion with Ekin="<<Ekin<<" below the minimum "<<minEkinPions<<" MeV threshold. Model not triggered."<<G4endl;
    #endif

    return passMinEkinPions;
  }

  bool passMinEkinOtherHadrons = Ekin > minEkinOtherHadrons;

  #ifdef FCS_DEBUG
    if(passMinEkinOtherHadrons) G4cout<<"[FastCaloSimModel::ModelTrigger] Model triggered"<<G4endl;
    else G4cout<<"[FastCaloSimModel::ModelTrigger] Other hadron with Ekin="<<Ekin<<" below the minimum "<<minEkinOtherHadrons<<" MeV threshold. Model not triggered."<<G4endl;
  #endif

  return passMinEkinOtherHadrons;
}

void FastCaloSimModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  // G4AtlasAlg/G4RunAlg seed Geant4's thread-local engine for each event.
  CLHEP::HepRandomEngine* rngEngine = G4Random::getTheEngine();
  TFCSSimulationState simState(rngEngine);
  TFCSTruthState truthState;
  TFCSExtrapolationState extrapolState;

  const G4Track* G4PrimaryTrack = fastTrack.GetPrimaryTrack();
  const G4ParticleDefinition* G4Particle = G4PrimaryTrack->GetDefinition();
  const int pdgID = G4Particle->GetPDGEncoding();

  // Do not simulate particles below 10 MeV
  if(G4PrimaryTrack->GetKineticEnergy() < 10){
    #ifdef FCS_DEBUG
      G4cout<<"[FastCaloSimModel::DoIt] Skipping particle with Ekin: " << G4PrimaryTrack->GetKineticEnergy() <<" MeV. Below the 10 MeV threshold"<<G4endl;
    #endif
    fastStep.KillPrimaryTrack();
    return;
  }
  if(G4Particle == G4Electron::Definition() ||  G4Particle == G4Positron::Definition() || G4Particle == G4Gamma::Definition())
  {
    truthState.set_pdgid(pdgID);
  }
  else{
    // Use the pion parametrization for other hadrons.
    truthState.set_pdgid(G4PionPlus::Definition()->GetPDGEncoding());
  }

  truthState.SetPtEtaPhiM(G4PrimaryTrack->GetMomentum().perp(),
                          G4PrimaryTrack->GetMomentum().eta(),
                          G4PrimaryTrack->GetMomentum().phi(),
                          G4Particle->GetPDGMass());

  truthState.set_vertex(G4PrimaryTrack->GetPosition().x(),
                        G4PrimaryTrack->GetPosition().y(),
                        G4PrimaryTrack->GetPosition().z());

  // Antinucleons use Ekin = E + M, represented by a 2M offset.
  if(pdgID == -2212 || pdgID == -2112) {
    truthState.set_Ekin_off(2 * G4Particle->GetPDGMass());
  }

  std::vector<G4FieldTrack> caloSteps =
    m_FastCaloSimParametrizationTool->transport(*G4PrimaryTrack);

  m_FastCaloSimParametrizationTool->extrapolate(
    extrapolState, &truthState, caloSteps);

  // Stop particles that could not be extrapolated to the ID-Calo boundary.
  if(extrapolState.IDCaloBoundary_eta() == -999){
    #ifdef FCS_DEBUG
      G4cout<<"[FastCaloSimModel::DoIt] Killing particle as extrapolation failed"<<G4endl;
    #endif
    fastStep.KillPrimaryTrack();
    return;
  }
  if(m_FastCaloSimParametrizationTool->simulate(simState, &truthState, &extrapolState) != FCSSuccess){
    G4Exception("FastCaloSimModel", "FailedSimulationCall", FatalException, "FastCaloSimModel: Simulation call failed.");
    std::abort();
  }

  #ifdef FCS_DEBUG
    G4cout<<"[FastCaloSimModel::DoIt] pdgID of G4PrimaryTrack: " << pdgID << G4endl;
    G4cout<<"[FastCaloSimModel::DoIt] Energy returned: " << simState.E() << G4endl;
    G4cout<<"[FastCaloSimModel::DoIt] Energy fraction for layer: " << G4endl;
    for (int s = 0; s < 24; s++) G4cout<<"[FastCaloSimModel::DoIt]   Sampling " << s << " energy " << simState.E(s) << G4endl;
  #endif

  CaloCellContainerSD* caloCellContainerSD = getCaloCellContainerSD();
  caloCellContainerSD->recordCells(simState);

  // Create punch-through secondaries after the calorimeter simulation.
  if (m_doPunchThrough){
    G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
    const double simE = simState.E();

    std::vector<double> simEfrac;
    simEfrac.reserve(24);
    for (unsigned int i = 0; i < 24; i++){simEfrac.push_back(simState.Efrac(i));}

    m_PunchThroughSimWrapper->DoPunchThroughSim(
      *ptable, rngEngine, simE, simEfrac, fastTrack, fastStep);
  }

  simState.DoAuxInfoCleanup();

  fastStep.KillPrimaryTrack();
  fastStep.SetPrimaryTrackPathLength(0.0);
}

CaloCellContainerSD* FastCaloSimModel::getCaloCellContainerSD(){
  G4SDManager* sdm = G4SDManager::GetSDMpointer();
  G4VSensitiveDetector* vsd =
    sdm->FindSensitiveDetector(m_CaloCellContainerSDName.value());

  if (!vsd){
    G4Exception("FastCaloSimModel", "FailedFindSensitiveDetector",
                FatalException,
                "Failed to find the configured CaloCellContainerSD.");
    std::abort();
  }
  CaloCellContainerSD* caloCellContainerSD =
    dynamic_cast<CaloCellContainerSD*>(vsd);

  if (!caloCellContainerSD){
    G4Exception("FastCaloSimModel", "FailedCastSensitiveDetector",
                FatalException,
                "The configured sensitive detector is not a CaloCellContainerSD.");
    std::abort();
  }
  return caloCellContainerSD;
}


G4bool FastCaloSimModel::passedIDCaloBoundary(const G4FastTrack& fastTrack){

  const float barrelR = 1148;
  const float barrelZ = 3549.5;
  const float innerBeamPipeR = 120;
  // Use the CALO::CALO boundary instead of the older AFII boundary.
  const float innerBeamPipeZ = 4587;
  const float outerBeamPipeR = 41;
  const float outerBeamPipeZ = 6783;

  const G4ThreeVector particlePosition =
    fastTrack.GetPrimaryTrack()->GetPosition();
  const float r = particlePosition.perp();
  const float z = particlePosition.z();
  const G4ThreeVector particleDirection =
    fastTrack.GetPrimaryTrack()->GetMomentum();
  // An outward step distinguishes ID particles from calorimeter backscatter.
  const G4ThreeVector helperLine = particlePosition + particleDirection;

  // Allow for Geant4 steps that cross the nominal boundary.
  const float triggerTolerance = 50;

  if (std::abs(z) <= barrelZ + triggerTolerance) {
    if (r >= barrelR && r < barrelR + triggerTolerance) return helperLine.perp() >= barrelR;
  }
  if (std::abs(z) >= barrelZ && std::abs(z) <= innerBeamPipeZ + triggerTolerance){
    if (r >= innerBeamPipeR && r < innerBeamPipeR + triggerTolerance) return helperLine.perp() >= innerBeamPipeR;
  }
  if (std::abs(z) >= innerBeamPipeZ && std::abs(z) <= outerBeamPipeZ){
    if (r >= outerBeamPipeR && r < outerBeamPipeR + triggerTolerance) return helperLine.perp() >= outerBeamPipeR;
  }
  if (r >= outerBeamPipeR && r<= innerBeamPipeR){
    if (std::abs(z) >= innerBeamPipeZ && std::abs(z) < innerBeamPipeZ + triggerTolerance) return std::abs(helperLine.z()) >= innerBeamPipeZ;
  }
  if (r >= innerBeamPipeR && r <= barrelR){
    if (std::abs(z) >= barrelZ && std::abs(z) < barrelZ + triggerTolerance) return std::abs(helperLine.z()) >= barrelZ;
  }

  return false;

}
