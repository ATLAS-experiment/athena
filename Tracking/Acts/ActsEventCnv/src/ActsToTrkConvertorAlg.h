/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_ACTSTOTRK_CONVERTER_ALG_H
#define ACTSEVENTCNV_ACTSTOTRK_CONVERTER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTrack/TrackCollection.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "Acts/Geometry/GeometryContext.hpp"

namespace ActsTrk
{

  class ActsToTrkConvertorAlg: public AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~ActsToTrkConvertorAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    StatusCode makeTracks(const EventContext &ctx,
                          const Acts::GeometryContext &tgContext,
                          const ActsTrk::TrackContainer &tracks,
                          ::TrackCollection &tracksContainer) const;

    std::unique_ptr<Trk::MeasurementBase>
    makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
                    const Trk::TrackParameters &parm) const;

  private:
    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};
    ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool{this, "BoundaryCheckTool", "InDet::InDetBoundaryCheckTool", "Boundary checking tool for detector sensitivities"};
    ToolHandle<Trk::IRIO_OnTrackCreator> m_RotCreatorTool{this, "RotCreatorTool", "", "optional RIO_OnTrack creator tool"};
    ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool{this, "SummaryTool", "ToolHandle for track summary tool"};

    SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksContainerKey{this, "ACTSTracksLocation", "SiSPSeededActsTrackContainer",
                                                                         "Output track collection (ActsTrk variant)"};
    SG::WriteHandleKey<::TrackCollection> m_tracksKey{this, "TracksLocation", "SiSPSeededActsTracks",
                                                      "Output track collection"};
  };

}

#endif
