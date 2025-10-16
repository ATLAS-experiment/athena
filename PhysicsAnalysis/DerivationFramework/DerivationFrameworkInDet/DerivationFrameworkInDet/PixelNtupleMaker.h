/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_PIXELNTUPLEMAKER_H
#define DERIVATIONFRAMEWORK_PIXELNTUPLEMAKER_H


#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include "Gaudi/Property.h"

namespace DerivationFramework {

  class PixelNtupleMaker : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      PixelNtupleMaker(const std::string& t, const std::string& n, const IInterface* p);

      ~PixelNtupleMaker();

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches(const EventContext& ctx) const;
      static void GetLayerEtaPhiFromId(uint64_t id,int *barrelEC, int *layer, int *eta, int *phi);

    private:
      Gaudi::Property<int> m_storeMode
      {this, "StoreMode", 1, "Storing mode: 1:full, 2:small, 3:Z->tautau"};

      ToolHandle<InDet::IInDetTrackSelectionTool> m_selector
      {this, "TrackSelectionTool","InDet::InDetTrackSelectionTool/TrackSelectionTool"}; // @TODO should not have a default value, since there is not generally correct default

      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_containerKey
      { this, "ContainerName", "InDetTrackParticles", "" };
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurementContainerKey
      { this, "MeasurementValidationKey","PixelClusters", ""};

      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_monitoringTracks
      { this, "PixelMonitoringTracksKey", "PixelMonitoringTrack","" };

      typedef std::vector<ElementLink< xAOD::TrackStateValidationContainer > > MeasurementsOnTrack;
      typedef std::vector<ElementLink< xAOD::TrackStateValidationContainer > >::const_iterator MeasurementsOnTrackIter;
  }; 
}

#endif
