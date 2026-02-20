/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;
    static void GetLayerEtaPhiFromId(uint64_t id,int *barrelEC, int *layer, int *eta, int *phi);

  private:
    Gaudi::Property<int> m_storeMode
    {this, "StoreMode", 1, "Storing mode: 1:full, 2:small, 3:Z->tautau"};

    ToolHandle<InDet::IInDetTrackSelectionTool> m_selector
      {this, "TrackSelectionTool",""};

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
