/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_AMBIGUITYRESOLUTIONALG_H
#define ACTSTRKFINDING_AMBIGUITYRESOLUTIONALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

// ACTS
#include "ActsEvent/TrackContainer.h"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"

#include <string>
#include <memory>

namespace ActsTrk
{

  class AmbiguityResolutionAlg : public AthReentrantAlgorithm
  {
  public:
    AmbiguityResolutionAlg(const std::string &name,
                           ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "", "Monitoring tool"};

    SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksKey
       {this, "TracksLocation", "ActsTracks", "Input track collection"};
    ActsTrk::MutableTrackContainerHandlesHelper m_resolvedTracksBackendHandles;
    SG::WriteHandleKey<ActsTrk::TrackContainer> m_resolvedTracksKey
       {this, "ResolvedTracksLocation", "ActsTracksResolved", "Ambiguity resolved output track collection"};

    Gaudi::Property<unsigned int> m_maximumSharedHits
       {this, "MaximumSharedHits", 3u, "Maximum number of shared hits per track."};
    Gaudi::Property<unsigned int> m_maximumIterations
       {this, "MaximumIterations", 10000u, "Maximum number of iterations to resolve ambiguities among all tracks."};
    Gaudi::Property<unsigned int> m_nMeasurementsMin
       {this, "NMeasurementsMin", 7u, "Minimum number of measurements per track."};

    std::unique_ptr<Acts::GreedyAmbiguityResolution> m_ambi;

  };

} // namespace

#endif
