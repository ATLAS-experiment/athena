/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackWriteFastSim/StoppedParticleFastSim.h"
#include "TrackWriteFastSim/TrackFastSimSD.h"

#include "TruthUtils/HepMCHelpers.h"

#include "G4FastTrack.hh"
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4ElementVector.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "CLHEP/Units/PhysicalConstants.h"

#include <cmath>

StoppedParticleFastSim::StoppedParticleFastSim(const std::string& name, const std::string& fsSDname)
  : G4VFastSimulationModel(name)
  , m_fsSDname(fsSDname)
{
}

G4bool StoppedParticleFastSim::IsApplicable(const G4ParticleDefinition&)
{
  return true;
}

G4bool StoppedParticleFastSim::ModelTrigger(const G4FastTrack& fastTrack)
{
  // Trigger if the energy is below our threshold or if the time is over 150 ns
  int id = fastTrack.GetPrimaryTrack()->GetDynamicParticle()->GetDefinition()->GetPDGEncoding();
  if (id<1000000 || id>1100000) return true; // skip SM particles and super-partners of RH-fermions
  if (MC::isSquarkLH(id) ||
      id == 1000021 || // gluino
      MC::isRHadron(id)) {
    G4Material * mat = fastTrack.GetPrimaryTrack()->GetMaterial();
    double minA=1500000.;
    for (unsigned int i=0;i<mat->GetNumberOfElements();++i){
      if (mat->GetElement(i) &&
          minA>mat->GetElement(i)->GetN()){
        minA=mat->GetElement(i)->GetN();
      }
    }
    if (fastTrack.GetPrimaryTrack()->GetVelocity()<0.15*std::pow(minA,-2./3.)*CLHEP::c_light) return true;
    return false;
  }
  return true;
}

void StoppedParticleFastSim::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  if (!m_init){
    m_init = true;

    G4SDManager *sdm = G4SDManager::GetSDMpointer();
    G4VSensitiveDetector * vsd = sdm->FindSensitiveDetector( m_fsSDname );
    if (vsd) {
      m_fsSD = dynamic_cast<TrackFastSimSD*>(vsd);
      if (!m_fsSD) {
        G4ExceptionDescription description;
        description << "DoIt: Could not cast the SD into an instance of TrackFasSimSD.";
        G4Exception("StoppedParticleFastSim", "MissingTrackFastSimSD", FatalException, description);
        abort();
      }
    }
    else {
      G4cout << "StoppedParticleFastSim::DoIt INFO Could not get TrackFastSimSD sensitive detector.  If you are not writing track records this is expected." << G4endl;
    }
    // found the SD
  } // End of lazy init
  const int id = fastTrack.GetPrimaryTrack()->GetDynamicParticle()->GetDefinition()->GetPDGEncoding();
  if (m_fsSD &&
      (MC::isSquarkLH(id) ||
       id == 1000021 || // gluino
       MC::isRHadron(id))) {
    m_fsSD->WriteTrack( fastTrack.GetPrimaryTrack() , false , true );
  }
  fastStep.KillPrimaryTrack();
}
