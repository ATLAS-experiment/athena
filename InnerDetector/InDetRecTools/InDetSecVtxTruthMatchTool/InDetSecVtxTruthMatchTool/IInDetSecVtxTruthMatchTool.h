/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IInDetSecVtxTruthMatchTool_h
#define IInDetSecVtxTruthMatchTool_h

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODTracking/VertexContainer.h"
#include "xAODTruth/TruthVertexContainer.h"


/** Class for vertex truth matching.
 * Match reconstructed vertices to truth level interactions vertices
 * through the chain: track -> particle -> genEvent -> genVertex
 * Categorize reconstructed vertices depending on their composition.
 */

class IInDetSecVtxTruthMatchTool : public virtual asg::IAsgTool {

ASG_TOOL_INTERFACE( IInDetSecVtxTruthMatchTool )

public:

//take const collection of vertices, match them, and decorate with matching info
 virtual StatusCode matchVertices(  std::vector<const xAOD::Vertex*> recoVerticesToMatch, 
                                    std::vector<const xAOD::TruthVertex*> truthVerticesToMatch, 
                                    const xAOD::TrackParticleContainer* trackParticles ) = 0;

};

#endif
