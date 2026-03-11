/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_STABLEMATCHINGBASE_H
#define INDETTRACKPERFMON_STABLEMATCHINGBASE_H

/**
 * @file   StableMatchingTool.h
 * @author Harry Simpson <harry.simpson@cern.ch>
 * @date   30 March 2024
 * @brief  Base class to perform matching of tracks based on the stable marriage algorithm
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "InDetTrackPerfMon/ITrackMatchingTool.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/TrackMatchingLookup.h"

#include <vector>
#include <algorithm>

namespace IDTPM {

  /// ------------------------------------------
  /// --------- Base (templated) class ---------
  /// ------------------------------------------
  template< typename T, typename R=T > 
  class StableMatchingBase : public asg::AsgTool {
  public:

    /// Constructor
    StableMatchingBase( const std::string& name ) :
        asg::AsgTool( name ) { };

    /// matchVectors
    virtual StatusCode matchVectors(const std::vector< const T* >& vTest,
				    const std::vector< const R* >& vRef,
				    ITrackMatchingLookup& matches ) const;

    // Override this in derived classes
    virtual float distance(const T& t, const R& r) const = 0;

  protected:

    // Helper methods (implemented in StableMatchingBase.icc)
    void buildDistanceMatrix(const std::vector<const T*>& vTest,
			     const std::vector<const R*>& vRef,
			     std::vector<std::vector<float>>& dist) const;

    void buildPreferenceLists(const std::vector<std::vector<float>>& dist,
			      std::vector<std::vector<int>>& testPrefs,
			      std::vector<std::vector<int>>& refRankings) const;

    void galeShapley(const std::vector<std::vector<int>>& testPrefs,
		     const std::vector<std::vector<int>>& refRankings,
                     const std::vector<std::vector<float>>& dist,
                     std::vector<int>& testMatch,
                     std::vector<int>& refMatch) const;
  };

} // namespace IDTPM

#include "InDetTrackPerfMon/StableMatchingBase.icc"
#endif // > !INDETTRACKPERFMON_STABLEMATCHINGBASE_H
