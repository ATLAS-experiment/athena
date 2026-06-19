/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                          InDetSecVtxFinder.h  -  Description
                             -------------------
    begin   : Nov 10 2016
    authors : Lianyou SHAN ( IHEP-Beijing ) 
    Base    : InDetPriVxFinder, only an interface for reconstructed PriVtx to a SecVtxTool
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    author of most recent changes : Neža Ribarič (Lancaster University, UK)
    changes : Added functionality to call multiple Finder Tools 
 ***************************************************************************/
#include "CxxUtils/checker_macros.h"

#ifndef INDETSECVXFINDER_INDETSECVXFINDER_H
#define INDETSECVXFINDER_INDETSECVXFINDER_H
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "InDetRecToolInterfaces/IInDetAdaptiveMultiSecVtxFinderTool.h"

/* Forward declarations */


namespace InDet
{
  class IAdaptiveMultiSecVertexFinder;
  
  class InDetSecVtxFinder : public AthAlgorithm
  {
  public:
    InDetSecVtxFinder(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~InDetSecVtxFinder() = default;
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();
  private: 

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticles{this,"inputTrackParticles","InDetTrackParticles","xAOD::TrackParticle Container used in Vertexing"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_outputSecondaryVertices{this,"outputSecondaryVertices","AdaptiveMultiSecVtx","Output Secondary Vertex Container"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_inputPrimaryVertices{this,"inputPrimaryVertices","PrimaryVertices","Input Primary Vertex Container"};
    
    ToolHandle<InDet::IAdaptiveMultiSecVertexFinder> m_AdaptiveMultiVertexFinderTool{this, "AdaptiveMultiVertexFinderTool", "InDet::InDetAdaptiveMultiSecVtxFinderTool", "adaptive multi secondary vertex finder tool"};
    
    // for summary output at the end
    unsigned int m_numEventsProcessed;
    unsigned int m_totalNumVerticesWithoutDummy;
  };
}
#endif
