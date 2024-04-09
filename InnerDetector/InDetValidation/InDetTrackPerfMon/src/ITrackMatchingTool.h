/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_ITRACKMATCHINGTOOL_H
#define INDETTRACKPERFMON_ITRACKMATCHINGTOOL_H

/**
 * @file ITrackMatchingTool.h
 * @brief interface for track matching tools in this package
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 21 March 2024
**/

/// Athena includes
#include "AsgTools/IAsgTool.h"

/// EDM includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"

/// STL includes
#include <string>


namespace IDTPM {

  /// Forward-declaring internal classes
  class TrackAnalysisCollections;
  class ITrackMatchingLookup;

  class ITrackMatchingTool : virtual public asg::IAsgTool {

  public:

    ASG_TOOL_INTERFACE( IDTPM::ITrackMatchingTool );

    /// General matching method, via TrackAnalysisCollections
    virtual StatusCode match(
        TrackAnalysisCollections& trkAnaColls,
        const std::string& chainRoIName ) const = 0;

    /// Specific matching method, via test/reference vectors
    /// -> Could be used independently of the
    ///    TrackAnalysis infrastructure
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >& vTest,
        const std::vector< const xAOD::TruthParticle* >& vRef,
        ITrackMatchingLookup& matches ) const = 0;

  };

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_ITRACKMATCHINGTOOL_H
