/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                          InDetPriVxResorter.h  -  Description
                             -------------------
    begin   : 15-08-2024
    authors : Teng Jian Khoo
    email   : teng.jian.khoo@cern.ch
    changes :

 ***************************************************************************/

#ifndef INDETPRIVXRESORTER_INDETPRIVXRESORTER_H
#define INDETPRIVXRESORTER_INDETPRIVXRESORTER_H
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/VertexContainer.h"

#include "TrkVertexFitterInterfaces/IVertexCollectionSortingTool.h"

/** Primary Vertex Finder.
  InDetPriVxFinder uses the InDetPrimaryVertexFinderTool in the package
  InnerDetector/InDetRecTools/InDetPriVxFinderTool. It only gives the trackcollection from storegate to it
  and records the returned VxContainer.
 */

namespace InDet
{  
  class InDetPriVxResorter : public AthReentrantAlgorithm
  {
  public:
    InDetPriVxResorter(const std::string &name, ISvcLocator *pSvcLocator);
    
    virtual ~InDetPriVxResorter() = default;

    // Gaudi algorithm hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    SG::ReadHandleKey<xAOD::VertexContainer> m_verticesInKey{this,"VerticesIn","PrimaryVertices","Input Vertex Collection"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_verticesOutKey{this,"VerticesOut","PrimaryVertices_resorted","Output Vertex Collection"};
    ToolHandle<Trk::IVertexCollectionSortingTool > m_VertexCollectionSortingTool{this, "VertexCollectionSortingTool", "", "Vertex collection sorting tool"};

  };
}
#endif
