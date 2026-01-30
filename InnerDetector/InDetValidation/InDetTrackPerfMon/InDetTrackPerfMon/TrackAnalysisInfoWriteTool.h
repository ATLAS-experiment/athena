/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISINFOWRITETOOL_H
#define INDETTRACKPERFMON_TRACKANALYSISINFOWRITETOOL_H

/**
 * @file    TrackAnalysisInfoWriteTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    08 July 2024
 * @brief   Tool to write Track Analysis Info to StoreGate
 */

/// Athena includes
#include "AsgTools/AsgTool.h"
#include "StoreGate/WriteHandle.h"

/// EDM includes
#include "xAODCore/BaseContainer.h"

/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"


namespace IDTPM {

  /// useful typedefs
  template< typename coll_t >
  using EL_t = ElementLink< coll_t >;

  template< typename coll_t >
  using VecEL_t = std::vector< EL_t< coll_t > >;

  template< typename coll_t >
  using Vec_t = std::vector< typename coll_t::const_value_type >;

  using VecF_t = std::vector< float >;

  template< typename Tcoll_t, typename Rcoll_t >
  using MatchInfo_t = std::tuple< VecEL_t<Tcoll_t>, VecEL_t<Rcoll_t>, VecF_t >;

  /// Forward declaration(s)
  class TrackAnalysisCollections;
  class ITrackMatchingLookup;


  class TrackAnalysisInfoWriteTool :
      public virtual asg::IAsgTool, 
      public asg::AsgTool {

    ASG_TOOL_CLASS( TrackAnalysisInfoWriteTool, IAsgTool );

  public:

    /// Constructor
    TrackAnalysisInfoWriteTool( const std::string& name );

    /// Destructor
    virtual ~TrackAnalysisInfoWriteTool() = default;

    /// RoiSelectionTool methods
    virtual StatusCode initialize() override;

    /// Main writing function
    StatusCode write(
        SG::WriteHandle< xAOD::BaseContainer >& wh,
        TrackAnalysisCollections& trkAnaColls,
        const std::string& chain = "",
        unsigned int roiIdx = 0,
        const std::string& roiStr = "" ) const;

    /// Print info (for debug)
    std::string printInfo(
        SG::WriteHandle< xAOD::BaseContainer >& wh ) const;

  private:

    /// getMatchInfo
    template< typename Tcoll_t, typename Rcoll_t >
    MatchInfo_t< Tcoll_t, Rcoll_t > getMatchInfo(
        const Vec_t< Tcoll_t >& Tvec,
        const Vec_t< Rcoll_t >& Rvec,
        const Tcoll_t* Tcoll,
        const Rcoll_t* Rcoll,
        const ITrackMatchingLookup* matches ) const;

    /// Print match info (for debug)
    template< typename Tcoll_t, typename Rcoll_t >
    std::string printMatchInfo(
        const VecEL_t< Tcoll_t >& testVec,
        const VecEL_t< Rcoll_t >& refVec,
        const VecF_t& distVec ) const;

    StringProperty m_anaTag{ this, "AnaTag", "", "Track analysis tag" };

    SmartIF<ITrackAnalysisDefinitionSvc> m_trkAnaDefSvc;
 
  }; // class TrackAnalysisInfoWriteTool

} // namespace IDTPM

#include "InDetTrackPerfMon/TrackAnalysisInfoWriteTool.icc"

#endif // > !INDETTRACKPERFMON_TRACKANALYSISINFOWRITETOOL_H
