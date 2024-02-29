/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H
#define ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"

#include "ActsEvent/TrackContainerHandle.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

// STL includes
#include <string>

/**
 * @class TrackContainerReader
 * @brief 
 **/
namespace ActsTrk { 
class TrackContainerReader : public AthReentrantAlgorithm {
public:
  TrackContainerReader(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackContainerReader() override = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& context) const override final;
  virtual StatusCode finalize() override;

private:
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  ActsTrk::ConstTrackContainerHandle<TrackContainerReader> m_handleKey{this, "", "SiSPSeededActsTrack"};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_tracksKey{this, "TrackContainer", "TrackContainer"};
};
}
#endif // ACTSCOLLECTIONALGS_TRACKCONTAINERREADER_H
