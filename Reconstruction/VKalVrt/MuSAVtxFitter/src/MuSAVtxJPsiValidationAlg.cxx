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
#include "VrtSecInclusive/Constants.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

namespace {
  // Accessors for vertex properties
  const SG::AuxElement::Accessor<float> vtx_pxAcc("vtx_px");
  const SG::AuxElement::Accessor<float> vtx_pyAcc("vtx_py");
  const SG::AuxElement::Accessor<float> vtx_pzAcc("vtx_pz");
  const SG::AuxElement::Accessor<float> vtx_massAcc("vtx_mass");
  const SG::AuxElement::Accessor<float> vtx_chargeAcc("vtx_charge");
  const SG::AuxElement::Accessor<float> minOpAngAcc("minOpAng");
  const SG::AuxElement::Accessor<float> chi2_coreAcc("chi2_core");
  const SG::AuxElement::Accessor<float> ndof_coreAcc("ndof_core");
  const SG::AuxElement::Accessor<float> chi2_assocAcc("chi2_assoc");
  const SG::AuxElement::Accessor<float> ndof_assocAcc("ndof_assoc");
  const SG::AuxElement::Accessor<float> massAcc("mass");
  const SG::AuxElement::Accessor<float> mass_eAcc("mass_e");
  const SG::AuxElement::Accessor<float> mass_selectedTracksAcc("mass_selectedTracks");
  const SG::AuxElement::Accessor<int>   num_trksAcc("num_trks");
  const SG::AuxElement::Accessor<int>   num_selectedTracksAcc("num_selectedTracks");
  const SG::AuxElement::Accessor<int>   num_associatedTracksAcc("num_associatedTracks");
  const SG::AuxElement::Accessor<float> dCloseVrtAcc("dCloseVrt");
}

namespace Rec {

MuSAVtxJPsiValidationAlg::MuSAVtxJPsiValidationAlg(const std::string& name, ISvcLocator* p)
  : AthAlgorithm(name,p)
{}

StatusCode MuSAVtxJPsiValidationAlg::initialize() {
    ATH_CHECK(m_muonContainer.initialize());
    ATH_CHECK(m_eventInfo.initialize());
    ATH_CHECK(m_JPsiMuonContainer.initialize());
    ATH_CHECK(m_JPsiVertexContainer.initialize()); 
    ATH_CHECK(m_JPsiTrackParticleContainer.initialize());

    ATH_CHECK(m_JPsiFinderTool.retrieve());
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

  SG::WriteHandle<xAOD::VertexContainer> JPsiVertexContainer(m_JPsiVertexContainer, ctx);
  ATH_CHECK(JPsiVertexContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));

  SG::WriteHandle<xAOD::TrackParticleContainer> JPsiTrackParticleContainer(m_JPsiTrackParticleContainer, ctx);
  ATH_CHECK(JPsiTrackParticleContainer.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));

  // 1) Run J/Psi finder
  auto jpsiVtxs = std::make_unique<xAOD::VertexContainer>();
  auto jpsiAux  = std::make_unique<xAOD::VertexAuxContainer>();
  jpsiVtxs->setStore(jpsiAux.get());
  ATH_CHECK(m_JPsiFinderTool->performSearch(ctx, *jpsiVtxs));
    
  if (jpsiVtxs->empty()) {
        ATH_MSG_DEBUG("No J/Psi candidates found");
        return StatusCode::SUCCESS;
  } else {
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

    // vertices right out of JPsiFinder have vxTrackAtVertex decorators that can't be saved to container
    // thus we make a fresh copy to avoid this issue
    xAOD::Vertex* newVtx = new xAOD::Vertex();
    JPsiVertexContainer->push_back(newVtx); // add the vertex to the container first

    // Set properties of the new vertex
    newVtx->setPosition(vtx->position());
    newVtx->setCovariance(vtx->covariance());
    newVtx->setFitQuality(vtx->chiSquared(), vtx->numberDoF());
    newVtx->setVertexType(vtx->vertexType());

    // copy track particle links
    for (const auto& link : vtx->trackParticleLinks()) {
      if (!link.isValid()) continue;
      const xAOD::TrackParticle* oldTrack = *link;
      xAOD::TrackParticle* newTrack = new xAOD::TrackParticle();
      newTrack->makePrivateStore(*oldTrack);
      JPsiTrackParticleContainer->push_back(newTrack);
      ElementLink<xAOD::TrackParticleContainer> newLink(*JPsiTrackParticleContainer, newTrack->index());
      newVtx->addTrackAtVertex(newLink, 1.0); // weight is set to 1.0 for simplicity
    }

    // add identical decos to musa for comparison
    constexpr double muonMass = 105.658; // Muon mass in MeV
    TLorentzVector sumP4_muon, sumP4_electron, sumP4_selected;
    std::vector<const xAOD::TrackParticle*> vtxTracks;

    int vtxCharge = 0;

    for (const auto& link : newVtx->trackParticleLinks()) {
      const xAOD::TrackParticle* track = link.isValid() ? *link : nullptr;
      if (!track) continue;

      double pt = track->pt();
      double eta = track->eta();
      double phi = track->phi();

      TLorentzVector p4_muon, p4_electron;
      p4_muon.SetPtEtaPhiM(pt, eta, phi, muonMass);
      p4_electron.SetPtEtaPhiM(pt, eta, phi, VKalVrtAthena::PhysConsts::mass_electron);

      sumP4_muon += p4_muon;
      sumP4_electron += p4_electron;
      sumP4_selected += p4_muon; // assuming selected tracks are muons...

      vtxCharge += track->charge();
      vtxTracks.push_back(track);
    }

    //calculate opening angle between two tracks
    float vtxDeltaR = 0;
    if (vtxTracks.size() == 2) {
        float dEta = vtxTracks[0]->eta() - vtxTracks[1]->eta();
        float dPhi = std::abs(vtxTracks[0]->phi() - vtxTracks[1]->phi());
        if (dPhi > M_PI) dPhi = 2 * M_PI - dPhi;
        vtxDeltaR = std::sqrt(dEta * dEta + dPhi * dPhi);
    }

    vtx_pxAcc(*newVtx) = sumP4_muon.Px();
    vtx_pyAcc(*newVtx) = sumP4_muon.Py();
    vtx_pzAcc(*newVtx) = sumP4_muon.Pz();
    vtx_massAcc(*newVtx) = sumP4_muon.M();
    vtx_chargeAcc(*newVtx) = vtxCharge;
    minOpAngAcc(*newVtx) = vtxDeltaR; 
    chi2_coreAcc(*newVtx) = vtx->chiSquared();
    ndof_coreAcc(*newVtx) = vtx->numberDoF();
    chi2_assocAcc(*newVtx) = vtx->chiSquared();
    ndof_assocAcc(*newVtx) = vtx->numberDoF();
    massAcc(*newVtx) = sumP4_muon.M();
    mass_eAcc(*newVtx) = sumP4_electron.M();
    mass_selectedTracksAcc(*newVtx) = sumP4_selected.M();
    num_trksAcc(*newVtx) = links.size();
    num_selectedTracksAcc(*newVtx) = links.size(); // Assuming all tracks are selected
    num_associatedTracksAcc(*newVtx) = links.size(); // Assuming all tracks are associated
    dCloseVrtAcc(*newVtx) = 0; // not implemented but assigned for comparison to MuSA
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