/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ForwardTransportModel.h"

// Athena includes
#include "ForwardTracker/Point.h"
#include "MCTruth/PrimaryParticleInformation.h"

#include "G4PrimaryParticle.hh"
#include "G4Proton.hh"
#include "G4Neutron.hh"
#include "G4Gamma.hh"
#include "G4Lambda.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"
#include "G4ParticleTable.hh"

#include "AtlasHepMC/GenEvent.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "ISF_Event/TruthBinding.h"
#include "ISF_Event/ISFParticle.h"

ForwardTransportModel::ForwardTransportModel(const std::string& name, const int verboseLevel, const std::string& FwdTrSvcName)
  : G4VFastSimulationModel(name)
  , m_fwdSvc(FwdTrSvcName, "ForwardTransportModel")
  , m_verboseLevel(verboseLevel)
{
  if (m_fwdSvc.retrieve().isFailure()) {
    G4ExceptionDescription description;
    description << "ForwardTransportModel::ForwardTransportModel Attempt to access ForwardTransportSvc failed.";
    G4Exception("ForwardTransportModel", "ForwardTransportModel01", FatalException, description);
    abort(); // to keep Coverity happy
  }

  m_fwdTrack.initialize(m_fwdSvc->getConfigData());

  if (m_verboseLevel>5) {
    G4cout << " transportFlag " << m_fwdSvc->getTransportFlag() << G4endl;
    G4cout << " etaCut        " << m_fwdSvc->getEtaCut() << G4endl;
    G4cout << " xiCut         " << m_fwdSvc->getXiCut() << G4endl;
  }
  return;
}


PrimaryParticleInformation* ForwardTransportModel::getPrimaryParticleInformation(const G4FastTrack& fastTrack) const
{
  const G4Track *track = fastTrack.GetPrimaryTrack();
  const G4DynamicParticle *dp = track->GetDynamicParticle();
  if (dp) {
    const G4PrimaryParticle *pp = dp->GetPrimaryParticle();
    if (pp) {
      // Extract the PrimaryParticleInformation
      return dynamic_cast<PrimaryParticleInformation*>
        ( pp->GetUserInformation() );
    }
  }
  return nullptr;
}


void ForwardTransportModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {
  // Depending on particle type and kinematics one can decide to kill the track,
  // modify it or change it into something else (e.g. a parameterised shower).
  if (m_verboseLevel>4) {
    G4cout <<"ForwardTransportModel::DoIt" << G4endl;
  }

  const int pdgcode = fastTrack.GetPrimaryTrack()->GetDefinition()->GetPDGEncoding();
  const G4ThreeVector& initialMomentum = fastTrack.GetPrimaryTrack()->GetMomentum();
  if (!m_fwdSvc->selectedParticle(initialMomentum, pdgcode)) { // FIXME Move method to this class?
    KillPrimaryTrack(fastTrack, fastStep);
    return;
  }

  const double charge = fastTrack.GetPrimaryTrack()->GetDefinition()->GetPDGCharge();
  const G4ThreeVector& initialPosition = fastTrack.GetPrimaryTrack()->GetPosition();
  const double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  const double energy = fastTrack.GetPrimaryTrack()->GetTotalEnergy();

  if (m_verboseLevel>5) {
    G4cout <<" pdgCode: " << pdgcode << " energy[GeV]: " << energy/CLHEP::GeV << " charge: " << charge << G4endl;
  }
  ForwardTracker::Particle fParticle = ForwardTracker::Particle(initialPosition.x()/CLHEP::m,
                                                                initialPosition.y()/CLHEP::m,
                                                                initialPosition.z()/CLHEP::m,
                                                                initialMomentum.x()/CLHEP::GeV,
                                                                initialMomentum.y()/CLHEP::GeV,
                                                                initialMomentum.z()/CLHEP::GeV,
                                                                std::abs(charge)>0);
  if (m_verboseLevel>5) {
    G4cout << fParticle << G4endl;
  }

  bool isTransported = m_fwdTrack.TrackParticle(fParticle);
  if (!isTransported) {
    KillPrimaryTrack(fastTrack, fastStep);
    return;
  }

  if (m_verboseLevel>5) {
    G4cout << m_fwdTrack.fPar() << G4endl;
  }

  ForwardTracker::Point fPos = m_fwdTrack.fPos();
  G4ThreeVector postTransportPosition(fPos.x()*CLHEP::m,   fPos.y()*CLHEP::m,   fPos.z()*CLHEP::m);

  ForwardTracker::Point fMom = m_fwdTrack.fMom();
  G4ThreeVector postTransportMomentum(fMom.x()*CLHEP::GeV, fMom.y()*CLHEP::GeV, fMom.z()*CLHEP::GeV);

  PrimaryParticleInformation *ppi = this->getPrimaryParticleInformation(fastTrack);
  HepMC::GenParticlePtr part = (ppi)? ppi->GetHepMCParticle() : HepMC::GenParticlePtr();
  HepMC::GenEvent* gEvent = (part) ? const_cast<HepMC::GenEvent*>(part->parent_event()) : nullptr;
  if (!gEvent) {
    G4ExceptionDescription description;
    description << "ForwardTransportModel::DoIt Cannot get HepMC::GenEvent pointer";
    G4Exception("ForwardTransportModel", "ForwardTransportModel03", FatalException, description);
    abort(); // to keep Coverity happy
  }
  // Update HepMC::GenEvent
  HepMC::GenVertexPtr gVertex = HepMC::newGenVertexPtr(
                                                       HepMC::FourVector(
                                                                         postTransportPosition.x(),
                                                                         postTransportPosition.y(),
                                                                         postTransportPosition.z(),
                                                                         time)); // TODO Update this value?
  gEvent->add_vertex(gVertex);
  gVertex->add_particle_in(part);
  HepMC::GenParticlePtr gParticle = HepMC::newGenParticlePtr(
                                                             HepMC::FourVector(
                                                                               postTransportMomentum.x(),
                                                                               postTransportMomentum.y(),
                                                                               postTransportMomentum.z(),
                                                                               energy),
                                                             pdgcode,
                                                             part->status() + HepMC::SIM_STATUS_INCREMENT);
  gVertex->add_particle_out(gParticle);
  HepMC::suggest_barcode(gParticle, HepMC::barcode(part)+HepMC::SIM_REGENERATION_INCREMENT);

  // Create secondary on the Geant4 side
  fastStep.SetNumberOfSecondaryTracks(1);

  const G4ParticleDefinition *aParticleDefinition{};
  /// Special cases for Geantinos
  if (pdgcode == MC::GEANTINOPLUS) {
    aParticleDefinition = G4ChargedGeantino::Definition();
  }
  if (pdgcode == MC::GEANTINO0) {
    aParticleDefinition = G4Geantino::GeantinoDefinition();
  }
  if (!aParticleDefinition) {
    /// Standard particles
    G4ParticleTable *ptable = G4ParticleTable::GetParticleTable();
    if (ptable) {
      aParticleDefinition = ptable->FindParticle(pdgcode);
    }
  }
  G4DynamicParticle dp2(aParticleDefinition, energy, postTransportMomentum);

  // Create UserInformation
  const ISF::ISFParticle* initialISP = ppi->GetISFParticle();
  std::unique_ptr<ISF::ISFParticle> postTransportISP{};
  if (initialISP) {
    // Create postTransportISP if required.
    const auto pBarcode = HepMC::barcode(gParticle);
    const auto particleID = HepMC::uniqueID(gParticle);
    const int status = gParticle->status();
    auto tBinding = std::make_unique<ISF::TruthBinding>(gParticle);
    auto hmpl = std::make_unique<HepMcParticleLink>(pBarcode, gEvent->event_number(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE); // FIXME barcode-based
    const Amg::Vector3D pos(postTransportPosition.x(), postTransportPosition.y(), postTransportPosition.z());
    const Amg::Vector3D mom(postTransportMomentum.x(), postTransportMomentum.y(), postTransportMomentum.z());
    postTransportISP = std::make_unique<ISF::ISFParticle>(pos,
                                                          mom,
                                                          initialISP->mass(),
                                                          initialISP->charge(),
                                                          initialISP->pdgCode(),
                                                          status,
                                                          time, // TODO Update??
                                                          *initialISP,
                                                          particleID,
                                                          pBarcode,
                                                          tBinding.release(),
                                                          hmpl.release());
  }
  std::unique_ptr<PrimaryParticleInformation> ppi2 = std::make_unique<PrimaryParticleInformation>(gParticle,postTransportISP.release());
  std::unique_ptr<G4PrimaryParticle> pp2 = std::make_unique<G4PrimaryParticle>(
                                                                               aParticleDefinition,
                                                                               postTransportMomentum.x(),
                                                                               postTransportMomentum.y(),
                                                                               postTransportMomentum.z());
  pp2->SetUserInformation(ppi2.release());
  dp2.SetPrimaryParticle(pp2.release());

  G4Track* postTransportTrack =
    fastStep.CreateSecondaryTrack(
                                  dp2,
                                  postTransportPosition,
                                  time,
                                  false); // position in global coordinates
  if (!postTransportTrack) {
    G4ExceptionDescription description;
    description << "ForwardTransportModel::DoIt Failed to create secondary G4Track.";
    G4Exception("ForwardTransportModel", "ForwardTransportModel04", FatalException, description);
    abort(); // to keep Coverity happy
  }
  fastStep.ProposePrimaryTrackFinalPosition(postTransportPosition, false); // position in global coordinates
  fastStep.ProposePrimaryTrackFinalMomentumDirection(postTransportMomentum, false); // FIXME This only sets the normalised direction of the momentum - possible use ProposePrimaryTrackFinalKineticEnergyAndDirection instead?
  fastStep.KillPrimaryTrack();
}


void ForwardTransportModel::KillPrimaryTrack(const G4FastTrack& fastTrack, G4FastStep& fastStep) {
  PrimaryParticleInformation *ppi = this->getPrimaryParticleInformation(fastTrack);
  HepMC::GenParticlePtr part = (ppi)? ppi->GetHepMCParticle() : HepMC::GenParticlePtr();
  HepMC::GenEvent* gEvent = (part) ? const_cast<HepMC::GenEvent*>(part->parent_event()) : nullptr;
  if (!gEvent) {
    G4ExceptionDescription description;
    description << "ForwardTransportModel::KillPrimaryTrack Cannot get HepMC::GenEvent pointer";
    G4Exception("ForwardTransportModel", "ForwardTransportModel02", FatalException, description);
    abort(); // to keep Coverity happy
  }
  const G4ThreeVector& initialPosition = fastTrack.GetPrimaryTrack()->GetPosition();
  // Add dummy end vertex in Truth
  HepMC::GenVertexPtr gVertex = HepMC::newGenVertexPtr(
                                                       HepMC::FourVector(
                                                                         initialPosition.x(),
                                                                         initialPosition.y(),
                                                                         initialPosition.z(),
                                                                         fastTrack.GetPrimaryTrack()->GetGlobalTime()));
  // Flag the fact that Forward Transport has occurred using the vertex status
#ifdef HEPMC3
  gVertex->set_status(HepMC::SIM_STATUS_THRESHOLD+1000+HepMC::FORWARD_TRANSPORT_MODEL_PROCESS);
#else
  gVertex->set_id(HepMC::SIM_STATUS_THRESHOLD+1000+HepMC::FORWARD_TRANSPORT_MODEL_PROCESS);
#endif
  gEvent->add_vertex(gVertex);
  gVertex->add_particle_in(part);
  // Kill track and deposit all energy in the current volume
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);
  fastStep.ProposeTotalEnergyDeposited(fastTrack.GetPrimaryTrack()->GetTotalEnergy());
}
