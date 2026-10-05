/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h"
#include "InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchTool.h"
#include "xAODTruth/TruthParticle.h"

#include "TH1.h"
#include "TEfficiency.h"
#include "TLorentzVector.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
  constexpr float GeV = 1000.;
}

namespace CP {

  SecVertexTruthMatchAlg::SecVertexTruthMatchAlg( const std::string& name, ISvcLocator* svcLoc )
    : EL::AnaAlgorithm( name, svcLoc ) {}

  StatusCode SecVertexTruthMatchAlg::initialize() {

    // Initializing Keys
    ATH_CHECK(m_secVtxContainerKey.initialize());
    ATH_CHECK(m_truthVtxContainerKey.initialize());
    ATH_CHECK(m_trackParticleContainerKey.initialize());

    // Retrieving the tool
    ATH_CHECK(m_matchTool.retrieve());

    if(m_writeHistograms) {
      std::vector<std::string> recoTypes{"All", "Matched", "Merged", "Fake", "Split", "Other"};
      std::vector<std::string> truthTypes{"Inclusive", "Reconstructable", "Accepted", "Seeded", "Reconstructed", "ReconstructedSplit"};
      // define SM origin categories if SM origin tracking is enabled
      std::vector<std::string> smOriginTypes;
      if(m_doSMOrigin) {
        smOriginTypes = {"FakeOrigin", "Pileup", "KshortDecay", "StrangeMesonDecay", "LambdaDecay", 
                        "StrangeBaryonDecay", "TauDecay", "GammaConversion", "OtherDecay", 
                        "HadronicInteraction", "OtherSecondary", "BHadronDecay", "DHadronDecay", 
                        "Fragmentation", "OtherOrigin", "Signal"};
      }

      //determine histogram ranges depending if we are in normal or MuSA mode
      //total bin counts stay the same for simplicity -- MuSA has less precision in general 
      float maxX = m_doMuSA ? 8000 : 500;
      float maxY = m_doMuSA ? 10000 : 500;
      float maxZ = m_doMuSA ? 10000 : 1500;
      float maxLxy = m_doMuSA ? 8000 : 500;
      float maxR = m_doMuSA ? 8000 : 600;
      float mind0 = m_doMuSA ? 2000 : 100;
      float maxd0 = m_doMuSA ? 2000 : 100;
      float maxTrackd0 = m_doMuSA ? 3000 : 300;
      float maxTrackz0 = m_doMuSA ? 5000 : 500;
      float maxErrd0 = m_doMuSA ? 300 : 30;
      float maxErrz0 = m_doMuSA ? 500 : 50;

      float maxResR = m_doMuSA ? 2000 : 20;
      float maxResZ = m_doMuSA ? 2000 : 20;

      ANA_CHECK (book(TH1F("RecoVertex/matchType", "Vertex Match Type", 65, -0.5, 64.5)));
      if (m_doSMOrigin) {
        ANA_CHECK (book(TH1F("RecoVertex/smOriginType", "Vertex SM Origin Type", 65537, -0.5, 65536.5)));
      }

      // book the reco vertex histograms of one category and cache their pointers
      auto bookRecoVertexHistos = [&](const std::string& category, bool withMatchScore) -> StatusCode {
        RecoVertexHists& h = m_recoHists[category];
        auto bookHist = [&](TH1*& target, const std::string& suffix, const char* title,
                            int nbins, double low, double high) -> StatusCode {
          const std::string name = "RecoVertex/" + category + suffix;
          ANA_CHECK (book(TH1F(name.c_str(), title, nbins, low, high)));
          target = hist(name);
          return StatusCode::SUCCESS;
        };

        ANA_CHECK (bookHist(h.x, "_x", "Reco vertex x [mm]", 1000, -maxX, maxX));
        ANA_CHECK (bookHist(h.y, "_y", "Reco vertex y [mm]", 1000, -maxY, maxY));
        ANA_CHECK (bookHist(h.z, "_z", "Reco vertex z [mm]", 1000, -maxZ, maxZ));
        ANA_CHECK (bookHist(h.Lxy, "_Lxy", "Reco vertex L_{xy} [mm]", 500, 0, maxLxy));
        ANA_CHECK (bookHist(h.pT, "_pT", "Reco vertex p_{T} [GeV]", 100, 0, 100));
        ANA_CHECK (bookHist(h.eta, "_eta", "Reco vertex #eta", 100, -5, 5));
        ANA_CHECK (bookHist(h.phi, "_phi", "Reco vertex #phi", 100, -std::numbers::pi, std::numbers::pi));
        ANA_CHECK (bookHist(h.mass, "_mass", "Reco vertex mass [GeV]", 500, 0, 100));
        ANA_CHECK (bookHist(h.mu, "_mu", "Reco vertex Red. Mass [GeV]", 500, 0, 100));
        ANA_CHECK (bookHist(h.chi2, "_chi2", "Reco vertex recoChi2", 100, 0, 10));
        ANA_CHECK (bookHist(h.dir, "_dir", "Reco vertex recoDirection", 100, -1, 1));
        ANA_CHECK (bookHist(h.charge, "_charge", "Reco vertex recoCharge", 20, -10, 10));
        ANA_CHECK (bookHist(h.H, "_H", "Reco vertex H [GeV]", 100, 0, 100));
        ANA_CHECK (bookHist(h.HT, "_HT", "Reco vertex Mass [GeV]", 100, 0, 100));
        ANA_CHECK (bookHist(h.minOpAng, "_minOpAng", "Reco vertex minOpAng", 100, -1, 1));
        ANA_CHECK (bookHist(h.maxOpAng, "_maxOpAng", "Reco vertex maxOpAng", 100, -1, 1));
        ANA_CHECK (bookHist(h.maxdR, "_maxdR", "Reco vertex maxDR", 100, 0, 10));
        ANA_CHECK (bookHist(h.mind0, "_mind0", "Reco vertex min d0 [mm]", 100, 0, mind0));
        ANA_CHECK (bookHist(h.maxd0, "_maxd0", "Reco vertex max d0 [mm]", 100, 0, maxd0));
        ANA_CHECK (bookHist(h.ntrk, "_ntrk", "Reco vertex n tracks", 30, 0, 30));

        // tracks
        ANA_CHECK (bookHist(h.Trk_qOverP, "_Trk_qOverP", "Reco track qOverP ", 100, 0, .01));
        ANA_CHECK (bookHist(h.Trk_theta, "_Trk_theta", "Reco track theta ", 64, 0, 3.2));
        ANA_CHECK (bookHist(h.Trk_E, "_Trk_E", "Reco track E ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_M, "_Trk_M", "Reco track M ", 100, 0, 10));
        ANA_CHECK (bookHist(h.Trk_Pt, "_Trk_Pt", "Reco track Pt ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_Px, "_Trk_Px", "Reco track Px ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_Py, "_Trk_Py", "Reco track Py ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_Pz, "_Trk_Pz", "Reco track Pz ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_Eta, "_Trk_Eta", "Reco track Eta ", 100, -5, 5));
        ANA_CHECK (bookHist(h.Trk_Phi, "_Trk_Phi", "Reco track Phi ", 63, -3.2, 3.2));
        ANA_CHECK (bookHist(h.Trk_D0, "_Trk_D0", "Reco track D0 ", 300, -maxTrackd0, maxTrackd0));
        ANA_CHECK (bookHist(h.Trk_Z0, "_Trk_Z0", "Reco track Z0 ", 500, -maxTrackz0, maxTrackz0));
        ANA_CHECK (bookHist(h.Trk_errD0, "_Trk_errD0", "Reco track errD0 ", 300, 0, maxErrd0));
        ANA_CHECK (bookHist(h.Trk_errZ0, "_Trk_errZ0", "Reco track errZ0 ", 500, 0, maxErrz0));
        ANA_CHECK (bookHist(h.Trk_Chi2, "_Trk_Chi2", "Reco track Chi2 ", 100, 0, 10));
        ANA_CHECK (bookHist(h.Trk_nDoF, "_Trk_nDoF", "Reco track nDoF ", 100, 0, 100));
        ANA_CHECK (bookHist(h.Trk_charge, "_Trk_charge", "Reco track charge ", 3, -1.5, 1.5));

        if (withMatchScore) {
          ANA_CHECK (bookHist(h.positionRes_R, "_positionRes_R", "Position resolution for vertices matched to truth decays", 400, -maxResR, maxResR));
          ANA_CHECK (bookHist(h.positionRes_Z, "_positionRes_Z", "Position resolution for vertices matched to truth decays", 400, -maxResZ, maxResZ));
          ANA_CHECK (bookHist(h.matchScore_weight, "_matchScore_weight", "Vertex Match Score (weight)", 101, 0, 1.01));
          ANA_CHECK (bookHist(h.matchScore_pt, "_matchScore_pt", "Vertex Match Score (pT)", 101, 0, 1.01));
        }
        return StatusCode::SUCCESS;
      };

      for(const auto& recoType : recoTypes) {
        // truth matching -- don't book for non-matched vertices
        ANA_CHECK (bookRecoVertexHistos(recoType, recoType != "All" and recoType != "Fake"));
      }

      // do reco vertices by SM origin if enabled -- NOTE these types are not exclusive (d decays will also be b decays in a cascade etc)
      for(const auto& smOriginType : smOriginTypes) {
        ANA_CHECK (bookRecoVertexHistos(smOriginType, false));
      }

      for(const auto& truthType : truthTypes) {
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_x").c_str(), "Truth vertex x [mm]", 1000, -maxX, maxX)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_y").c_str(), "Truth vertex y [mm]", 500, -maxY, maxY)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_z").c_str(), "Truth vertex z [mm]", 500, -maxZ, maxZ)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_R").c_str(), "Truth vertex r [mm]", 6000, 0, maxR)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Eta").c_str(), "Truth vertex Eta", 100, -5, 5)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Phi").c_str(), "Truth vertex Phi", 64, -3.2, 3.2)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Ntrk_out").c_str(), "Truth vertex n outgoing tracks", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_E").c_str(), "Reco track E", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_M").c_str(), "Reco track M", 500, 0, 500)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_Pt").c_str(), "Reco track Pt", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_Eta").c_str(), "Reco track Eta", 100, -5, 5)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_Phi").c_str(), "Reco track Phi", 63, -3.2, 3.2)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_Parent_charge").c_str(), "Reco track charge", 3, -1, 1)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdX").c_str(), "truthParentProd vertex x [mm]", 500, -500, 500)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdY").c_str(), "truthParentProd vertex y [mm]", 500, -500, 500)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdZ").c_str(), "truthParentProd vertex z [mm]", 500, -500, 500)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdR").c_str(), "truthParentProd vertex r [mm]", 6000, 0, 600)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdEta").c_str(), "truthParentProd vertex Eta", 100, -5, 5)));
        ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdPhi").c_str(), "truthParentProd vertex Phi", 64, -3.2, 3.2)));
      }

      // now add the efficiencies
      // Define two different bin arrays - one for standard mode, one for MuSA
      const std::vector<double> bins = m_doMuSA
        ? std::vector<double>{0.0, 1, 5, 10, 20, 50, 100, 200, 300, 500, 750, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000}
        : std::vector<double>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 35, 40, 50, 60, 70, 80, 90, 100, 125, 150, 200, 300, 500};
      const int nbins = std::size(bins) - 1;

      ANA_CHECK (book(TEfficiency("Acceptance", "Acceptance", nbins, bins.data())));
      ANA_CHECK (book(TEfficiency("eff_seed", "Seed efficiency", nbins, bins.data())));
      ANA_CHECK (book(TEfficiency("eff_core", "Core efficiency", nbins, bins.data())));
      ANA_CHECK (book(TEfficiency("eff_total", "Total efficiency", nbins, bins.data())));
      
    }


    return StatusCode::SUCCESS;
  }

  StatusCode SecVertexTruthMatchAlg::execute(const EventContext& ctx) {

    //Retrieve the vertices:
    SG::ReadHandle<xAOD::VertexContainer> recoVertexContainer(m_secVtxContainerKey, ctx);
    if (!recoVertexContainer.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve secondary vertex container " << m_secVtxContainerKey.key());
      return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::TruthVertexContainer> truthVertexContainer(m_truthVtxContainerKey, ctx);
    if (!truthVertexContainer.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve truth vertex container " << m_truthVtxContainerKey.key());
      return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleContainer(m_trackParticleContainerKey, ctx);
    if (!trackParticleContainer.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve track particle container " << m_trackParticleContainerKey.key());
      return StatusCode::FAILURE;
    }

    std::vector<const xAOD::Vertex*> recoVerticesToMatch;
    std::vector<const xAOD::TruthVertex*> truthVerticesToMatch;

    for(const auto recoVertex : *recoVertexContainer) {
      if(recoVertex->vertexType() != xAOD::VxType::SecVtx ){
        ATH_MSG_DEBUG("Vertex not labeled as secondary");
        continue;
      }
      recoVerticesToMatch.push_back(recoVertex);
    }

    for(const auto truthVertex : *truthVertexContainer) {
      if(truthVertex->nIncomingParticles() != 1) {
        continue;
      }
      const xAOD::TruthParticle* truthPart = truthVertex->incomingParticle(0);
      if(not truthPart) {
        continue;
      }
      if(std::ranges::find(m_targetPDGIDs.value(), std::abs(truthPart->pdgId())) == m_targetPDGIDs.value().end()) {
        continue;
      }
      if(truthVertex->nOutgoingParticles() < 2) {
        continue;
      }
      truthVerticesToMatch.push_back(truthVertex);
    }

    //pass to the tool for decoration:
    ATH_CHECK( m_matchTool->matchVertices( recoVerticesToMatch, truthVerticesToMatch, trackParticleContainer.cptr() ) );

    if(m_writeHistograms) {
      static const xAOD::Vertex::ConstAccessor<int> matchTypeAcc("vertexMatchType");
      static const xAOD::Vertex::ConstAccessor<int> originTypeAcc("vertexMatchOriginType");

      static const std::map<InDetSecVtxTruthMatchUtils::VertexMatchOriginType, std::string> originTypeMap = {
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::FakeOrigin, "FakeOrigin"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::Pileup, "Pileup"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::KshortDecay, "KshortDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::StrangeMesonDecay, "StrangeMesonDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::LambdaDecay, "LambdaDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::StrangeBaryonDecay, "StrangeBaryonDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::TauDecay, "TauDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::GammaConversion, "GammaConversion"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::OtherDecay, "OtherDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::HadronicInteraction, "HadronicInteraction"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::OtherSecondary, "OtherSecondary"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::BHadronDecay, "BHadronDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::DHadronDecay, "DHadronDecay"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::Fragmentation, "Fragmentation"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::OtherOrigin, "OtherOrigin"},
        {InDetSecVtxTruthMatchUtils::VertexMatchOriginType::Signal, "Signal"},
      };

      std::vector<const RecoVertexHists*> categories;
      for(const auto& secVtx : recoVerticesToMatch) {
        categories.clear();
        int matchTypeBitset = matchTypeAcc(*secVtx);
        hist("RecoVertex/matchType")->Fill(matchTypeBitset);

        if(InDetSecVtxTruthMatchUtils::isMatched(matchTypeBitset)) {
          categories.push_back(&m_recoHists.at("Matched"));
        }
        if(InDetSecVtxTruthMatchUtils::isMerged(matchTypeBitset)) {
          categories.push_back(&m_recoHists.at("Merged"));
        }
        if(InDetSecVtxTruthMatchUtils::isFake(matchTypeBitset)) {
          categories.push_back(&m_recoHists.at("Fake"));
        }
        if(InDetSecVtxTruthMatchUtils::isSplit(matchTypeBitset)) {
          categories.push_back(&m_recoHists.at("Split"));
        }
        if(InDetSecVtxTruthMatchUtils::isOther(matchTypeBitset)) {
          categories.push_back(&m_recoHists.at("Other"));
        }
        categories.push_back(&m_recoHists.at("All"));

        if (m_doSMOrigin) {
          int smOriginTypeBitset = originTypeAcc(*secVtx);
          hist("RecoVertex/smOriginType")->Fill(smOriginTypeBitset);

          for(const auto& entry : originTypeMap) {
            if(InDetSecVtxTruthMatchUtils::isOriginType(smOriginTypeBitset, entry.first)) {
              categories.push_back(&m_recoHists.at(entry.second));
            }
          }
        }

        fillRecoHistograms(secVtx, categories);
      }

      static const xAOD::TruthVertex::ConstAccessor<int> truthTypeAcc("truthVertexMatchType");
      for(const auto& truthVtx : truthVerticesToMatch) {
        int truthTypeBitset = truthTypeAcc(*truthVtx);
        if(InDetSecVtxTruthMatchUtils::isReconstructable(truthTypeBitset)) {
          fillTruthHistograms(truthVtx, "Reconstructable");

          // fill efficiencies
          efficiency("Acceptance")->Fill(InDetSecVtxTruthMatchUtils::isAccepted(truthTypeBitset), truthVtx->perp());
          efficiency("eff_total")->Fill(InDetSecVtxTruthMatchUtils::isReconstructed(truthTypeBitset), truthVtx->perp());
        }
        if(InDetSecVtxTruthMatchUtils::isAccepted(truthTypeBitset)) {
          fillTruthHistograms(truthVtx, "Accepted");
          efficiency("eff_seed")->Fill(InDetSecVtxTruthMatchUtils::isSeeded(truthTypeBitset), truthVtx->perp());
        }
        if(InDetSecVtxTruthMatchUtils::isSeeded(truthTypeBitset)) {
          fillTruthHistograms(truthVtx, "Seeded");
          efficiency("eff_core")->Fill(InDetSecVtxTruthMatchUtils::isReconstructed(truthTypeBitset), truthVtx->perp());
        }
        if(InDetSecVtxTruthMatchUtils::isReconstructed(truthTypeBitset)) {
          fillTruthHistograms(truthVtx, "Reconstructed");
        }
        if(InDetSecVtxTruthMatchUtils::isReconstructedSplit(truthTypeBitset)) {
          fillTruthHistograms(truthVtx, "ReconstructedSplit");
        }
        fillTruthHistograms(truthVtx, "Inclusive");

      }
      
    }

    return StatusCode::SUCCESS;

  }
  void SecVertexTruthMatchAlg::fillRecoHistograms(const xAOD::Vertex* secVtx, const std::vector<const RecoVertexHists*>& categories) {

    // set of accessors for tracks and truth matching info
    xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
    const xAOD::Vertex::ConstAccessor<std::vector<InDetSecVtxTruthMatchUtils::VertexTruthMatchInfo> > matchInfoAcc("truthVertexMatchingInfos");

    TVector3 reco_pos(secVtx->x(), secVtx->y(), secVtx->z());
    float Lxy = reco_pos.Perp();

    size_t ntracks;
    const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( *secVtx );
    ntracks = trkParts.size();

    TLorentzVector sumP4(0,0,0,0);
    double H = 0.0;
    double HT = 0.0;
    int charge = 0;
    // NOTE: minOpAng/maxOpAng hold the cosines of the minimum/maximum opening
    // angle between two tracks, i.e. minOpAng is the largest cosine
    double minOpAng = -1.0* 1.e10;
    double maxOpAng =  1.0* 1.e10;
    double minD0 = 1.0* 1.e10;
    double maxD0 = 0.0;
    double maxDR = 0.0;
    size_t nValidTracks = 0;

    ATH_MSG_DEBUG("Loop over tracks");
    for(size_t t = 0; t < ntracks; t++){
      if(!trkParts[t].isValid()){
        ATH_MSG_DEBUG("Track " << t << " is bad!");
        continue;
      }
      const xAOD::TrackParticle & trk = **trkParts[t];
      ++nValidTracks;

      double trk_d0 = std::abs(trk.definingParameters()[0]);
      double trk_z0 = std::abs(trk.definingParameters()[1]);

      if(trk_d0 < minD0){ minD0 = trk_d0; }
      if(trk_d0 > maxD0){ maxD0 = trk_d0; }

      TLorentzVector vv;
      // TODO: use values computed w.r.t SV
      vv.SetPtEtaPhiM(trk.pt(),trk.eta(), trk.phi0(), trk.m());
      sumP4 += vv;
      H += vv.Vect().Mag();
      HT += vv.Pt();

      TLorentzVector v_minus_iv(0,0,0,0);
      for(size_t j = 0; j < ntracks; j++){
        if (j == t){ continue; }
        if(!trkParts[j].isValid()){
          ATH_MSG_DEBUG("Track " << j << " is bad!");
          continue;
        }

        const xAOD::TrackParticle & trk_2 = **trkParts[j];

        TLorentzVector tmp;
        // TODO: use values computed w.r.t. SV
        tmp.SetPtEtaPhiM(trk_2.pt(),trk_2.eta(), trk_2.phi0(), trk_2.m());
        v_minus_iv += tmp;

        if( j > t ) {
          double tm = vv * tmp / ( vv.Mag() * tmp.Mag() );
          if( minOpAng < tm ) minOpAng = tm;
          if( maxOpAng > tm ) maxOpAng = tm;
        }
      }
      double DR = vv.DeltaR(v_minus_iv);
      if( DR > maxDR ){ maxDR = DR;}

      charge += trk.charge();

      xAOD::TrackParticle::ConstAccessor<float> Trk_Chi2("chiSquared");
      xAOD::TrackParticle::ConstAccessor<float> Trk_nDoF("numberDoF");

      const bool hasChi2 = Trk_Chi2.isAvailable(trk) && Trk_Chi2(trk) && Trk_nDoF.isAvailable(trk) && Trk_nDoF(trk);
      const auto& covDiag = trk.definingParametersCovMatrixDiagVec();

      for (const RecoVertexHists* h : categories) {
        if ( hasChi2 )  {
          h->Trk_Chi2->Fill(Trk_Chi2(trk) / Trk_nDoF(trk));
          h->Trk_nDoF->Fill(Trk_nDoF(trk));
        }
        h->Trk_D0->Fill(trk_d0);
        h->Trk_Z0->Fill(trk_z0);
        h->Trk_theta->Fill(trk.definingParameters()[3]);
        h->Trk_qOverP->Fill(trk.definingParameters()[4]);
        h->Trk_Eta->Fill(trk.eta());
        h->Trk_Phi->Fill(trk.phi0());
        h->Trk_E->Fill(trk.e() / GeV);
        h->Trk_M->Fill(trk.m() / GeV);
        h->Trk_Pt->Fill(trk.pt() / GeV);
        h->Trk_Px->Fill(trk.p4().Px() / GeV);
        h->Trk_Py->Fill(trk.p4().Py() / GeV);
        h->Trk_Pz->Fill(trk.p4().Pz() / GeV);
        h->Trk_charge->Fill(trk.charge());
        if (covDiag.size() > 1) {
          h->Trk_errD0->Fill(std::sqrt(covDiag[0]));
          h->Trk_errZ0->Fill(std::sqrt(covDiag[1]));
        }
      }

    } // end loop over tracks

    const double sumP3Mag = sumP4.Vect().Mag();
    const double recoPosMag = reco_pos.Mag();

    xAOD::Vertex::ConstAccessor<float> Chi2("chiSquared");
    xAOD::Vertex::ConstAccessor<float> nDoF("numberDoF");

    for (const RecoVertexHists* h : categories) {
      h->x->Fill(secVtx->x());
      h->y->Fill(secVtx->y());
      h->z->Fill(secVtx->z());
      h->Lxy->Fill(Lxy);
      h->ntrk->Fill(ntracks);
      h->pT->Fill(sumP4.Pt() / GeV);
      h->eta->Fill(sumP4.Eta());
      h->phi->Fill(sumP4.Phi());
      h->mass->Fill(sumP4.M() / GeV);
      if (maxDR > 0) {
        h->mu->Fill(sumP4.M() / maxDR / GeV);
      }
      h->chi2->Fill(Chi2(*secVtx)/nDoF(*secVtx));
      if (sumP3Mag > 0 && recoPosMag > 0) {
        h->dir->Fill(sumP4.Vect().Dot( reco_pos ) / sumP3Mag / recoPosMag);
      }
      h->charge->Fill(charge);
      h->H->Fill(H / GeV);
      h->HT->Fill(HT / GeV);
      if (nValidTracks > 1) {
        h->minOpAng->Fill(minOpAng);
        h->maxOpAng->Fill(maxOpAng);
      }
      if (nValidTracks > 0) {
        h->mind0->Fill(minD0);
        h->maxd0->Fill(maxD0);
      }
      h->maxdR->Fill(maxDR);

      // This includes all matched vertices, including splits
      if (h->matchScore_weight) {
        const auto& truthmatchinfo = matchInfoAcc(*secVtx);
        if(not truthmatchinfo.empty()){
          float matchScore_weight = std::get<1>(truthmatchinfo.at(0));
          float matchScore_pt     = std::get<2>(truthmatchinfo.at(0));

          ATH_MSG_DEBUG("Match Score and probability: " << matchScore_weight << " " << matchScore_pt/0.01);

          const ElementLink<xAOD::TruthVertexContainer>& truthVertexLink = std::get<0>(truthmatchinfo.at(0));
          const xAOD::TruthVertex& truthVtx = **truthVertexLink ;

          h->positionRes_R->Fill(Lxy - truthVtx.perp());
          h->positionRes_Z->Fill(secVtx->z() - truthVtx.z());
          h->matchScore_weight->Fill(matchScore_weight);
          h->matchScore_pt->Fill(matchScore_pt);
        }
      }
    }
  }

  void SecVertexTruthMatchAlg::fillTruthHistograms(const xAOD::TruthVertex* truthVtx, const std::string& truthType) {

    hist("TruthVertex/" + truthType + "_x")->Fill(truthVtx->x());
    hist("TruthVertex/" + truthType + "_y")->Fill(truthVtx->y());
    hist("TruthVertex/" + truthType + "_z")->Fill(truthVtx->z());
    hist("TruthVertex/" + truthType + "_R")->Fill(truthVtx->perp());
    hist("TruthVertex/" + truthType + "_Eta")->Fill(truthVtx->eta());
    hist("TruthVertex/" + truthType + "_Phi")->Fill(truthVtx->phi());
    hist("TruthVertex/" + truthType + "_Ntrk_out")->Fill(truthVtx->nOutgoingParticles());

    ATH_MSG_DEBUG("Plotting truth parent");
    const xAOD::TruthParticle& truthPart = *truthVtx->incomingParticle(0);

    hist("TruthVertex/" + truthType + "_Parent_E")->Fill(truthPart.e() / GeV);
    hist("TruthVertex/" + truthType + "_Parent_M")->Fill(truthPart.m() / GeV);
    hist("TruthVertex/" + truthType + "_Parent_Pt")->Fill(truthPart.pt() / GeV);
    hist("TruthVertex/" + truthType + "_Parent_Phi")->Fill(truthPart.phi());
    hist("TruthVertex/" + truthType + "_Parent_Eta")->Fill(truthPart.eta());
    hist("TruthVertex/" + truthType + "_Parent_charge")->Fill(truthPart.charge());

    ATH_MSG_DEBUG("Plotting truth prod vtx");
    if(truthPart.hasProdVtx()){
      const xAOD::TruthVertex & vertex = *truthPart.prodVtx();

      hist("TruthVertex/" + truthType + "_ParentProdX")->Fill(vertex.x());
      hist("TruthVertex/" + truthType + "_ParentProdY")->Fill(vertex.y());
      hist("TruthVertex/" + truthType + "_ParentProdZ")->Fill(vertex.z());
      hist("TruthVertex/" + truthType + "_ParentProdR")->Fill(vertex.perp());
      hist("TruthVertex/" + truthType + "_ParentProdEta")->Fill(vertex.eta());
      hist("TruthVertex/" + truthType + "_ParentProdPhi")->Fill(vertex.phi());
    }
  }

} // namespace CP
