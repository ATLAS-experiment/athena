/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "ISF_Geant4Event/Geant4TruthIncident.h"

// package includes
#include "ISF_Geant4Event/ISFG4Helper.h"

// Atlas G4 Helpers
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"
#include "MCTruth/TrackInformation.h"
#include "MCTruth/PrimaryParticleInformation.h"

// Units
#include "GaudiKernel/PhysicalConstants.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/MagicNumbers.h"

// Geant4 includes
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include "G4TrackStatus.hh"
#include "G4ProcessType.hh"
#include "G4EmProcessSubType.hh"
#include "G4DynamicParticle.hh"
#include "G4PrimaryParticle.hh"

#include "G4EventManager.hh"
#include "G4Event.hh"

// ISF includes
#include "ISF_Event/ISFParticle.h"
#include <limits>
/*
  Comments:
  what about parent particle surviving (e.g. bremstrahlung)


  Existing code:
  Simulation/G4Sim/SimHelpers/src/StepHelper.cxx
  - provids convenient information of a G4Step
  Simulation/G4Utilities/G4TruthStrategies/src/BremsstrahlungStrategy.cxx
  - retrieves information from a G4Step (via StepHelper)
  Simulation/G4Sim/MCTruth/MCTruth/TruthStrategy.h
  - common base for different truth strategies
  Simulation/G4Sim/MCTruth/src/AtlasG4EventUserInfo.cxx
  - stores HepMCevent in G4
  Simulation/G4Sim/MCTruth/src/TrackInformation.cxx
  Simulation/G4Sim/MCTruth/src/TrackHelper.cxx
  - store/manage barcode
*/


iGeant4::Geant4TruthIncident::Geant4TruthIncident( const G4Step *step,
                                               const ISF::ISFParticle& baseISP,
                                               AtlasDetDescr::AtlasRegion geoID,
                                               AtlasG4EventUserInfo *atlasG4EvtUserInfo) :
  ITruthIncident(geoID, step->GetSecondaryInCurrentStep()->size()), // switch to G4Step::GetNumberOfSecondariesInCurrentStep() once we're using G4 10.2 or later
  m_positionSet(false),
  m_position(),
  m_step(step),
  m_baseISP(baseISP),
  m_atlasG4EvtUserInfo(atlasG4EvtUserInfo)
{
  // prepare children:
  prepareChildren();

  // calculate position:
  const G4StepPoint *postStepPoint = m_step->GetPostStepPoint();
  const G4ThreeVector         &pos = postStepPoint->GetPosition();
  const G4double              time = postStepPoint->GetGlobalTime()*Gaudi::Units::c_light;
  m_position.set( pos.x(), pos.y(), pos.z(), time );
}

const HepMC::FourVector& iGeant4::Geant4TruthIncident::position() const {
  return m_position;
}

int iGeant4::Geant4TruthIncident::physicsProcessCategory() const {
  const G4VProcess *process = m_step->GetPostStepPoint()->GetProcessDefinedStep();
  // TODO: need to check that G4ProcessSubTypes Match int
  return process->GetProcessType();
}

int iGeant4::Geant4TruthIncident::physicsProcessCode() const {
  const G4VProcess *process = m_step->GetPostStepPoint()->GetProcessDefinedStep();
  // TODO: need to check that G4ProcessSubTypes Match int
  return process->GetProcessSubType();
}

double iGeant4::Geant4TruthIncident::parentP2() const {
  const G4ThreeVector & mom= m_step->GetPreStepPoint()->GetMomentum();
  return mom.mag2();
}

double iGeant4::Geant4TruthIncident::parentPt2() const {
  const G4ThreeVector & mom= m_step->GetPreStepPoint()->GetMomentum();
  return mom.perp2();
}

double iGeant4::Geant4TruthIncident::parentEkin() const {
  return (m_step->GetPreStepPoint()->GetKineticEnergy()/CLHEP::MeV);
}

int iGeant4::Geant4TruthIncident::parentPdgCode() const {
  return  m_step->GetTrack()->GetDefinition()->GetPDGEncoding();
}

int iGeant4::Geant4TruthIncident::parentBarcode() { // TODO Remove this method
  auto parent = parentParticle();

  return (parent) ? HepMC::barcode(parent) : HepMC::UNDEFINED_ID;
}

int iGeant4::Geant4TruthIncident::parentUniqueID() {
  auto parent = parentParticle();

  return (parent) ? HepMC::uniqueID(parent) : HepMC::UNDEFINED_ID;
}

int iGeant4::Geant4TruthIncident::parentStatus()  {
 auto parent = parentParticle();

 return (parent) ? parent->status() : 0;
}

HepMC::GenParticlePtr iGeant4::Geant4TruthIncident::parentParticle() {
  return m_atlasG4EvtUserInfo->GetCurrentGenParticle();
}

bool iGeant4::Geant4TruthIncident::parentSurvivesIncident() const { 
  const G4Track *track = m_step->GetTrack();

  // check if particle is a alive in G4 or in ISF
  if ( particleAlive(track) ) {
    return true;
  } else {
    return false;
  }
}

