/*
Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
*/

// this algorithm produces a J/Psi candidate muons container to be passed to the MuSAVtxFitter
// to be used in "tag and probe" style studies on the MuSA performance 
#include "MuSAVtxFitter/MuSAVtxJPsiValidationAlg.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "AthLinks/ElementLink.h"
#include "xAODTracking/VertexContainer.h"
#include "JpsiUpsilonTools/JpsiFinder.h"

namespace Rec {

MuSAVtxJPsiValidationAlg::MuSAVtxJPsiValidationAlg(const std::string& name, ISvcLocator* p)
  : AthAlgorithm(name,p)
{}

StatusCode MuSAVtxJPsiValidationAlg::initialize() {
  ATH_CHECK( m_muonContainer.initialize() );
  ATH_CHECK( m_eventInfo.initialize() );
  ATH_CHECK( m_JPsiMuonContainer.initialize() );

  ATH_CHECK( m_JPsiFinderTool.retrieve() );
  ATH_MSG_DEBUG("Retrieved J/Psi finder tool: " << m_JPsiFinderTool);

  ATH_MSG_DEBUG("Initialization successful");

  return StatusCode::SUCCESS;
}

StatusCode MuSAVtxJPsiValidationAlg::execute() {
    
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<xAOD::MuonContainer> muonContainer(m_muonContainer, ctx);
  ATH_CHECK(muonContainer.isValid());

  SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfo, ctx);
  ATH_CHECK(evtInfo.isValid());

  SG::WriteHandle<xAOD::MuonContainer> JPsiMuonContainer(m_JPsiMuonContainer, ctx);
  ATH_CHECK(JPsiMuonContainer.record(std::make_unique<xAOD::MuonContainer>(), std::make_unique<xAOD::MuonAuxContainer>()));

  // 1) Run J/Psi finder
  auto jpsiVtxs = std::make_unique<xAOD::VertexContainer>();
  auto jpsiAux  = std::make_unique<xAOD::VertexAuxContainer>();
  jpsiVtxs->setStore(jpsiAux.get());
  ATH_CHECK(m_JPsiFinderTool->performSearch(ctx, *jpsiVtxs));
    
  if (jpsiVtxs->empty()) {
        ATH_MSG_DEBUG("No J/Psi candidates found");
        return StatusCode::SUCCESS;
  }else{
        ATH_MSG_DEBUG("Found " << jpsiVtxs->size() << " J/Psi candidates");
  }

  for (const xAOD::Vertex* vtx : *jpsiVtxs) {
    const auto& links = vtx->trackParticleLinks();
    if (links.size() == 2) {
      const xAOD::TrackParticle* tp1 = links[0].isValid() ? *links[0] : nullptr;
      const xAOD::TrackParticle* tp2 = links[1].isValid() ? *links[1] : nullptr;
      const xAOD::Muon* mu1 = tp1 ? findMuonFromTrack(tp1, muonContainer.cptr()) : nullptr;
      const xAOD::Muon* mu2 = tp2 ? findMuonFromTrack(tp2, muonContainer.cptr()) : nullptr;

      if (!mu1 && !mu2) {
        ATH_MSG_WARNING("No muons found for J/Psi vertex with index " << vtx->index());
        continue;
      } else if (!mu1 || !mu2) {
        ATH_MSG_WARNING("Only one muon found for J/Psi vertex with index " << vtx->index());
      } else {
        ATH_MSG_DEBUG("Found J/Psi vertex with index " << vtx->index() << " and muons: " << mu1->index() << ", " << mu2->index());
      }

      if (mu1) JPsiMuonContainer->push_back(new xAOD::Muon(*mu1));
      if (mu2) JPsiMuonContainer->push_back(new xAOD::Muon(*mu2));
    }
  }

  
  return StatusCode::SUCCESS;
}

const xAOD::Muon* MuSAVtxJPsiValidationAlg::findMuonFromTrack(const xAOD::TrackParticle* tp, const xAOD::MuonContainer* muons) {
  for (const xAOD::Muon* mu : *muons) {
    if (mu->trackParticle(xAOD::Muon::CombinedTrackParticle) == tp) return mu;
    if (mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle) == tp) return mu;
  }
  return nullptr;
}

} // namespace Rec