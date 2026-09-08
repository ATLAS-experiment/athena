/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Reco_Vertex.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_Reco_Vertex_H
#define DERIVATIONFRAMEWORK_Reco_Vertex_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "JpsiUpsilonTools/ICandidateSearch.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework {

  class Reco_Vertex : public AthReentrantAlgorithm {
    public:
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      virtual StatusCode initialize();

      virtual StatusCode execute(const EventContext& ctx) const;

    private:
      /** tools
       */
      PublicToolHandle<Trk::V0Tools> m_v0Tools{this, "V0Tools", "Trk::V0Tools"};
      ToolHandle<Analysis::ICandidateSearch> m_SearchTool{this, "VertexSearchTool", ""};
      PublicToolHandle<Analysis::PrimaryVertexRefitter> m_pvRefitter{this, "PVRefitter", "Analysis::PrimaryVertexRefitter"};
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
      /** job options
       */
      SG::WriteHandleKey<xAOD::VertexContainer> m_outputVtxContainerName{this, "OutputVtxContainerName", "OniaCandidates"};
      SG::ReadHandleKey<xAOD::VertexContainer> m_pvContainerName{this, "PVContainerName", "PrimaryVertices"};
      SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName{this, "RefPVContainerName", "RefittedPrimaryVertices"};
      Gaudi::Property<bool> m_refitPV{this, "RefitPV", false};
      Gaudi::Property<int> m_PV_max{this, "MaxPVrefit", 1000};
      Gaudi::Property<int> m_DoVertexType{this, "DoVertexType", 7};
      Gaudi::Property<size_t> m_PV_minNTracks{this, "MinNTracksInPV", 0};// minimum number of tracks for PV to be considered for PV association
      Gaudi::Property<bool> m_do3d{this, "Do3d", false};
      Gaudi::Property<bool> m_checkCollections{this, "CheckCollections", false};
      SG::ReadHandleKeyArray<xAOD::VertexContainer> m_CollectionsToCheck{this, "CheckVertexContainers" , {}};
      SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers{this, "RelinkTracks", {}, "Track Containers if they need to be relinked through indirect use" };
      SG::ReadHandleKeyArray<xAOD::MuonContainer> m_RelinkMuons{this, "RelinkMuons", {}, "Muon Containers if they need to be relinked through indirect use" };
  };
}

#endif // DERIVATIONFRAMEWORK_Reco_Vertex_H
