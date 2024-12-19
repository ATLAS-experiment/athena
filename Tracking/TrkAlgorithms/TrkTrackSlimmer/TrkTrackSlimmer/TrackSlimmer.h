/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSlimmer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKTRACKSLIMMER_TRK_TRACKSLIMMER_H
#define TRKTRACKSLIMMER_TRK_TRACKSLIMMER_H

// Gaudi includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkToolInterfaces/ITrackSlimmingTool.h"

#include <atomic>

namespace Trk {

/** @class TrackSlimmer

Simple algorithm to load tracks and use Trk::ITrackSlimmingTool to e.g. strip
them down to hits + perigee parameters for use in AOD.

@author  Edward Moyse <Edward.Moyse@cern.ch>
@autho   Christos Anastopoulos (Athena MT)
*/

class TrackSlimmer final : public AthReentrantAlgorithm
{
public:
  /** Standard Athena-Algorithm Constructor */
  TrackSlimmer(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  ~TrackSlimmer();

  /** standard Athena-Algorithm method */
  StatusCode initialize() override;
  /** standard Athena-Algorithm method */
  StatusCode execute(const EventContext& ctx) const override;
  /** standard Athena-Algorithm method */
  StatusCode finalize() override;

private:
  /** member variables for algorithm properties: */
  // int/double/bool  m_propertyName;
  ToolHandle<ITrackSlimmingTool> m_slimTool{this, "TrackSlimmingTool",
      "Trk::TrkTrackSlimmingTool/TrkTrackSlimmingTool"};
  SG::ReadHandleKeyArray<TrackCollection> m_trackLocation
    {this, "TrackLocation", {"ConvertedMooreTracks"}};
};
} // end of namespace

#endif
