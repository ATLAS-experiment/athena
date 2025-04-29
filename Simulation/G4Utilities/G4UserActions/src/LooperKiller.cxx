/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LooperKiller.h"

#include "G4RunManagerKernel.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4PropagatorInField.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4StackManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "MCTruth/TrackHelper.h"
#include "MCTruth/TrackInformation.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODEventInfo/EventInfo.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  LooperKiller::LooperKiller(const Config& config)
    : AthMessaging(Gaudi::svcLocator()->service<IMessageSvc>("MessageSvc"),
                   "LooperKiller"),
      m_evtStore("StoreGateSvc/StoreGateSvc", "LooperKiller"),
      m_detStore("StoreGateSvc/DetectorStore", "LooperKiller"),
      m_config(config), m_report(), m_count_steps(0)
  {
  }

  //---------------------------------------------------------------------------
  void LooperKiller::UserSteppingAction(const G4Step* aStep)
  {

    if (aStep->GetTrack()->GetCurrentStepNumber() < m_config.MaxSteps) {
      if (m_count_steps==0) return;
      // Track recovered...
      ATH_MSG_WARNING("Track finished on its own.  Congrats.  Moving on with the event.");
      m_count_steps = 0;
      G4TransportationManager *tm = G4TransportationManager::GetTransportationManager();
      tm->GetNavigatorForTracking()->SetVerboseLevel(0);
      tm->GetPropagatorInField()->SetVerboseLevel(0);
      G4RunManagerKernel *rmk = G4RunManagerKernel::GetRunManagerKernel();
      rmk->GetTrackingManager()->SetVerboseLevel(0);
      rmk->GetTrackingManager()->GetSteppingManager()->SetVerboseLevel(0);
      rmk->GetStackManager()->SetVerboseLevel(0);
      return;
    } else if (aStep->GetTrack()->GetCurrentStepNumber() == m_config.MaxSteps) {
      ATH_MSG_WARNING("LooperKiller triggered!! Hold on to your hats!!!!!!!!" );
    }

    G4TransportationManager *tm = G4TransportationManager::GetTransportationManager();
    tm->GetNavigatorForTracking()->SetVerboseLevel(m_config.VerboseLevel);
    tm->GetPropagatorInField()->SetVerboseLevel(m_config.VerboseLevel);

    G4RunManagerKernel *rmk = G4RunManagerKernel::GetRunManagerKernel();
    rmk->GetTrackingManager()->SetVerboseLevel(m_config.VerboseLevel);
    rmk->GetTrackingManager()->GetSteppingManager()->SetVerboseLevel(m_config.VerboseLevel);
    rmk->GetStackManager()->SetVerboseLevel(m_config.VerboseLevel);

    m_count_steps++;

    if (m_count_steps>m_config.PrintSteps) {
      m_count_steps = 0;
      m_report.killed_tracks++;
      aStep->GetTrack()->SetTrackStatus(fStopAndKill);
      tm->GetNavigatorForTracking()->SetVerboseLevel(0);
      tm->GetPropagatorInField()->SetVerboseLevel(0);
      rmk->GetTrackingManager()->SetVerboseLevel(0);
      rmk->GetTrackingManager()->GetSteppingManager()->SetVerboseLevel(0);
      rmk->GetStackManager()->SetVerboseLevel(0);
      int pdg_id{0};
      TrackHelper trackHelper(aStep->GetTrack());
      if ( m_config.BSM_Only && (trackHelper.IsPrimary() || trackHelper.IsRegisteredSecondary()) ) {
        HepMC::GenParticlePtr part = trackHelper.GetTrackInformation()->GetCurrentGenParticle();
        if (part) { pdg_id = part->pdg_id(); }
      }
      if ( !m_config.BSM_Only || MC::isBSM(pdg_id)) { // Sometimes we may ony want to bail out for BSM particles.
        // Bail out...
        if (m_config.AbortEvent){
          rmk->GetEventManager()->AbortCurrentEvent();
          rmk->GetEventManager()->GetNonconstCurrentEvent()->SetEventAborted();
        }
        if (m_config.SetError){

          // Set error state in eventInfo
          SG::ReadHandle<xAOD::EventInfo> eic("McEventInfo");
          if (! eic.isValid()){
            ATH_MSG_WARNING( "Failed to retrieve EventInfo" );
          } else {
            eic->updateErrorState(xAOD::EventInfo::Core,xAOD::EventInfo::Error);
            ATH_MSG_WARNING( "Set error state in event info!" );
          }
        } // End of set error
      } // End of BSM-only check
      const std::string name = aStep->GetTrack()->GetDefinition()->GetParticleName();
      if ( ( m_config.BSM_Only && !MC::isBSM(pdg_id) ) || !m_config.AbortEvent )  {
          ATH_MSG_INFO ("Quietly stopped tracking looping " << name
                        << " (trackID " << aStep->GetTrack()->GetTrackID()
                        << ", track pos: "<<aStep->GetTrack()->GetPosition()
                        << ", mom: "<<aStep->GetTrack()->GetMomentum()
                        << ", parentID " << aStep->GetTrack()->GetParentID() << ")");
        } else {
          ATH_MSG_WARNING ("Stopped tracking looping " << name
                        << " (trackID " << aStep->GetTrack()->GetTrackID()
                        << ", track pos: "<<aStep->GetTrack()->GetPosition()
                        << ", mom: "<<aStep->GetTrack()->GetMomentum()
                        << ", parentID " << aStep->GetTrack()->GetParentID() << "). The event will abort now.");
        }
    } // End of handling end of error time
  }

} // namespace G4UA
