/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_ITRACKSELECTIONTOOL_H
#define INDETTRACKPERFMON_ITRACKSELECTIONTOOL_H

/**
 * @file    ITrackSelectionTool.h
 * @brief   header file for interface for all the various
 *          track selection tools in this package
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    02 October 2023
**/

/// Athena includes
#include "AsgTools/IAsgTool.h"

namespace IDTPM {

  class TrackAnalysisCollections;


  class ITrackSelectionTool : virtual public asg::IAsgTool {

  public:

    ASG_TOOL_INTERFACE( IDTPM::ITrackSelectionTool )

    virtual StatusCode selectTracks(
        TrackAnalysisCollections& trkAnaColls ) = 0;

  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_ITRACKSELECTIONTOOL_H
