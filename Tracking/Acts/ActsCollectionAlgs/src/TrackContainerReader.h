/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H
#define ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsEvent/TrackContainer.h"

// STL includes
#include <string>

/**
 * @class TrackContainerReader
 * @brief 
 **/
namespace ActsTrk { 
class TrackContainerReader : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;  
  virtual ~TrackContainerReader() override = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& context) const override final;
private:
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  ActsTrk::ConstTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_tracksKey{this, "TrackContainer", "TrackContainer"};
};
}
#endif // ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H
