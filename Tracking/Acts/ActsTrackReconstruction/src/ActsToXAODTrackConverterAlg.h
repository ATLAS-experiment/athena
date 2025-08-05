/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ACTSTOXAOD_TRACKCONVERTERALG_H
#define ACTSTRACKRECONSTRUCTION_ACTSTOXAOD_TRACKCONVERTERALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/PersistentTrackContainer.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"

namespace ActsTrk {

  class ActsToXAODTrackConverterAlg
    : public AthReentrantAlgorithm {
    public:
    ActsToXAODTrackConverterAlg(const std::string &name,
                                ISvcLocator *pSvcLocator);
    virtual ~ActsToXAODTrackConverterAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey< ActsTrk::TrackContainer > m_inputTrackContainerKey {this, "InputActsTracksLocation", ""};
    SG::WriteHandleKey< ActsTrk::PersistentTrackContainer > m_outputTrackContainerKey {this, "OutputActsTracksLocation", ""};

    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};
    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  };

} // namespace

#endif