HepMC::GenParticlePtr iGeant4::Geant4TruthIncident::parentParticleAfterIncident(int newBarcode) {
  const G4Track *track = m_step->GetTrack();

  // check if particle is a alive in G4 or in ISF
  if ( !parentSurvivesIncident() ) {
    return nullptr;
  }

  // internally cache the HepMC representation of the parent particle
  // (to allow multiple calls to this method on the same instance)
  if ( !m_parentParticleAfterIncident ) {
    // create new HepMC particle, using momentum and energy
    // from G4DynamicParticle (which should be equivalent to postStep)
    m_parentParticleAfterIncident = convert(track, newBarcode, false);
    
    m_atlasG4EvtUserInfo->SetCurrentGenParticle( m_parentParticleAfterIncident );
    
    // store (new) hepmc particle in track's UserInformation
    TrackHelper       tHelper(track);
    TrackInformation *tInfo = tHelper.GetTrackInformation();
    if (tInfo) {
      // do NOT update the TrackInformation for regenerated particles!
      // (most recent truth info is kept in AtlasG4EventUserInfo)
      //tInfo->SetCurrentGenParticle( m_parentParticleAfterIncident );
      int regenerationNr = tInfo->GetRegenerationNr();
      regenerationNr++;
      tInfo->SetRegenerationNr(regenerationNr);
      if ( tHelper.IsPrimary() ) { 
        tInfo->SetClassification(TrackInformation::RegeneratedPrimary);
      }
    }

  } // <-- end if m_parentParticleAfterIncident is not filled


  // return the HepMC particle representation of the parent
  return m_parentParticleAfterIncident;
}

double iGeant4::Geant4TruthIncident::childP2(unsigned short i) const {
  const G4ThreeVector & mom= m_children[i]->GetMomentum();
  return mom.mag2();
}

const G4ThreeVector iGeant4::Geant4TruthIncident::childP(unsigned short i) const {
  const G4ThreeVector & mom= m_children[i]->GetMomentum();
  return mom;
}

double iGeant4::Geant4TruthIncident::childPt2(unsigned short i) const {
  const G4ThreeVector & mom= m_children[i]->GetMomentum();
  return mom.perp2();
}

double iGeant4::Geant4TruthIncident::childEkin(unsigned short i) const {
  return (m_children[i]->GetKineticEnergy()/CLHEP::MeV);
}

int iGeant4::Geant4TruthIncident::childPdgCode(unsigned short i) const {
  return m_children[i]->GetDefinition()->GetPDGEncoding();
}

int  iGeant4::Geant4TruthIncident::childBarcode(unsigned short index) const {
  // the G4Track instance for the current child particle
  const G4Track* track = m_children[index];
  // This should be a *secondary* track.  If it has a primary, it was a decay and
  //  we are running with quasi-stable particle simulation.  Note that if the primary
  //  track is passed in as a secondary that survived the interaction, then this was
  //  *not* a decay and we should not treat it in this way
  if (track->GetDynamicParticle() &&
      track->GetDynamicParticle()->GetPrimaryParticle() &&
      track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation()){
    // Then the new particle should use the same barcode as the old one!!
    PrimaryParticleInformation* primaryPartInfo = dynamic_cast<PrimaryParticleInformation*>( track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation() );
    return primaryPartInfo->GetParticleBarcode();
  }
  return 0;
}

HepMC::GenParticlePtr iGeant4::Geant4TruthIncident::childParticle(unsigned short i,
                                                                  int newBarcode) {
  // the G4Track instance for the current child particle
  const G4Track* thisChildTrack = m_children[i];

  // NB: NOT checking if secondary is actually alive. Even with zero momentum,
  //     secondary could decay right away and create further particles which pass the
  //     truth strategies.

  HepMC::GenParticlePtr hepParticle = convert( thisChildTrack, newBarcode, true );
  TrackHelper tHelper(thisChildTrack);
  TrackInformation *trackInfo = tHelper.GetTrackInformation();

  // needed to make AtlasG4 work with ISF TruthService
  if (trackInfo==nullptr) {
    trackInfo = new TrackInformation( hepParticle );
    thisChildTrack->SetUserInformation( trackInfo );
  }

  trackInfo->SetCurrentGenParticle(hepParticle);
  trackInfo->SetClassification(TrackInformation::RegisteredSecondary);
  trackInfo->SetRegenerationNr(0);

  return hepParticle;
}


bool iGeant4::Geant4TruthIncident::particleAlive(const G4Track *track) const {
  G4TrackStatus  trackStatus = track->GetTrackStatus();

  if ( trackStatus != fAlive && trackStatus != fStopButAlive ) {
    // parent does not exist in G4 anymore after this step

    // check whether the particle was returned to ISF
    auto*    trackInfo = ISFG4Helper::getISFTrackInfo( *track );
    bool returnedToISF = trackInfo ? trackInfo->GetReturnedToISF() : false;
    if ( !returnedToISF ) {
      // particle was not sent to ISF either
      return false;
    }
  }

  return true;
}

