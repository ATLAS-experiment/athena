/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Reco_Vertex.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_Reco_Vertex_H
#define DERIVATIONFRAMEWORK_Reco_Vertex_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/ICandidateSearch.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework {

  class Reco_Vertex : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      Reco_Vertex(const std::string& t, const std::string& n, const IInterface* p);

      virtual StatusCode initialize();
      
      virtual StatusCode addBranches() const;
      
    private:
      /** tools
       */
      ToolHandle<Trk::V0Tools>                    m_v0Tools;
      ToolHandle<Analysis::ICandidateSearch>      m_SearchTool;
      ToolHandle<Analysis::PrimaryVertexRefitter> m_pvRefitter;
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
      /** job options
       */
      SG::WriteHandleKey<xAOD::VertexContainer> m_outputVtxContainerName;
      SG::ReadHandleKey<xAOD::VertexContainer> m_pvContainerName;
      SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName;
      bool        m_refitPV;
      int         m_PV_max;
      int         m_DoVertexType;
      size_t      m_PV_minNTracks;
      bool        m_do3d;
      bool        m_checkCollections;
      SG::ReadHandleKeyArray<xAOD::VertexContainer> m_CollectionsToCheck;
      SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers{this, "RelinkTracks", {}, "Track Containers if they need to be relinked through indirect use" };
      SG::ReadHandleKeyArray<xAOD::MuonContainer> m_RelinkMuons{this, "RelinkMuons", {}, "Muon Containers if they need to be relinked through indirect use" };
  }; 
}

#endif // DERIVATIONFRAMEWORK_Reco_Vertex_H
