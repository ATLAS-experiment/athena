// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H
#define TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>

#include "TrackingAnalysisAlgorithms/PixelDEdxUtils.h"

#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/TrackParticleAuxContainer.h>
#include "xAODTracking/TrackStateValidation.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include <AsgTools/PropertyWrapper.h>
#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadHandleKeyArray.h>
#include <AsgDataHandles/ReadDecorHandleKeyArray.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadHandle.h>
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "TrkAnalysisInterfaces/IPixelDEdxEqualizationTool.h"

#include <string>
#include <vdt/vdtMath.h> // for RDataFrame

#include <TString.h>
#include <ROOT/RDataFrame.hxx>

namespace CP {

  /// \brief an algorithm to calculate equalized pixel dE/dx. 

  /// Algorithm to decorate tracks with corrected pixel dE/dx measurements.
  ///
  /// Scale factors are applied to either track-level truncatd mean dE/dx...
  /// or to individual pixel cluster dE/dx measurements.
  /// These scale factors equalize the dE/dx measurments throughout time (lumi).
  ///
  /// @author Ian Dyckes
  ///
  class PixelDEdxEqualizationAlg final : public EL::AnaReentrantAlgorithm {

  public:
    /// Algorithm constructor
    PixelDEdxEqualizationAlg( const std::string& name, ISvcLocator* svcLoc );

    /// Function initialising the algorithm
    StatusCode initialize() override;

    /// Function executing the algorithm
    StatusCode execute(const EventContext& ctx) const override;
    
  private:

    PixelDEdx::PixelClusterStruct getPixelClusterStruct(const xAOD::TrackMeasurementValidation* pixclus, const xAOD::TrackStateValidation* msos) const;
    
    ToolHandle<CP::IPixelDEdxEqualizationTool> m_pixelDEdxEqualizationTool{this, "PixelDEdxEqualizationTool", "", "tool for pixel dE/dx"};

    /// @name Algorithm properties
    /// @{
    // Declare the algorithm's properties:

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "event info key"};

    /// Input track collection to decorate
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerName {
    this, "TrackContainerName", "InDetTrackParticles", "Input track collection to decorate with corrected dE/dx measurements."};

    /// Name of link from tracks to MSOSs.
    Gaudi::Property<std::string> m_msosLink
    { this, "MSOSLink", "Reco_msosLink"};

    /// Flags
    Gaudi::Property<bool> m_equalizeTrackMeasurements
    { this, "EqualizeTrackMeasurements", false, "Equalize track-level truncated mean dE/dx"};
    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, "Equalize cluster dE/dx before truncated mean"};
    Gaudi::Property<bool> m_tightClusterCleaning
    { this, "TightClusterCleaning", false, "Apply tight cluster cleaning requirements (e.g. cluster size/shape cuts)"};

    //////////////////
    /// Decorators ///
    //////////////////

    /// Equalized truncated mean dE/dx of the track 
    /// Declare WriteDectorHandleKey but set dynamically in initialize() once track container is known...
    /// And once the equalization strategy is known (cluster- or track-level).
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackdEdxEqKey
      {this, "TrackdEdxDecorKey", "", "SG key for the equalized truncated mean dE/dx decoration" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackdEdxEqStdDevKey
      {this, "TrackdEdxStdDevDecorKey", "", "SG key for the equalized truncated standard deviation dE/dx decoration" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackdEdxEqNUsedKey
      {this, "TrackdEdxNUsedDecorKey", "", "SG key for decorating track with number of used hits in dE/dx truncated mean." };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackdEdxEqIBLOFKey
      {this, "TrackdEdxIBLOFDecorKey", "", "SG key for decorating track with number of good IBL hits in overflow." };

    /// Only one PixelClusters container shared by all track containers, so should not need to modify keys...
    /// Raw cluster dE/dx
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxKey
      {this, "clusterdEdxKey", "PixelClusters.dEdx", "SG key for the raw pixel cluster dE/dx attribute"};
    /// Equalized cluster dE/dx:
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxEqKey
      {this, "clusterdEdxEqKey", "PixelClusters.dEdxEq", "SG key for the equalized pixel cluster dE/dx attribute"};

    /// Counters.  Maybe drop?
    mutable std::atomic<unsigned long> m_nEventsProcessed{};
    mutable std::atomic<unsigned long> m_nTracksProcessed{};

  }; // class PixelDEdxEqualizationAlg

} // namespace CP

#endif // TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