#ifdef HEPMC3
HepMC::GenParticlePtr iGeant4::Geant4TruthIncident::convert(const G4Track *track, const int, const bool secondary) const {
#else
HepMC::GenParticlePtr iGeant4::Geant4TruthIncident::convert(const G4Track *track, const int barcode, const bool secondary) const {
#endif

  const G4ThreeVector & mom =  track->GetMomentum();
  const double energy =  track->GetTotalEnergy();
  const int pdgCode = track->GetDefinition()->GetPDGEncoding();
  const HepMC::FourVector fourMomentum( mom.x(), mom.y(), mom.z(), energy);

  const HepMC::GenParticlePtr parent = m_atlasG4EvtUserInfo->GetCurrentGenParticle();
  int status = (secondary) ? 1 + HepMC::SIM_STATUS_THRESHOLD : parent->status() + HepMC::SIM_STATUS_INCREMENT;
  // Treat child particles of pre-defined decays differently
  if (this->interactionClassification() == ISF::QS_PREDEF_VTX) {
    const G4DynamicParticle* dynPart = track->GetDynamicParticle();
    bool hasPredefinedDecay = (dynPart && (nullptr!=(dynPart->GetPreAssignedDecayProducts())));
    status = (hasPredefinedDecay)? 2 : 1;
  }
  // Treat particles with a pre-defined decay which have survived an
  // interaction differently for now.
  if (this->interactionClassification() == ISF::QS_SURV_VTX) {
    status = status%HepMC::SIM_STATUS_INCREMENT;
    // Such particles were previously assigned barcodes below
    // HepMC::SIM_BARCODE_OFFSET and therefore
    // HepMC::is_simulation_particle(barcode) would return false. Not
    // applying HepMC::SIM_STATUS_INCREMENT means that
    // HepMC::is_simulation_particle(status) will also return false.
    // TODO This should be revisited in the future should we decide
    // that we want to be able to easily identify such particles.
  }
  HepMC::GenParticlePtr newParticle = HepMC::newGenParticlePtr(fourMomentum, pdgCode, status);

#ifndef HEPMC3
  // This should be a *secondary* track.  If it has a primary, it was a decay and
  //  we are running with quasi-stable particle simulation.  Note that if the primary
  //  track is passed in as a secondary that survived the interaction, then this was
  //  *not* a decay and we should not treat it in this way
  if (secondary &&
      track->GetDynamicParticle() &&
      track->GetDynamicParticle()->GetPrimaryParticle() &&
      track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation()){
    // Then the new particle should use the same barcode as the old one!!
    PrimaryParticleInformation* primaryPartInfo = dynamic_cast<PrimaryParticleInformation*>( track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation() );
    HepMC::suggest_barcode( newParticle, primaryPartInfo->GetParticleBarcode() );
  } else {
    HepMC::suggest_barcode( newParticle, barcode );
  }
#endif

  return newParticle;
}


void iGeant4::Geant4TruthIncident::prepareChildren() {

  const std::vector<const G4Track*>* tracks = m_step->GetSecondaryInCurrentStep();
  const int iSize = tracks->size();
  const int iLast = iSize - numberOfChildren() - 1; //NB can be -1.
  for(int i=iSize-1;i>iLast;i--) {
    m_children.push_back((*tracks)[i]);
  }
}

/**  The interaction classifications are described as follows:
     STD_VTX: interaction of a particle without a pre-defined decay;
     QS_SURV_VTX: a particle with a pre-defined decay under-going a
     non-destructive interaction;
     QS_DEST_VTX: a particle with a pre-defined decay under-going a
     destructive interaction other than its pre-defined decay;
     QS_PREDEF_VTX: a particle under-going its pre-defined decay */
ISF::InteractionClass_t iGeant4::Geant4TruthIncident::interactionClassification() const {
  G4Track* track=m_step->GetTrack();
  const G4DynamicParticle* dynPart = track->GetDynamicParticle();
  const double preAsgnDecPropTime = dynPart->GetPreAssignedDecayProperTime();
  const double trackPropTime = track->GetProperTime();
  bool parentIsQuasiStable = (nullptr!=(dynPart->GetPreAssignedDecayProducts()));
  const G4VProcess *process = m_step->GetPostStepPoint()->GetProcessDefinedStep();
  const int processType = process->GetProcessType();
  ISF::InteractionClass_t classification(ISF::STD_VTX);
  if(parentIsQuasiStable) {
    if(this->parentSurvivesIncident()) {
      classification = ISF::QS_SURV_VTX;
    }
    else if(processType==6 && (trackPropTime-preAsgnDecPropTime) < -std::numeric_limits<double>::epsilon()) {
      // Particle decayed before its expected pre-defined decay time
      classification = ISF::QS_DEST_VTX;
    }
    else {
      classification = ISF::QS_PREDEF_VTX;
    }
  }
  return classification;
  // end of code to determine interaction type
}
