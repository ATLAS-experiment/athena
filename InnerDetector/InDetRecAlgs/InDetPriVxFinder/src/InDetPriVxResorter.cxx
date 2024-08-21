/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                         InDetPriVxResorter.cxx  -  Description
                             -------------------
    begin   : 15-08-2024
    authors : Teng Jian Khoo
    email   : khoo@cern.ch
    changes :
 ***************************************************************************/
#include "InDetPriVxFinder/InDetPriVxResorter.h"

#include <memory>
#include "xAODTracking/VertexContainer.h"

namespace InDet
{

InDetPriVxResorter::InDetPriVxResorter
 (const std::string& name,ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator)
 { }


  StatusCode InDetPriVxResorter::initialize()
  {

    if ( m_VertexCollectionSortingTool.retrieve().isFailure() )
    {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_VertexCollectionSortingTool);
      return StatusCode::FAILURE;
    }
   
    ATH_CHECK(m_verticesInKey.initialize());
    ATH_CHECK(m_verticesOutKey.initialize());
    return StatusCode::SUCCESS;
  }


  StatusCode InDetPriVxResorter::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::VertexContainer> inputVertices (m_verticesInKey, ctx);
    SG::WriteHandle<xAOD::VertexContainer> outputVertices (m_verticesOutKey, ctx);

    auto vertexContainerPair = m_VertexCollectionSortingTool->sortVertexContainer(*inputVertices);
    ATH_CHECK(outputVertices.record(std::unique_ptr<xAOD::VertexContainer>(vertexContainerPair.first),std::unique_ptr<xAOD::VertexAuxContainer>(vertexContainerPair.second)));

    if(msgLevel()==MSG::VERBOSE) {
      for(size_t ivx=0; ivx<inputVertices->size(); ++ivx) {
        ATH_MSG_VERBOSE(
          "Ntracks for original (resorted) vertex " << ivx << ": "
          << inputVertices->at(ivx)->nTrackParticles()
          << "(" << outputVertices->at(ivx)->nTrackParticles() << ")"
        );
      }
    }

    return StatusCode::SUCCESS;
  }
  
} // end namespace InDet
