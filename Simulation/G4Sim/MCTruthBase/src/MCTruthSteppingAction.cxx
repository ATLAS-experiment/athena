/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruthSteppingAction.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"
#include "MCTruth/TrackInformation.h"
#include "ISF_Geant4Event/Geant4TruthIncident.h"
#include "ISF_Geant4Event/ISFG4GeoHelper.h"
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Interfaces/ITruthSvc.h"

#include "G4Event.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"

#include <memory>

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  MCTruthSteppingAction::
  MCTruthSteppingAction(const VolumeCollectionMap_t& volCollMap,
                        int secondarySavingLevel,
                        int subDetVolLevel,
                        ISF::ITruthSvc& truthRecordSvc,
                        ISF::IGeoIDSvc& geoIDSvc,
                        IMessageSvc* msgSvc, MSG::Level level)
    : AthMessaging(msgSvc, "MCTruthSteppingAction"),
      m_isInitialized(false),
      m_secondarySavingLevel(secondarySavingLevel),
      m_subDetVolLevel(subDetVolLevel),
      m_truthRecordSvc(truthRecordSvc),
      m_geoIDSvc(geoIDSvc),
      m_volumeCollectionMap(volCollMap)
  {
    msg().setLevel(level);
  }

  //---------------------------------------------------------------------------
  // Setup the recording envelopes
  //---------------------------------------------------------------------------
  void MCTruthSteppingAction::setupRecEnvelopes()
  {
    ATH_MSG_DEBUG("Setting up " << m_volumeCollectionMap.size() <<
                  " recording envelopes:");
    m_recordingEnvelopes.reserve( m_volumeCollectionMap.size() );
    for(const auto& volCollPair : m_volumeCollectionMap) {
      ATH_MSG_DEBUG("  " << volCollPair.first << ", " << volCollPair.second);

      // Construct the helper in place on the vector
      m_recordingEnvelopes.emplace_back(volCollPair.first, volCollPair.second);
      RecordingEnvelope& recEnvelope = m_recordingEnvelopes.back();

      // Make sure the RecEnvelope can initialize properly
      if(!recEnvelope.Initialize()) {
        //FIXME - should this be an error?
        ATH_MSG_WARNING("Envelope volume " << recEnvelope.GetVolumeName() <<
                        " not found in geometry!");
        ATH_MSG_WARNING("TrackRecordCollection " <<
                        recEnvelope.GetTrackRecordCollectionName() <<
                        " will NOT be recorded");
        // Throw away uninitialized RecordingEnvelope
        m_recordingEnvelopes.pop_back();
      }
    }
    if (m_recordingEnvelopes.size() == 0) {
      ATH_MSG_WARNING("No recording envelopes found!");
    }
  }

  //---------------------------------------------------------------------------
  // Beginning of event
  //---------------------------------------------------------------------------
  void MCTruthSteppingAction::BeginOfEventAction(const G4Event* event)
  {
    // First time initialization
    if(!m_isInitialized) {
      setupRecEnvelopes();
      if (m_recordingEnvelopes.size() == 0) {
        ATH_MSG_WARNING("No recording envelopes found!");
      }
      m_isInitialized = true;
    }
    // Every event initialization
    for (auto& recEnvelope : m_recordingEnvelopes) {
      if(auto* eventInfo = static_cast<AtlasG4EventUserInfo*>( event->GetUserInformation())){
        recEnvelope.BeginOfEvent(eventInfo->GetHitCollectionMap()->Find<TrackRecordCollection>(recEnvelope.GetTrackRecordCollectionName()));
      }
    }
  }

  //---------------------------------------------------------------------------
  // Process one tracking step
  //---------------------------------------------------------------------------
  void MCTruthSteppingAction::UserSteppingAction(const G4Step* aStep)
  {
    // Most steps neither create secondaries nor change volume. Nothing below has
    // an effect for them, so return before building the TrackHelper (dynamic_cast).
    const std::vector<const G4Track*>* secondaries = aStep->GetSecondaryInCurrentStep();
    const bool hasSecondaries = secondaries && !secondaries->empty();

    G4StepPoint* preStep = aStep->GetPreStepPoint();
    G4StepPoint* postStep = aStep->GetPostStepPoint();
    const bool checkEnvelopes = !m_recordingEnvelopes.empty() &&
      preStep->GetPhysicalVolume() != postStep->GetPhysicalVolume();

    if (!hasSecondaries && !checkEnvelopes) return;

    TrackHelper trackHelper(aStep->GetTrack());

    if (hasSecondaries) {
      // info must be propagated to secondaries before MC truth incident can be created
      propagatePrimaryInfoToSecondaries(*secondaries, trackHelper);

      // A saved primary is reclassified, but its old trajectory stayed active.
      const bool processTruth =
        trackHelper.IsPrimary() || trackHelper.IsRegeneratedPrimary() ||
        (trackHelper.IsRegisteredSecondary() && m_secondarySavingLevel > 1);
      if (processTruth) {
        createTruthIncident(aStep);
      }
    }

    if (!checkEnvelopes) return;
    if (trackHelper.IsSecondary()) return;

    const G4TouchableHistory* preTHist = static_cast<const G4TouchableHistory*>(preStep->GetTouchable());
    const G4TouchableHistory* postTHist = static_cast<const G4TouchableHistory*>(postStep->GetTouchable());
    const int preStepVolDepth = preTHist->GetHistoryDepth();
    const int postStepVolDepth = postTHist->GetHistoryDepth();

    for (auto& recEnvelope : m_recordingEnvelopes)
    {
      const int envelopeLevel = recEnvelope.GetLevel();
      if (envelopeLevel <= preStepVolDepth)
      {
        //NB  preTHist->GetVolume(preStepVolDepth) would just give us the World volume.
        const G4LogicalVolume* logicalVolume1 =
          preTHist->GetVolume(preStepVolDepth-envelopeLevel)->GetLogicalVolume();
        if (logicalVolume1 != recEnvelope.GetLogicalVolume()) continue;

        if (envelopeLevel <= postStepVolDepth &&
            logicalVolume1 == postTHist->GetVolume(postStepVolDepth-envelopeLevel)
                              ->GetLogicalVolume())
        {
          continue;
        }

        // We have a track crossing a recording envelope
        // volume boundary, so make a TrackRecord
        recEnvelope.AddTrackRecord(aStep);

        // Done with this volume.
        break;
      }
    }
  }

  void MCTruthSteppingAction::createTruthIncident(const G4Step* aStep) const
  {
    const AtlasDetDescr::AtlasRegion geoID =
      iGeant4::ISFG4GeoHelper::nextGeoId(aStep, m_subDetVolLevel, &m_geoIDSvc);
    iGeant4::Geant4TruthIncident truth(aStep, geoID);
    m_truthRecordSvc.registerTruthIncident(truth);
  }

  void MCTruthSteppingAction::propagatePrimaryInfoToSecondaries(const std::vector<const G4Track*>& secondaries,
                                                                 TrackHelper& parentHelper) const
  {
    TrackInformation* parentInfo = parentHelper.GetTrackInformation();
    if (!parentInfo) {
      return;
    }
    HepMC::GenParticlePtr primaryGenParticle = parentInfo->GetPrimaryGenParticle();
    if (!primaryGenParticle) {
      return;
    }

    for (const G4Track* secondary : secondaries) {
      if (!secondary || secondary->GetUserInformation()) {
        continue;
      }

      auto trackInfo = std::make_unique<TrackInformation>();
      trackInfo->SetPrimaryGenParticle(primaryGenParticle);
      trackInfo->SetClassification(TrackInformation::Secondary);
      secondary->SetUserInformation(trackInfo.release());
    }
  }

} // namespace G4UA
