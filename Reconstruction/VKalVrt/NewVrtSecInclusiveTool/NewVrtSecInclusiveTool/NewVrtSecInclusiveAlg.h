/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NewVrtSecInclusiveAlg.h, (c) ATLAS Detector software
// author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)
///////////////////////////////////////////////////////////////////

/**
* Example algorithm to run the NewVrtSecInclusive algorithm and save results to StoreGate
*/

#ifndef VKalVrt_NewVrtSecInclusiveAlg_H
#define VKalVrt_NewVrtSecInclusiveAlg_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "NewVrtSecInclusiveTool/IVrtInclusive.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODJet/JetContainer.h"


namespace Rec {

   class NewVrtSecInclusiveAlg : public AthReentrantAlgorithm {
     public: 

       NewVrtSecInclusiveAlg( const std::string& name, ISvcLocator* pSvcLocator );

       StatusCode initialize() override;
       StatusCode execute(const EventContext &ctx) const override;
       StatusCode finalize() override;

    private:

      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tpContainerKey{this,"TrackParticleContainer","InDetTrackParticles","Read TrackParticle container"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_gsfContainerKey{this,"GSFTrackParticleContainer","GSFTrackParticles","Read GSFTrackParticle container"};
      SG::ReadHandleKey<xAOD::MuonContainer>          m_muonContainerKey{this,"MuonContainer","Muons","Read muon container"};
      SG::ReadHandleKey<xAOD::ElectronContainer>      m_electronContainerKey{this,"ElectronContainer","Electrons","Read electron container"};
      SG::ReadHandleKey<xAOD::VertexContainer>        m_pvContainerKey{this,"PrimaryVertexContainer","PrimaryVertices","Read PrimaryVertices container"};
      SG::ReadHandleKey<xAOD::JetContainer>           m_jetContainerKey{this,"JetContainer","AntiKt4EMPFlowJets","Read Jets container"};
      SG::ReadHandleKey<xAOD::VertexContainer>        m_btsvContainerKey{this,"BTagSVContainer","BTagging_AntiKt4EMPFlowSecVtx","Read BTagiingSV container"};

      SG::WriteHandleKey<xAOD::VertexContainer>  m_foundVerticesKey{this,"BVertexContainerName","AllBVertices","Found vertices container"};
      ToolHandle < Rec::IVrtInclusive >          m_bvertextool{this, "BVertexTool", "Rec::NewVrtSecInclusiveTool/SVTool"};

      Gaudi::Property<bool> m_addIDTracks{this, "AddIDTracks", true, "Set to true if you want to add ID tracks to the pool"};
      Gaudi::Property<bool> m_addGSFTracks{this, "AddGSFTracks", false, "Set to true if you want to add GSF tracks to the pool"};
      Gaudi::Property<bool> m_addMuonTracks{this, "AddMuonTracks", false, "Set to true if you want to add ID tracks from muons to the pool"};
      Gaudi::Property<bool> m_addElectronTracks{this, "AddElectronTracks", false, "Set to true if you want to add GSF tracks from electrons to the pool"};
      Gaudi::Property<bool> m_removeNonLepVerts{this, "RemoveNonLepVertices", false, "Set to true if you want to remove vertices with no associated lepton tracks"};

      void addInDetTracks(const EventContext &, std::unordered_set<const xAOD::TrackParticle*>&) const;
      void addGSFTracks(const EventContext &, std::unordered_set<const xAOD::TrackParticle*>&) const;
      void addMuonTracks(const EventContext &, std::unordered_set<const xAOD::TrackParticle*>&) const;
      void addElectronTracks(const EventContext &, std::unordered_set<const xAOD::TrackParticle*>&) const;
      bool vertexHasNoLep(const EventContext &, const xAOD::Vertex*) const;
  };
}

#endif
