/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_IVERTEXSELECTIONTOOL_H
#define INDETTRACKPERFMON_IVERTEXSELECTIONTOOL_H

/**
 * @file    IVertexSelectionTool.h
 * @brief   header file for interface for all the
 *          vertex selection tools in this package
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    04 December 2024
**/

/// Athena includes
#include "AsgTools/IAsgTool.h"

namespace IDTPM {

  class TrackAnalysisCollections;


  class IVertexSelectionTool : virtual public asg::IAsgTool {

  public:

    ASG_TOOL_INTERFACE( IDTPM::IVertexSelectionTool )

    virtual StatusCode selectVertices(
        TrackAnalysisCollections& trkAnaColls ) = 0;

  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_IVERTEXSELECTIONTOOL_H
