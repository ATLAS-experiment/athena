/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNVertexFitter/GNNVertexFitterTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "ranges"
#include <boost/iterator/zip_iterator.hpp>

namespace Rec {

GNNVertexFitterTool::GNNVertexFitterTool(const std::string &type, const std::string &name,
                                                   const IInterface *parent)
    : AthAlgTool(type, name, parent), m_vertexFitterTool("Trk::TrkVKalVrtFitter/VertexFitterTool", this),
      m_deco_mass("mass"),
      m_deco_pt("pt"),
      m_deco_charge("charge"),
      m_deco_vPos("vPos"),
      m_deco_lxy("Lxy"),
      m_deco_sig3D("significance3D"),
      m_deco_deltaR("deltaR"),
      m_deco_ntrk("ntrk"),
      m_deco_lxyz("Lxyz"),
      m_deco_eFrac("efracsv"){
  declareInterface<IGNNVertexFitterInterface>(this);
  declareProperty("JetTrackLinks", m_trackLinksKey = "BTagging_AntiKt4EMPFlow."+m_gnnModel+"_TrackLinks");
  declareProperty("JetTrackOrigins", m_trackOriginsKey = "BTagging_AntiKt4EMPFlow."+m_gnnModel+"_TrackOrigin");
  declareProperty("JetVertexLinks", m_vertexLinksKey = "BTagging_AntiKt4EMPFlow."+m_gnnModel+"_VertexIndex");
  declareProperty("VertexFitterTool", m_vertexFitterTool, "Vertex fitting tool");
  m_massPi = 139.5702 * Gaudi::Units::MeV;
}

/* Destructor */
GNNVertexFitterTool::~GNNVertexFitterTool() { ATH_MSG_DEBUG("GNNVertexFitterTool destructor called"); }

StatusCode GNNVertexFitterTool::initialize() {

  ATH_MSG_DEBUG("GNNVertexFitter Tool in initialize()");

  // Initialize keys
  ATH_CHECK(m_trackLinksKey.initialize());
  ATH_CHECK(m_trackOriginsKey.initialize());
  ATH_CHECK(m_vertexLinksKey.initialize());

  m_jetWriteDecorKeyVertexLink = m_jetCollection + "."+m_gnnModel+"VerticesLink";
  ATH_CHECK(m_jetWriteDecorKeyVertexLink.initialize());

  // Retrieve tools
  ATH_CHECK(m_vertexFitterTool.retrieve());
  // Additional Info for Vertex Fit
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  return StatusCode::SUCCESS;
}

// Total Momentum of Jet
TLorentzVector GNNVertexFitterTool::TotalMom(const std::vector<const xAOD::TrackParticle *> &selTrk) const {
  TLorentzVector sum(0., 0., 0., 0.);
  for (int i = 0; i < (int)selTrk.size(); ++i) {
    if (selTrk[i] == NULL)
      continue;
    sum += selTrk[i]->p4();
  }
  return sum;
}

// Vertex to Vertex Distance
double GNNVertexFitterTool::vrtVrtDist(const xAOD::Vertex &primVrt, const Amg::Vector3D &secVrt,
                                            const std::vector<double> &secVrtErr, double &signif) const {
  double distx = primVrt.x() - secVrt.x();
  double disty = primVrt.y() - secVrt.y();
  double distz = primVrt.z() - secVrt.z();

  AmgSymMatrix(3) primCovMtx = primVrt.covariancePosition(); // Create
  primCovMtx(0, 0) += secVrtErr[0];
  primCovMtx(0, 1) += secVrtErr[1];
  primCovMtx(1, 0) += secVrtErr[1];
  primCovMtx(1, 1) += secVrtErr[2];
  primCovMtx(0, 2) += secVrtErr[3];
  primCovMtx(2, 0) += secVrtErr[3];
  primCovMtx(1, 2) += secVrtErr[4];
  primCovMtx(2, 1) += secVrtErr[4];
  primCovMtx(2, 2) += secVrtErr[5];

  AmgSymMatrix(3) wgtMtx = primCovMtx.inverse();

  signif = distx * wgtMtx(0, 0) * distx + disty * wgtMtx(1, 1) * disty + distz * wgtMtx(2, 2) * distz +
           2. * distx * wgtMtx(0, 1) * disty + 2. * distx * wgtMtx(0, 2) * distz + 2. * disty * wgtMtx(1, 2) * distz;
  signif = std::sqrt(std::abs(signif));
  if (signif != signif)
    signif = 0.;
  return std::sqrt(distx * distx + disty * disty + distz * distz);
}

// Perform Vertex fit using the jet decorations of the GNN
StatusCode GNNVertexFitterTool::fitAllVertices(const xAOD::JetContainer *inJetContainer,
                                                      xAOD::VertexContainer *outVertexContainer,
                                                      const xAOD::Vertex &primVrt, const EventContext &ctx) const {

  using TLC = std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1>>>;
  using TL = ElementLink<DataVector<xAOD::TrackParticle_v1>>;

  // Read Decor Handle for Track links and Vertex links
  SG::ReadDecorHandle<xAOD::BTaggingContainer, TLC> trackLinksHandle(m_trackLinksKey, ctx);
  SG::ReadDecorHandle<xAOD::BTaggingContainer, std::vector<char>> trackOriginsHandle(m_trackOriginsKey, ctx);
  SG::ReadDecorHandle<xAOD::BTaggingContainer, std::vector<char>> vertexLinksHandle(m_vertexLinksKey, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::VertexContainer>>>
      jetWriteDecorHandleVertexLink(m_jetWriteDecorKeyVertexLink, ctx);

  static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::BTaggingContainer> > btagLinkAcc 
    = SG::AuxElement::ConstAccessor<ElementLink<xAOD::BTaggingContainer> >("btaggingLink");

  // Loop over the jets
  for (const auto &jet : *inJetContainer) {
    const xAOD::BTagging* btag = *btagLinkAcc(*jet);

    // Retrieve the Vertex and Track Collections
    auto vertexCollection = vertexLinksHandle(*btag);
    auto trackCollection = trackLinksHandle(*btag);
    auto trackOriginCollection = trackOriginsHandle(*btag);

    using indexList = std::vector<int>;
    using vertexHFMap = std::map<char, bool>;
    using trackCountMap = std::map<char, std::set<TL>>;

    indexList iList(vertexCollection.size());

    vertexHFMap HeavyFlavourTracksMap;   // Map with does a vertex contain at least 1 heavy flavour track
    trackCountMap AllTracksMap;          // All the vertices and the corresponding track links
    trackCountMap HeavyFlavourVertexMap; // All Heavy Flavour Tracks associated with a vertex
    trackCountMap FittingMap;            // Map filled with vertices to be fitted

    FittingMap.clear();
    HeavyFlavourVertexMap.clear();
    HeavyFlavourTracksMap.clear();
    AllTracksMap.clear();

    for (int index=0; index<int(vertexCollection.size()); index++){
    
      auto vertex = vertexCollection[index];
      auto trackOrigin = trackOriginCollection[index];
      auto trackLink   = trackCollection[index];

      AllTracksMap[vertex].insert(trackLink);

      // Checking if vertex has a heavy flavour track
      if (InDet::ExclusiveOrigin::FromB == trackOrigin || InDet::ExclusiveOrigin::FromBC == trackOrigin ||
          InDet::ExclusiveOrigin::FromC == trackOrigin) {
        HeavyFlavourTracksMap[vertex] = (true);
        HeavyFlavourVertexMap[vertex].insert(trackLink);
      }
      };

    auto HFRatioFunc = [&HeavyFlavourVertexMap, &HFRatio = m_HFRatioThres, &FittingMap](const auto &d) {
      const auto &[vertex, tcm] = d;
      if (HeavyFlavourVertexMap.find(vertex) != HeavyFlavourVertexMap.end() &&
          (static_cast<float>(HeavyFlavourVertexMap[vertex].size()) / tcm.size()) >= HFRatio) {
        FittingMap.insert(std::pair<char, std::set<TL>>(vertex, tcm));
      };
    };

    if (m_HFRatioThres > 0) {
      ATH_MSG_DEBUG("Vertex with HF Ratio ");
      std::for_each(AllTracksMap.cbegin(), AllTracksMap.cend(), HFRatioFunc);
    } else {
      ATH_MSG_DEBUG("No Requirement on Track Origins");
      std::for_each(AllTracksMap.cbegin(), AllTracksMap.cend(), HFRatioFunc);
    }

    // Working xAOD   
    std::unique_ptr<workVectorArrxAOD> xAODwrk (new workVectorArrxAOD());
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};

    // Beam Conditions
    xAODwrk->beamX = beamSpotHandle->beamPos().x();
    xAODwrk->beamY = beamSpotHandle->beamPos().y();
    xAODwrk->beamZ = beamSpotHandle->beamPos().z();
    xAODwrk->tanBeamTiltX = tan(beamSpotHandle->beamTilt(0));
    xAODwrk->tanBeamTiltY = tan(beamSpotHandle->beamTilt(1));

    std::unique_ptr<std::vector<WrkVrt>> wrkVrtSet = std::make_unique<std::vector<WrkVrt>>();
    WrkVrt newvrt;
    newvrt.Good = true;
    std::unique_ptr<Trk::IVKalState> state = m_vertexFitterTool->makeState();
    std::vector<const xAOD::NeutralParticle *> neutralPartDummy(0);
    Amg::Vector3D IniVrt(0., 0., 0.);

    for (const auto &pair : FittingMap) {
      if (pair.second.size() >= 2) {
        // Need at least 2 tracks to perform a fit
        int NTRKS = pair.second.size();
        std::vector<double> InpMass(NTRKS, m_massPi);
        m_vertexFitterTool->setMassInputParticles(InpMass, *state);

        xAODwrk->listSelTracks.clear();

        for (auto TrackLink : pair.second) {
          xAODwrk->listSelTracks.push_back(*TrackLink);
        }

        Amg::Vector3D FitVertex, vDist;
        TLorentzVector jetDir(jet->p4()); // Jet Direction

        // Get Estimate
        StatusCode sc = m_vertexFitterTool->VKalVrtFitFast(xAODwrk->listSelTracks, FitVertex, *state);

        if (sc.isFailure() || FitVertex.perp() > m_maxLxy) { /* No initial estimation */
          IniVrt = primVrt.position();
          if (m_multiWithPrimary)
            IniVrt.setZero();
        } else {
          vDist = FitVertex - primVrt.position();
          double JetVrtDir = jetDir.Px() * vDist.x() + jetDir.Py() * vDist.y() + jetDir.Pz() * vDist.z();
          if (m_multiWithPrimary)
            JetVrtDir = fabs(JetVrtDir); /* Always positive when primary vertex is seeked for*/
          if (JetVrtDir > 0.)
            IniVrt = FitVertex; /* Good initial estimation */
          else
            IniVrt = primVrt.position();
        }

        m_vertexFitterTool->setApproximateVertex(IniVrt.x(), IniVrt.y(), IniVrt.z(), *state);

        // Perform the Vertex Fit
        sc = (m_vertexFitterTool->VKalVrtFit(xAODwrk->listSelTracks, neutralPartDummy, newvrt.vertex, newvrt.vertexMom,
                                             newvrt.vertexCharge, newvrt.vertexCov, newvrt.chi2PerTrk, newvrt.trkAtVrt,
                                             newvrt.chi2, *state, false));
        if (sc.isFailure())
          continue;

        // Chi2 Cut
        auto NDOF = 2 * (newvrt.trkAtVrt.size()) - 3.0; // From VrtSecInclusive

        if (newvrt.chi2 / NDOF >= m_maxChi2)
          continue;
        ATH_MSG_DEBUG("Found IniVertex=" << newvrt.vertex[0] << ", " << newvrt.vertex[1] << ", " << newvrt.vertex[2]
                                         << " trks " << newvrt.trkAtVrt.size());

        Amg::Vector3D vDir = newvrt.vertex - primVrt.position(); // Vertex Dirction in relation to Primary

        Amg::Vector3D jetVrtDir(jet->p4().Px(), jet->p4().Py(), jet->p4().Pz());
        
        double vPos =
            (vDir.x() * newvrt.vertexMom.Px() + vDir.y() * newvrt.vertexMom.Py() + vDir.z() * newvrt.vertexMom.Pz()) /
            newvrt.vertexMom.Rho();

        double Lxy = sqrt(vDir[0] * vDir[0] + vDir[1] * vDir[1]);
        double Lxyz = sqrt(vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2]);
        ATH_MSG_DEBUG("Lxyz  " << Lxyz);
        
        double drJPVSV = Amg::deltaR(jetVrtDir, vDir); // DeltaR
        
        int ntrk = newvrt.trkAtVrt.size(); // # Tracks in Vertex

        TLorentzVector MomentumVtx = TotalMom(xAODwrk->listSelTracks);

        double eRatio = MomentumVtx.E() / jet->p4().E();
        double signif3D;
        double distToPV = vrtVrtDist(primVrt, newvrt.vertex, newvrt.vertexCov, signif3D);
        
        // apply quality cuts
        if (ntrk < m_minNTrack)
          continue;
        if (distToPV < m_minSig3D && newvrt.vertex.perp() > 24.0)
          continue;
        if (Lxy <= m_minLxy ) 
          continue;
        if ( newvrt.vertex.perp() < m_minPerp)
          continue;
        
        // Register Container
        auto* GNNvertex = outVertexContainer->emplace_back(new xAOD::Vertex);

        for (const auto *trk : xAODwrk->listSelTracks) {
          ElementLink<xAOD::TrackParticleContainer> link_trk(
              *(dynamic_cast<const xAOD::TrackParticleContainer *>(trk->container())),
              trk->index());
          GNNvertex->addTrackAtVertex(link_trk, 1.);
        }

        //Add Vertex Info into Container 
        GNNvertex->setVertexType(xAOD::VxType::SecVtx);
        GNNvertex->setPosition(newvrt.vertex);
        GNNvertex->setFitQuality(newvrt.chi2, NDOF);
        m_deco_mass(*GNNvertex)            = newvrt.vertexMom.M();
        m_deco_pt(*GNNvertex)              = newvrt.vertexMom.Perp();
        m_deco_charge(*GNNvertex)          = newvrt.vertexCharge;
        m_deco_vPos(*GNNvertex)            = vPos;
        m_deco_lxy(*GNNvertex)             = Lxy;
        m_deco_lxyz(*GNNvertex)            = Lxyz;
        m_deco_sig3D(*GNNvertex)           = signif3D; 
        m_deco_ntrk(*GNNvertex)            = ntrk;
        m_deco_deltaR(*GNNvertex)          = drJPVSV;
        m_deco_eFrac(*GNNvertex)           = eRatio;
        
        ElementLink<xAOD::VertexContainer> linkVertex;
        linkVertex.setElement(GNNvertex);
        linkVertex.setStorableObject(*outVertexContainer);
        jetWriteDecorHandleVertexLink(*jet).push_back(linkVertex);

      } // end of 2 Track requirement
    }
  } // end loop over jets
  return StatusCode::SUCCESS;
} // end performVertexFit

StatusCode GNNVertexFitterTool::finalize() { return StatusCode::SUCCESS; }

} // namespace Rec

