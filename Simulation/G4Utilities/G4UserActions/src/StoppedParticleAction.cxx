/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StoppedParticleAction.h"

#include "TrackWriteFastSim/TrackFastSimSD.h"

#include "TruthUtils/HepMCHelpers.h"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VProcess.hh"
#include "G4ProcessType.hh"

#include <cmath>

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"


namespace G4UA
{

  //---------------------------------------------------------------------------
  StoppedParticleAction::StoppedParticleAction()
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc"),
                   "StoppedParticleAction"),
      m_fsSD(0), m_init(false)
  {}

  //---------------------------------------------------------------------------
  void StoppedParticleAction::UserSteppingAction(const G4Step* aStep)
  {

    // Trigger if the energy is below our threshold or if the R-hadron is decaying
    const int id = std::abs(aStep->GetTrack()->GetDynamicParticle()->GetDefinition()->GetPDGEncoding());

    // Special treatment for SUSY particles and R-hadrons
    if (id>=1000000 && id<=1100000 && // exclude super-partners of RH-fermions
        (MC::isSquarkLH(id) ||
         id == 1000021 || // gluino
         MC::isRHadron(id))) {

      G4Material * mat = aStep->GetTrack()->GetMaterial();
      double minA=1500000.;
      for (unsigned int i=0;i<mat->GetNumberOfElements();++i){
        if (mat->GetElement(i) &&
            minA>mat->GetElement(i)->GetN()){
          minA=mat->GetElement(i)->GetN();
        }
      }

      // Stopping condition
      if (aStep->GetPostStepPoint()->GetVelocity()>0.15*std::pow(minA,-2./3.)*CLHEP::c_light && // Stopping condition or...
           ( !aStep->GetPostStepPoint()->GetProcessDefinedStep() || // null pointer?
             aStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessType()!=fDecay) ) // Decaying particle (does not fire for hadronic interactions)
        return;

      if (!m_init){
        m_init = true;

        G4SDManager * g4sdm = G4SDManager::GetSDMpointer();
        if (!g4sdm) {
          ATH_MSG_ERROR( "StoppedParticleFastSim could not get sensitive detector catalog." );
        } else {
          G4VSensitiveDetector * g4sd = g4sdm->FindSensitiveDetector("ToolSvc.SensitiveDetectorMasterTool.TrackFastSimSD");
          if (!g4sd) {
            ATH_MSG_ERROR( "StoppedParticleFastSim could not get ToolSvc.SensitiveDetectorMasterTool.TrackFastSimSD sensitive detector." );
          } else {
            m_fsSD = dynamic_cast<TrackFastSimSD*>(g4sd);
            if (!m_fsSD) {
              ATH_MSG_ERROR( "StoppedParticleFastSim could not cast the SD." );
            }
          } // found the SD
        } // got the catalog
      }

      if (m_fsSD) {
        m_fsSD->WriteTrack( aStep->GetTrack() , false , true );
      }
    }

    aStep->GetTrack()->SetTrackStatus(fStopAndKill);
    const G4TrackVector *tv = aStep->GetSecondary();
    for (unsigned int i=0;i<tv->size();i++){
      (*tv)[i]->SetTrackStatus(fStopAndKill);
    }
  }

} // namespace G4UA
