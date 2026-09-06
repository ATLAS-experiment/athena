/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTruthBase_MCTruthSteppingActionTool_h
#define MCTruthBase_MCTruthSteppingActionTool_h

// Local includes
#include "MCTruthSteppingAction.h"

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

// ISF includes
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Interfaces/ITruthSvc.h"

// STL includes
#include <string>

namespace G4UA
{

  /// @class MCTruthSteppingActionTool
  /// @brief A tool for managing the MCTruthSteppingAction
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class MCTruthSteppingActionTool : public UserActionToolBase<MCTruthSteppingAction>
  {

    public:

      /// Standard constructor
      MCTruthSteppingActionTool(const std::string& type, const std::string& name,
                                const IInterface* parent);

      /// Initialize the tool
      virtual StatusCode initialize() override final;

    protected:

      /// Setup the user action for current thread
      virtual std::unique_ptr<MCTruthSteppingAction>
      makeAndFillAction(G4AtlasUserActions&) override final;

    /// Calls BeginOfAthenaEvent
    StatusCode BeginOfAthenaEvent(HitCollectionMap&) override;
    /// Calls EndOfAthenaEvent
    StatusCode EndOfAthenaEvent(HitCollectionMap&) override;

    private:

      /// Map of volume name to output collection name
      Gaudi::Property<MCTruthSteppingAction::VolumeCollectionMap_t>
        m_volumeCollectionMap{this, "VolumeCollectionMap", {},
                              "Map of volume name to output collection name"};

      /// The saving level for secondaries
      Gaudi::Property<int> m_secondarySavingLevel{
        this, "SecondarySavingLevel", 2,
        "Three valid options: 1 - Primaries; 2 - StoredSecondaries(default); 3 - All"};

      /// The level in the G4 volume hierarchy at which we find the sub-detector
      Gaudi::Property<int> m_subDetVolLevel{
        this, "SubDetVolumeLevel", 1,
        "The level in the G4 volume hierarchy at which we find the sub-detector name"};

      /// Central Truth Service
      ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{
        this, "TruthRecordSvc", "ISF_TruthRecordSvc", "ISF Particle Truth Service"};

      /// Geo ID Service
      ServiceHandle<ISF::IGeoIDSvc> m_geoIDSvc{
        this, "GeoIDSvc", "ISF_GeoIDSvc", "ISF GeoID Service"};

  }; // class MCTruthSteppingActionTool

} // namespace G4UA

#endif
