/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTruthBase_MCTruthSteppingAction_H
#define MCTruthBase_MCTruthSteppingAction_H

// System includes
#include <map>
#include <string>
#include <vector>

// Framework includes
#include "AthenaBaseComps/AthMessaging.h"

// Geant4 includes
#include "G4UserEventAction.hh"
#include "G4UserSteppingAction.hh"

// Local includes
#include "RecordingEnvelope.h"

namespace ISF
{
  class IGeoIDSvc;
  class ITruthSvc;
}

namespace G4UA
{

  /// @class MCTruthSteppingAction
  /// @brief User action which handles recording-envelope truth tracks.
  ///
  /// This user action utilizes RecordingEnvelope objects to save truth tracks
  /// at entry/exit layers of certain configured detector layers.
  ///
  class MCTruthSteppingAction : public G4UserEventAction,
                                public G4UserSteppingAction,
                                public AthMessaging
  {

    public:

      /// Map of volume name to output TrackRecordCollection name
      using VolumeCollectionMap_t = std::map<std::string, std::string>;

      /// Construct the action with specified volumes and output collections.
      /// @param[in] collMap A map of recording envelope volume name to output
      /// TrackRecordCollection name.
      MCTruthSteppingAction(const VolumeCollectionMap_t& collMap,
                            int secondarySavingLevel,
                            int subDetVolLevel,
                            ISF::ITruthSvc& truthRecordSvc,
                            ISF::IGeoIDSvc& geoIDSvc,
                            IMessageSvc* msgSvc, MSG::Level level);

      /// Called at the start of each G4 event. Used to ensure that the
      /// TrackRecordCollection WriteHandles are valid.
      virtual void BeginOfEventAction(const G4Event*) override final;

      /// Process one particle step. If the step crosses a recording
      /// envelope volume boundary, passes the step to the corresponding
      /// RecordingEnvelope to add a TrackRecord.
      virtual void UserSteppingAction(const G4Step*) override final;

    private:

      /// Setup the list of RecordingEnvelope helpers
      void setupRecEnvelopes();

      /// Propagate parent primary truth attribution to ordinary secondaries.
      void propagatePrimaryInfoToSecondaries(const G4Step*) const;

      /// Create and register a truth incident for the current step.
      void createTruthIncident(const G4Step*) const;

      /// Used to delay initialization until the event loop, after geo is ready
      bool m_isInitialized;

      /// The saving level for secondaries
      int m_secondarySavingLevel;

      /// The level in the G4 volume hierarchy at which we find the sub-detector
      int m_subDetVolLevel;

      /// ISF Truth Service used to register truth incidents
      ISF::ITruthSvc& m_truthRecordSvc;

      /// ISF GeoID Service used to identify the next detector region
      ISF::IGeoIDSvc& m_geoIDSvc;

      /// Map of volume name to output collection name
      VolumeCollectionMap_t m_volumeCollectionMap;

      /// List of RecordingEnvelope helpers to invoke
      std::vector<RecordingEnvelope> m_recordingEnvelopes;

  }; // class MCTruthSteppingAction

} // namespace G4UA

#endif
