/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <iostream>
#include <memory>
#include <string>

// Local includes
#include "AthenaDebugStackingAction.h"

// Truth includes
#include "MCTruth/PrimaryParticleInformation.h"
#include "MCTruth/TrackInformation.h"
#include "MCTruth/TrackBarcodeInfo.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

// Geant4 includes
#include "G4Track.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"

#include "CxxUtils/checker_macros.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  AthenaDebugStackingAction::AthenaDebugStackingAction(const Config& config):
    AthenaStackingAction(config) {}

  //---------------------------------------------------------------------------
  // Classify a new track
  //---------------------------------------------------------------------------
  G4ClassificationOfNewTrack
  AthenaDebugStackingAction::ClassifyNewTrack(const G4Track* track)
  {
    // Kill neutrinos if enabled
    if(m_config.killAllNeutrinos && isNeutrino(track)) {
      return fKill;
    }

    // Kill super-low-E photons
    const double safeCut = 0.00005;
    double totalE = track->GetTotalEnergy();
    if(isGamma(track) && totalE < safeCut) {
      return fKill;
    }

    // TODO: Why is this here? Can I remove it?
    G4Event* ev = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
    AtlasG4EventUserInfo* atlasG4EvtUserInfo __attribute__ ((unused)) =
      static_cast<AtlasG4EventUserInfo*> (ev->GetUserInformation());

    // Was track subject to a RR?
    bool rouletted = false;

    // Neutron Russian Roulette
    if (m_config.russianRouletteNeutronThreshold > 0 && isNeutron(track) &&
        track->GetWeight() < m_config.russianRouletteNeutronWeight && // do not re-Roulette particles
        track->GetKineticEnergy() < m_config.russianRouletteNeutronThreshold) {
      // shoot random number
      if ( CLHEP::RandFlat::shoot() > m_oneOverWeightNeutron ) {
        if (m_config.applyNRR) {
          // Kill (w-1)/w neutrons
          return fKill;
        } else {
          // process them at the end of the stack
          return fWaiting;
        }
      }
      rouletted = true;
      // Weight the rest 1/w neutrons with a weight of w
      if (m_config.applyNRR) {
        // TODO There may be another way to set the weights via
        // another G4 interface avoiding the const_cast, but the
        // changes are more major and will need more careful validation.
        G4Track* mutableTrack ATLAS_THREAD_SAFE = const_cast<G4Track*> (track);
        mutableTrack->SetWeight(m_config.russianRouletteNeutronWeight);
      }
    }

    // Photon Russian Roulette
    if (m_config.russianRoulettePhotonThreshold > 0 && isGamma(track) && track->GetOriginTouchable() &&
        track->GetOriginTouchable()->GetVolume()->GetName().substr(0, 3) == "LAr" && // only for photons created in LAr
        track->GetWeight() < m_config.russianRoulettePhotonWeight && // do not re-Roulette particles
        track->GetKineticEnergy() < m_config.russianRoulettePhotonThreshold) {
      // shoot random number
      if ( CLHEP::RandFlat::shoot() > m_oneOverWeightPhoton ) {
        if (m_config.applyPRR) {
          // Kill (w-1)/w photons
          return fKill;
        } else {
          // process them at the end of the stack
          return fWaiting;
        }
      }
      rouletted = true;
      // Weight the rest 1/w neutrons with a weight of w
      if (m_config.applyPRR) {
        // TODO There may be another way to set the weights via
        // another G4 interface avoiding the const_cast, but the
        // changes are more major and will need more careful validation.
        G4Track* mutableTrack ATLAS_THREAD_SAFE = const_cast<G4Track*> (track);
        mutableTrack->SetWeight(m_config.russianRoulettePhotonWeight);
      }
    }

    // Handle primary particles
    if(track->GetParentID() == 0) { // Condition for Primaries
      // Extract the PrimaryParticleInformation
      PrimaryParticleInformation* primaryPartInfo = this->getPrimaryParticleInformation(track);
      // Fill some information for this track
      if(primaryPartInfo) {
        if (!m_config.isISFJob) {
          // don't do anything
          auto part = primaryPartInfo->GetHepMCParticle();
          if(part) {
            // OK, we got back to HepMC
            std::unique_ptr<TrackInformation> ti = std::make_unique<TrackInformation>(part);
            ti->SetRegenerationNr(0);
            ti->SetClassification(Primary);
            // regNr=0 and classify=Primary are default values anyway
            /// Pass ownership to track. The G4VUserTrackInformation*
            /// fpUserInformation member variable set by this method
            /// is mutable. G4Tracks are thread-local.
            track->SetUserInformation(ti.release());
          }
          // What does this condition mean?
          else if(primaryPartInfo->GetParticleUniqueID() >= 0 && primaryPartInfo->GetParticleBarcode() >= 0) {
            // PrimaryParticleInformation should at least provide a barcode
            std::unique_ptr<TrackBarcodeInfo> bi = std::make_unique<TrackBarcodeInfo>(primaryPartInfo->GetParticleUniqueID(), primaryPartInfo->GetParticleBarcode());
            /// Pass ownership to track. The G4VUserTrackInformation*
            /// fpUserInformation member variable set by this method
            /// is mutable. G4Tracks are thread-local.
            track->SetUserInformation(bi.release());
          }
        } // no ISFParticle attached
      } // has PrimaryParticleInformation
    }
    // Secondary track; decide whether to save or kill
    else if( isGamma(track) &&
             m_config.photonEnergyCut > 0 &&
             totalE < m_config.photonEnergyCut )
      {
        return fKill;
      }
    // Put rouletted tracks at the end of the stack
    if (rouletted)
      return fWaiting;
    else
      return fUrgent;
  }

} // namespace G4UA
