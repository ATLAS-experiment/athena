/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_VERTEXROISELECTIONTOOL_H
#define INDETTRACKPERFMON_VERTEXROISELECTIONTOOL_H

/**
 * @file    VertexRoiSelectionTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    04 December 2024
 * @brief   Tool to select reco && truth vertices
 *          in given RoI
**/

/// Athena include(s)
#include "AsgTools/IAsgTool.h"
#include "AsgTools/AsgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "AthLinks/ElementLink.h"

/// STD includes
#include <string>
#include <vector>

class TrigRoiDescriptorCollection;

namespace IDTPM {

  class TrackAnalysisCollections;

  class VertexRoiSelectionTool :
      public virtual asg::IAsgTool,
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( VertexRoiSelectionTool, IAsgTool );

    /// Constructor
    VertexRoiSelectionTool( const std::string& name );

    /// Destructor
    virtual ~VertexRoiSelectionTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Main Vertex selection method
    StatusCode selectVerticesInRoI(
        TrackAnalysisCollections& trkAnaColls,
        const ElementLink< TrigRoiDescriptorCollection >& roiLink );
  
    /// geometric RoI filters - for non-trigger veritices (e.g. offline, truth, etc.)
    template< class V >
    bool accept( const V& v, const TrigRoiDescriptor* r ) const;

    /// vertex getter function (for offline tracks || truth particles)
    template< class V >
    std::vector< const V* > getVertices(
        const std::vector< const V* >& vvec,
        const TrigRoiDescriptor* r ) const;

    /// TrigDecTool- && EventView-based getter function for trigger vertices
    std::vector< const xAOD::Vertex* > getTrigVertices( 
        const std::vector< const xAOD::Vertex* >& vvec,
        const ElementLink< TrigRoiDescriptorCollection >& roiLink ) const;

  private:

    /// Trigger vertex container's name
    SG::ReadHandleKey< xAOD::VertexContainer > m_triggerVertexContainerName {
        this, "TriggerVertexContainerName", "", "Name of container of trigger vertices" };

    /// TrigDecTool
    PublicToolHandle< Trig::TrigDecisionTool > m_trigDecTool {
        this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", "" };

  }; // class VertexRoiSelectionTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_VERTEXROISELECTIONTOOL_H
