/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_VERTEXQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_VERTEXQUALITYSELECTIONTOOL_H

/**
 * @file    VertexQualitySelectionTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    04 December 2024
 * @brief   Tool to handle all required reco && truth
 *          vertex quality selections in this package
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "IVertexSelectionTool.h"

/// STD includes
#include <string>


namespace IDTPM {

  class VertexQualitySelectionTool : 
      public virtual IDTPM::IVertexSelectionTool,  
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( VertexQualitySelectionTool, IVertexSelectionTool );
   
    /// Constructor 
    VertexQualitySelectionTool( const std::string& name );

    /// Destructor
    virtual ~VertexQualitySelectionTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Main Vertex selection method
    virtual StatusCode selectVertices(
        TrackAnalysisCollections& trkAnaColls ) override;

  private:

    /// TODO: include extra selection tools && properties here

  }; // class VertexQualitySelectionTool

} // namespace IDTPM



#endif // > ! INDETTRACKPERFMON_VERTEXQUALITYSELECTIONTOOL_H
