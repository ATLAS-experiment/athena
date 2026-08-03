/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



// Local includes
#include "AthenaStackingAction.h"

// Truth includes
#include "MCTruth/PrimaryParticleInformation.h"
#include "MCTruth/TrackInformation.h"
#include "MCTruth/TrackBarcodeInfo.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

// Geant4 includes
#include "G4Track.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4NeutrinoE.hh"
#include "G4NeutrinoMu.hh"
#include "G4NeutrinoTau.hh"
#include "G4AntiNeutrinoE.hh"
#include "G4AntiNeutrinoMu.hh"
#include "G4AntiNeutrinoTau.hh"
#include "G4Gamma.hh"
#include "G4Neutron.hh"

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/fpcompare.h"

// System includes
#include <memory>
#include <string>

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  AthenaStackingAction::AthenaStackingAction(const Config& config):
    m_config(config),
    m_oneOverWeightNeutron(0),
    m_oneOverWeightPhoton(0)
  {
    // calculate this division only once
    if (m_config.applyNRR)
      m_oneOverWeightNeutron = 1./m_config.russianRouletteNeutronWeight;

    // calculate this division only once
    if (m_config.applyPRR)
      m_oneOverWeightPhoton = 1./m_config.russianRoulettePhotonWeight;
  }

  //---------------------------------------------------------------------------
  // Classify a new track
  //---------------------------------------------------------------------------
  G4ClassificationOfNewTrack
  AthenaStackingAction::ClassifyNewTrack(const G4Track* track)
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

    // Neutron Russian Roulette
    if (m_config.applyNRR && isNeutron(track) &&
        CxxUtils::fpcompare::equal(track->GetWeight(), 1.0) && // do not re-Roulette particles
        track->GetKineticEnergy() < m_config.russianRouletteNeutronThreshold) {
      // shoot random number
      if ( CLHEP::RandFlat::shoot() > m_oneOverWeightNeutron ) {
        // Kill (w-1)/w neutrons
        return fKill;
      }
      // TODO There may be another way to set the weights via
      // another G4 interface avoiding the const_cast, but the
      // changes are more major and will need more careful validation.
      G4Track* mutableTrack ATLAS_THREAD_SAFE = const_cast<G4Track*> (track);
      // Weight the rest 1/w neutrons with a weight of w
      mutableTrack->SetWeight(m_config.russianRouletteNeutronWeight);
    }

    // Photon Russian Roulette
    if (m_config.applyPRR && isGamma(track) && track->GetOriginTouchable() &&
        track->GetOriginTouchable()->GetVolume()->GetName().substr(0, 3) == "LAr" && // only for photons created in LAr
        CxxUtils::fpcompare::equal(track->GetWeight(), 1.0) && // do not re-Roulette particles
        track->GetKineticEnergy() < m_config.russianRoulettePhotonThreshold) {
      // shoot random number
      if ( CLHEP::RandFlat::shoot() > m_oneOverWeightPhoton ) {
        // Kill (w-1)/w photons
        return fKill;
      }
      // TODO There may be another way to set the weights via
      // another G4 interface avoiding the const_cast, but the
      // changes are more major and will need more careful validation.
      G4Track* mutableTrack ATLAS_THREAD_SAFE = const_cast<G4Track*> (track);
      // Weight the rest 1/w neutrons with a weight of w
      mutableTrack->SetWeight(m_config.russianRoulettePhotonWeight);
    }

    // Handle primary particles
    if(track->GetParentID() == 0) { // Condition for Primaries
      // Extract the PrimaryParticleInformation
      PrimaryParticleInformation* primaryPartInfo = this->getPrimaryParticleInformation(track);
      // Fill some information for this track
      if(primaryPartInfo) {
        if (!m_config.isISFJob) {
          // don't do anything
          auto  part = primaryPartInfo->GetHepMCParticle();
          if (part) {
            // OK, we got back to HepMC
            std::unique_ptr<TrackInformation> ti = std::make_unique<TrackInformation>(part);
            ti->SetPrimaryGenParticle(part);
            ti->SetGenerationZeroGenParticle(part);
            ti->SetRegenerationNr(0);
            ti->SetClassification(TrackInformation::Primary);
            // regNr=0 and classify=Primary are default values anyway
            /// Pass ownership to track. The G4VUserTrackInformation*
            /// fpUserInformation member variable set by this method
            /// is mutable. G4Tracks are thread-local.
            track->SetUserInformation(ti.release());
          }
          // TODO What does this condition mean?
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
    return fUrgent;
  }

  PrimaryParticleInformation* AthenaStackingAction::getPrimaryParticleInformation(const G4Track *track) const
  {
    const G4DynamicParticle* dp = track->GetDynamicParticle();
    if(dp) {
      const G4PrimaryParticle* pp = nullptr;
      pp = dp->GetPrimaryParticle();
      if(pp) {
        // Extract the PrimaryParticleInformation
        return dynamic_cast<PrimaryParticleInformation*>
          ( pp->GetUserInformation() );
      }
    }
    return nullptr;
  }

  //---------------------------------------------------------------------------
  // Identify track definition
  //---------------------------------------------------------------------------
  bool AthenaStackingAction::isNeutrino(const G4Track* track) const
  {
    auto particleDef = track->GetParticleDefinition();
    return (particleDef == G4NeutrinoE::NeutrinoEDefinition()           ||
            particleDef == G4AntiNeutrinoE::AntiNeutrinoEDefinition()   ||
            particleDef == G4NeutrinoMu::NeutrinoMuDefinition()         ||
            particleDef == G4AntiNeutrinoMu::AntiNeutrinoMuDefinition() ||
            particleDef == G4NeutrinoTau::NeutrinoTauDefinition()       ||
            particleDef == G4AntiNeutrinoTau::AntiNeutrinoTauDefinition());
  }

  //---------------------------------------------------------------------------
  bool AthenaStackingAction::isGamma(const G4Track* track) const
  {
    return track->GetParticleDefinition() == G4Gamma::Gamma();
  }

  //---------------------------------------------------------------------------
  bool AthenaStackingAction::isNeutron(const G4Track* track) const
  {
    return track->GetParticleDefinition() == G4Neutron::Neutron();
  }

} // namespace G4UA
