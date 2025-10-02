/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h"
#include "InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchTool.h"
#include "xAODTruth/TruthParticle.h"

#include "TMath.h"
#include "TH1.h"
#include "TEfficiency.h"

const float GeV = 1000.;

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
      

      for(const auto& recoType : recoTypes) {
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_x").c_str(), "Reco vertex x [mm]", 1000, -maxX, maxX)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_y").c_str(), "Reco vertex y [mm]", 1000, -maxY, maxY)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_z").c_str(), "Reco vertex z [mm]", 1000, -maxZ, maxZ)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Lxy").c_str(), "Reco vertex L_{xy} [mm]", 500, 0, maxLxy)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_pT").c_str(), "Reco vertex p_{T} [GeV]", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_eta").c_str(), "Reco vertex #eta", 100, -5, 5)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_phi").c_str(), "Reco vertex #phi", 100, -TMath::Pi(), TMath::Pi())));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_mass").c_str(), "Reco vertex mass [GeV]", 500, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_mu").c_str(), "Reco vertex Red. Mass [GeV]", 500, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_chi2").c_str(), "Reco vertex recoChi2", 100, 0, 10)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_dir").c_str(), "Reco vertex recoDirection", 100, -1, 1)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_charge").c_str(), "Reco vertex recoCharge", 20, -10, 10)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_H").c_str(), "Reco vertex H [GeV]", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_HT").c_str(), "Reco vertex Mass [GeV]", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_minOpAng").c_str(), "Reco vertex minOpAng", 100, -1, 1)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_maxOpAng").c_str(), "Reco vertex maxOpAng", 100, -1, 1)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_maxdR").c_str(), "Reco vertex maxDR", 100, 0, 10)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_mind0").c_str(), "Reco vertex min d0 [mm]", 100, 0, mind0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_maxd0").c_str(), "Reco vertex max d0 [mm]", 100, 0, maxd0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_ntrk").c_str(), "Reco vertex n tracks", 30, 0, 30)));

        // tracks
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_qOverP").c_str(), "Reco track qOverP ", 100, 0, .01)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_theta").c_str(), "Reco track theta ", 64, 0, 3.2)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_E").c_str(), "Reco track E ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_M").c_str(), "Reco track M ", 100, 0, 10)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Pt").c_str(), "Reco track Pt ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Px").c_str(), "Reco track Px ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Py").c_str(), "Reco track Py ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Pz").c_str(), "Reco track Pz ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Eta").c_str(), "Reco track Eta ", 100, -5, 5)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Phi").c_str(), "Reco track Phi ", 63, -3.2, 3.2)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_D0").c_str(), "Reco track D0 ", 300, -maxTrackd0, maxTrackd0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Z0").c_str(), "Reco track Z0 ", 500, -maxTrackz0, maxTrackz0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_errD0").c_str(), "Reco track errD0 ", 300, 0, maxErrd0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_errZ0").c_str(), "Reco track errZ0 ", 500, 0, maxErrz0)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_Chi2").c_str(), "Reco track Chi2 ", 100, 0, 10)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_nDoF").c_str(), "Reco track nDoF ", 100, 0, 100)));
        ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_Trk_charge").c_str(), "Reco track charge ", 3, -1.5, 1.5)));

        // truth matching -- don't book for non-matched vertices
        if ( recoType != "All" and recoType != "Fake" ) {
          ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_positionRes_R").c_str(), "Position resolution for vertices matched to truth decays", 400, -maxResR, maxResR)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_positionRes_Z").c_str(), "Position resolution for vertices matched to truth decays", 400, -maxResZ, maxResZ)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_matchScore_weight").c_str(), "Vertex Match Score (weight)", 101, 0, 1.01)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_matchScore_pt").c_str(), "Vertex Match Score (pT)", 101, 0, 1.01)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + recoType + "_matchedTruthID").c_str(), "Vertex Truth Match ID", 100, 0, 100)));
        }
      }

      // do reco vertices by SM origin if enabled -- NOTE these types are not exclusive (d decays will also be b decays in a cascade etc)
      if (m_doSMOrigin) {
        for(const auto& smOriginType : smOriginTypes) {
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_x").c_str(), "Reco vertex x [mm]", 1000, -maxX, maxX)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_y").c_str(), "Reco vertex y [mm]", 1000, -maxY, maxY)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_z").c_str(), "Reco vertex z [mm]", 1000, -maxZ, maxZ)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Lxy").c_str(), "Reco vertex L_{xy} [mm]", 500, 0, maxLxy)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_pT").c_str(), "Reco vertex p_{T} [GeV]", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_eta").c_str(), "Reco vertex #eta", 100, -5, 5)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_phi").c_str(), "Reco vertex #phi", 100, -TMath::Pi(), TMath::Pi())));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_mass").c_str(), "Reco vertex mass [GeV]", 500, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_mu").c_str(), "Reco vertex Red. Mass [GeV]", 500, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_chi2").c_str(), "Reco vertex recoChi2", 100, 0, 10)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_dir").c_str(), "Reco vertex recoDirection", 100, -1, 1)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_charge").c_str(), "Reco vertex recoCharge", 20, -10, 10)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_H").c_str(), "Reco vertex H [GeV]", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_HT").c_str(), "Reco vertex Mass [GeV]", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_minOpAng").c_str(), "Reco vertex minOpAng", 100, -1, 1)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_maxOpAng").c_str(), "Reco vertex maxOpAng", 100, -1, 1)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_maxdR").c_str(), "Reco vertex maxDR", 100, 0, 10)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_mind0").c_str(), "Reco vertex min d0 [mm]", 100, 0, mind0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_maxd0").c_str(), "Reco vertex max d0 [mm]", 100, 0, maxd0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_ntrk").c_str(), "Reco vertex n tracks", 30, 0, 30)));

          // tracks
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_qOverP").c_str(), "Reco track qOverP ", 100, 0, .01)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_theta").c_str(), "Reco track theta ", 64, 0, 3.2)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_E").c_str(), "Reco track E ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_M").c_str(), "Reco track M ", 100, 0, 10)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Pt").c_str(), "Reco track Pt ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Px").c_str(), "Reco track Px ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Py").c_str(), "Reco track Py ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Pz").c_str(), "Reco track Pz ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Eta").c_str(), "Reco track Eta ", 100, -5, 5)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Phi").c_str(), "Reco track Phi ", 63, -3.2, 3.2)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_D0").c_str(), "Reco track D0 ", 300, -maxTrackd0, maxTrackd0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Z0").c_str(), "Reco track Z0 ", 500, -maxTrackz0, maxTrackz0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_errD0").c_str(), "Reco track errD0 ", 300, 0, maxErrd0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_errZ0").c_str(), "Reco track errZ0 ", 500, 0, maxErrz0)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_Chi2").c_str(), "Reco track Chi2 ", 100, 0, 10)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_nDoF").c_str(), "Reco track nDoF ", 100, 0, 100)));
          ANA_CHECK (book(TH1F(("RecoVertex/" + smOriginType + "_Trk_charge").c_str(), "Reco track charge ", 3, -1.5, 1.5)));

          
        }
      }


      for(const auto& truthType : truthTypes) {
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_x").c_str(), "Truth vertex x [mm]", 1000, -maxX, maxX)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_y").c_str(), "Truth vertex y [mm]", 500, -maxY, maxY)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_z").c_str(), "Truth vertex z [mm]", 500, -maxZ, maxZ)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_R").c_str(), "Truth vertex r [mm]", 6000, 0, maxR)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_distFromPV").c_str(), "Truth vertex distFromPV [mm]", 600, 0, maxR)));
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
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProddistFromPV").c_str(), "truthParentProd vertex distFromPV [mm]", 500, 0, 500)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdEta").c_str(), "truthParentProd vertex Eta", 100, -5, 5)));
				ANA_CHECK (book(TH1F(("TruthVertex/" + truthType + "_ParentProdPhi").c_str(), "truthParentProd vertex Phi", 64, -3.2, 3.2)));
			}
      // now add the efficiencies
      // Define two different bin arrays - one for standard mode, one for MuSA
      Double_t standard_bins[] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 35, 40, 50, 60, 70, 80, 90, 100, 125, 150, 200, 300, 500};
      Double_t muSA_bins[] = {0.0, 1, 5, 10, 20, 50, 100, 200, 300, 500, 750, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000};

      // Determine which bins to use based on whether we're in MuSA mode
      Double_t* bins = m_doMuSA ? muSA_bins : standard_bins;
      size_t nbins = m_doMuSA ? sizeof(muSA_bins)/sizeof(muSA_bins[0])-1 : sizeof(standard_bins)/sizeof(standard_bins[0])-1;

      ANA_CHECK (book(TEfficiency("Acceptance", "Acceptance", nbins, bins)));
      ANA_CHECK (book(TEfficiency("eff_seed", "Seed efficiency", nbins, bins)));
      ANA_CHECK (book(TEfficiency("eff_core", "Core efficiency", nbins, bins)));
      ANA_CHECK (book(TEfficiency("eff_total", "Total efficiency", nbins, bins)));
      
    }


    return StatusCode::SUCCESS;
  }

  StatusCode SecVertexTruthMatchAlg::execute() {

    //Retrieve the vertices:
    SG::ReadHandle<xAOD::VertexContainer> recoVertexContainer(m_secVtxContainerKey);
    SG::ReadHandle<xAOD::TruthVertexContainer> truthVertexContainer(m_truthVtxContainerKey);
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleContainer(m_trackParticleContainerKey);

    std::vector<const xAOD::Vertex*> recoVerticesToMatch;
    std::vector<const xAOD::TruthVertex*> truthVerticesToMatch;

    for(const auto recoVertex : *recoVertexContainer) {
      xAOD::VxType::VertexType vtxType = static_cast<xAOD::VxType::VertexType>( recoVertex->vertexType() );

      if(vtxType != xAOD::VxType::SecVtx ){
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
      if(std::find(m_targetPDGIDs.begin(), m_targetPDGIDs.end(), std::abs(truthPart->pdgId())) == m_targetPDGIDs.end()) {
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
      static const xAOD::Vertex::Decorator<int> matchTypeDecor("vertexMatchType");
      for(const auto& secVtx : recoVerticesToMatch) {
        int matchTypeBitset = matchTypeDecor(*secVtx);
        hist("RecoVertex/matchType")->Fill(matchTypeBitset);

        if(InDetSecVtxTruthMatchUtils::isMatched(matchTypeBitset)) {
          fillRecoHistograms(secVtx, "Matched");
        }
        if(InDetSecVtxTruthMatchUtils::isMerged(matchTypeBitset)) {
          fillRecoHistograms(secVtx, "Merged");
        }
        if(InDetSecVtxTruthMatchUtils::isFake(matchTypeBitset)) {
          fillRecoHistograms(secVtx, "Fake");
        }
        if(InDetSecVtxTruthMatchUtils::isSplit(matchTypeBitset)) {
          fillRecoHistograms(secVtx, "Split");
        }
        if(InDetSecVtxTruthMatchUtils::isOther(matchTypeBitset)) {
          fillRecoHistograms(secVtx, "Other");
        }
        fillRecoHistograms(secVtx, "All");
      }

      static const xAOD::Vertex::Decorator<int> truthTypeDecor("truthVertexMatchType");
      for(const auto& truthVtx : truthVerticesToMatch) {
        int truthTypeBitset = truthTypeDecor(*truthVtx);
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
      if (m_doSMOrigin) {
        static const xAOD::Vertex::Decorator<int> originTypeDecor("vertexMatchOriginType");

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

        for(const auto& secVtx : recoVerticesToMatch) {
          int smOriginTypeBitset = originTypeDecor(*secVtx);
          hist("RecoVertex/smOriginType")->Fill(smOriginTypeBitset);

          for(const auto& entry : originTypeMap) {
            if(InDetSecVtxTruthMatchUtils::isOriginType(smOriginTypeBitset, entry.first)) {
              fillOriginHistograms(secVtx, entry.second);
            }
          }
        }
      }
      
    }

    return StatusCode::SUCCESS;

  }
  void SecVertexTruthMatchAlg::fillRecoHistograms(const xAOD::Vertex* secVtx, const std::string& matchType) {

		// set of accessors for tracks and weights
		xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
		xAOD::Vertex::ConstAccessor<std::vector<float> > weightAcc("trackWeights");

		// set of decorators for truth matching info
		const xAOD::Vertex::Decorator<std::vector<InDetSecVtxTruthMatchUtils::VertexTruthMatchInfo> > matchInfoDecor("truthVertexMatchingInfos");

    TVector3 reco_pos(secVtx->x(), secVtx->y(), secVtx->z());
    float Lxy = reco_pos.Perp();

    size_t ntracks;
    const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( *secVtx );
    ntracks = trkParts.size();

    TLorentzVector sumP4(0,0,0,0);
    double H = 0.0;
    double HT = 0.0;
    int charge = 0;
    double minOpAng = -1.0* 1.e10;
    double maxOpAng =  1.0* 1.e10;
    double minD0 = 1.0* 1.e10;
    double maxD0 = 0.0;
    double maxDR = 0.0;

    xAOD::TrackParticle::ConstAccessor< std::vector< float > > accCovMatrixDiag( "definingParametersCovMatrixDiag" );

    ATH_MSG_DEBUG("Loop over tracks");
    for(size_t t = 0; t < ntracks; t++){
      if(!trkParts[t].isValid()){
        ATH_MSG_DEBUG("Track " << t << " is bad!");
        continue;
      }
      const xAOD::TrackParticle & trk = **trkParts[t];

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

      if ( Trk_Chi2.isAvailable(trk) && Trk_Chi2(trk) && Trk_nDoF.isAvailable(trk) && Trk_nDoF(trk) )  {
        hist("RecoVertex/" + matchType + "_Trk_Chi2")->Fill(Trk_Chi2(trk) / Trk_nDoF(trk));
        hist("RecoVertex/" + matchType + "_Trk_nDoF")->Fill(Trk_nDoF(trk));
      }
      hist("RecoVertex/" + matchType + "_Trk_D0")->Fill(trk_d0);
      hist("RecoVertex/" + matchType + "_Trk_Z0")->Fill(trk_z0);
      hist("RecoVertex/" + matchType + "_Trk_theta")->Fill(trk.definingParameters()[3]);
      hist("RecoVertex/" + matchType + "_Trk_qOverP")->Fill(trk.definingParameters()[4]);
      hist("RecoVertex/" + matchType + "_Trk_Eta")->Fill(trk.eta());
      hist("RecoVertex/" + matchType + "_Trk_Phi")->Fill(trk.phi0());
      hist("RecoVertex/" + matchType + "_Trk_E")->Fill(trk.e() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_M")->Fill(trk.m() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_Pt")->Fill(trk.pt() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_Px")->Fill(trk.p4().Px() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_Py")->Fill(trk.p4().Py() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_Pz")->Fill(trk.p4().Pz() / GeV);
      hist("RecoVertex/" + matchType + "_Trk_charge")->Fill(trk.charge());
      hist("RecoVertex/" + matchType + "_Trk_errD0")->Fill(trk.definingParametersCovMatrix()(0,0));
      hist("RecoVertex/" + matchType + "_Trk_errZ0")->Fill(trk.definingParametersCovMatrix()(1,1));

    } // end loop over tracks

    const double dir  = sumP4.Vect().Dot( reco_pos ) / sumP4.Vect().Mag() / reco_pos.Mag();

    xAOD::Vertex::ConstAccessor<float> Chi2("chiSquared");
    xAOD::Vertex::ConstAccessor<float> nDoF("numberDoF");

    hist("RecoVertex/" + matchType + "_x")->Fill(secVtx->x());
    hist("RecoVertex/" + matchType + "_y")->Fill(secVtx->y());
    hist("RecoVertex/" + matchType + "_z")->Fill(secVtx->z());
    hist("RecoVertex/" + matchType + "_Lxy")->Fill(Lxy);
    hist("RecoVertex/" + matchType + "_ntrk")->Fill(ntracks);
    hist("RecoVertex/" + matchType + "_pT")->Fill(sumP4.Pt() / GeV);
    hist("RecoVertex/" + matchType + "_eta")->Fill(sumP4.Eta());
    hist("RecoVertex/" + matchType + "_phi")->Fill(sumP4.Phi());
    hist("RecoVertex/" + matchType + "_mass")->Fill(sumP4.M() / GeV);
    hist("RecoVertex/" + matchType + "_mu")->Fill(sumP4.M()/maxDR / GeV);
    hist("RecoVertex/" + matchType + "_chi2")->Fill(Chi2(*secVtx)/nDoF(*secVtx));
    hist("RecoVertex/" + matchType + "_dir")->Fill(dir);
    hist("RecoVertex/" + matchType + "_charge")->Fill(charge);
    hist("RecoVertex/" + matchType + "_H")->Fill(H / GeV);
    hist("RecoVertex/" + matchType + "_HT")->Fill(HT / GeV);
    hist("RecoVertex/" + matchType + "_minOpAng")->Fill(minOpAng);
    hist("RecoVertex/" + matchType + "_maxOpAng")->Fill(maxOpAng);
    hist("RecoVertex/" + matchType + "_mind0")->Fill(minD0);
    hist("RecoVertex/" + matchType + "_maxd0")->Fill(maxD0);
    hist("RecoVertex/" + matchType + "_maxdR")->Fill(maxDR);

    std::vector<InDetSecVtxTruthMatchUtils::VertexTruthMatchInfo> truthmatchinfo;
    truthmatchinfo = matchInfoDecor(*secVtx);

    // This includes all matched vertices, including splits
    if (matchType != "All" and matchType != "Fake") {
      if(not truthmatchinfo.empty()){
        float matchScore_weight = std::get<1>(truthmatchinfo.at(0));
        float matchScore_pt     = std::get<2>(truthmatchinfo.at(0));

        ATH_MSG_DEBUG("Match Score and probability: " << matchScore_weight << " " << matchScore_pt/0.01);

        const ElementLink<xAOD::TruthVertexContainer>& truthVertexLink = std::get<0>(truthmatchinfo.at(0));
        const xAOD::TruthVertex& truthVtx = **truthVertexLink ;

        hist("RecoVertex/" + matchType + "_positionRes_R")->Fill(Lxy - truthVtx.perp());
        hist("RecoVertex/" + matchType + "_positionRes_Z")->Fill(secVtx->z() - truthVtx.z());
        hist("RecoVertex/" + matchType + "_matchScore_weight")->Fill(matchScore_weight);
        hist("RecoVertex/" + matchType + "_matchScore_pt")->Fill(matchScore_pt);
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
    // m_truthInclusive_r->Fill(truthVtx.perp());

    // if(matchTypeDecor(truthVtx) >= RECONSTRUCTABLE){
    //   m_truthReconstructable_r->Fill(truthVtx.perp());
    // }
    // if(matchTypeDecor(truthVtx) >= ACCEPTED){
    //   m_truthAccepted_r->Fill(truthVtx.perp());
    // }
    // if(matchTypeDecor(truthVtx) >= SEEDED){
    //   m_truthSeeded_r->Fill(truthVtx.perp());
    // }
    // if(matchTypeDecor(truthVtx) >= RECONSTRUCTED){
    //   m_truthReconstructed_r->Fill(truthVtx.perp());
    // }
    // if(matchTypeDecor(truthVtx) >= RECONSTRUCTEDSPLIT){
    //   m_truthSplit_r->Fill(truthVtx.perp());
    // }
    //
	}

  void SecVertexTruthMatchAlg::fillOriginHistograms(const xAOD::Vertex* secVtx, const std::string& originType) {

    // set of accessors for tracks and weights
    xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
    xAOD::Vertex::ConstAccessor<std::vector<float> > weightAcc("trackWeights");

    TVector3 reco_pos(secVtx->x(), secVtx->y(), secVtx->z());
    float Lxy = reco_pos.Perp();

    size_t ntracks;
    const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( *secVtx );
    ntracks = trkParts.size();

    TLorentzVector sumP4(0,0,0,0);
    double H = 0.0;
    double HT = 0.0;
    int charge = 0;
    double minOpAng = -1.0* 1.e10;
    double maxOpAng =  1.0* 1.e10;
    double minD0 = 1.0* 1.e10;
    double maxD0 = 0.0;
    double maxDR = 0.0;

    xAOD::TrackParticle::ConstAccessor< std::vector< float > > accCovMatrixDiag( "definingParametersCovMatrixDiag" );

    // Loop over tracks to calculate derived quantities
    for(size_t t = 0; t < ntracks; t++){
      if(!trkParts[t].isValid()){
        continue;
      }
      const xAOD::TrackParticle & trk = **trkParts[t];

      double trk_d0 = std::abs(trk.definingParameters()[0]);
      double trk_z0 = std::abs(trk.definingParameters()[1]);

      if(trk_d0 < minD0){ minD0 = trk_d0; }
      if(trk_d0 > maxD0){ maxD0 = trk_d0; }

      TLorentzVector vv;
      vv.SetPtEtaPhiM(trk.pt(), trk.eta(), trk.phi0(), trk.m());
      sumP4 += vv;
      H += vv.Vect().Mag();
      HT += vv.Pt();

      TLorentzVector v_minus_iv(0,0,0,0);
      for(size_t j = 0; j < ntracks; j++){
        if (j == t){ continue; }
        if(!trkParts[j].isValid()){
          continue;
        }

        const xAOD::TrackParticle & trk_2 = **trkParts[j];

        TLorentzVector tmp;
        tmp.SetPtEtaPhiM(trk_2.pt(), trk_2.eta(), trk_2.phi0(), trk_2.m());
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

      // Fill track-level histograms
      xAOD::TrackParticle::ConstAccessor<float> Trk_Chi2("chiSquared");
      xAOD::TrackParticle::ConstAccessor<float> Trk_nDoF("numberDoF");

      if ( Trk_Chi2.isAvailable(trk) && Trk_Chi2(trk) && Trk_nDoF.isAvailable(trk) && Trk_nDoF(trk) )  {
        hist("RecoVertex/" + originType + "_Trk_Chi2")->Fill(Trk_Chi2(trk) / Trk_nDoF(trk));
        hist("RecoVertex/" + originType + "_Trk_nDoF")->Fill(Trk_nDoF(trk));
      }
      hist("RecoVertex/" + originType + "_Trk_D0")->Fill(trk_d0);
      hist("RecoVertex/" + originType + "_Trk_Z0")->Fill(trk_z0);
      hist("RecoVertex/" + originType + "_Trk_theta")->Fill(trk.definingParameters()[3]);
      hist("RecoVertex/" + originType + "_Trk_qOverP")->Fill(trk.definingParameters()[4]);
      hist("RecoVertex/" + originType + "_Trk_Eta")->Fill(trk.eta());
      hist("RecoVertex/" + originType + "_Trk_Phi")->Fill(trk.phi0());
      hist("RecoVertex/" + originType + "_Trk_E")->Fill(trk.e() / GeV);
      hist("RecoVertex/" + originType + "_Trk_M")->Fill(trk.m() / GeV);
      hist("RecoVertex/" + originType + "_Trk_Pt")->Fill(trk.pt() / GeV);
      hist("RecoVertex/" + originType + "_Trk_Px")->Fill(trk.p4().Px() / GeV);
      hist("RecoVertex/" + originType + "_Trk_Py")->Fill(trk.p4().Py() / GeV);
      hist("RecoVertex/" + originType + "_Trk_Pz")->Fill(trk.p4().Pz() / GeV);
      hist("RecoVertex/" + originType + "_Trk_charge")->Fill(trk.charge());
      hist("RecoVertex/" + originType + "_Trk_errD0")->Fill(trk.definingParametersCovMatrix()(0,0));
      hist("RecoVertex/" + originType + "_Trk_errZ0")->Fill(trk.definingParametersCovMatrix()(1,1));
    }

    const double dir = sumP4.Vect().Dot( reco_pos ) / sumP4.Vect().Mag() / reco_pos.Mag();

    xAOD::Vertex::ConstAccessor<float> Chi2("chiSquared");
    xAOD::Vertex::ConstAccessor<float> nDoF("numberDoF");

    // Fill vertex-level histograms
    hist("RecoVertex/" + originType + "_x")->Fill(secVtx->x());
    hist("RecoVertex/" + originType + "_y")->Fill(secVtx->y());
    hist("RecoVertex/" + originType + "_z")->Fill(secVtx->z());
    hist("RecoVertex/" + originType + "_Lxy")->Fill(Lxy);
    hist("RecoVertex/" + originType + "_ntrk")->Fill(ntracks);
    hist("RecoVertex/" + originType + "_pT")->Fill(sumP4.Pt() / GeV);
    hist("RecoVertex/" + originType + "_eta")->Fill(sumP4.Eta());
    hist("RecoVertex/" + originType + "_phi")->Fill(sumP4.Phi());
    hist("RecoVertex/" + originType + "_mass")->Fill(sumP4.M() / GeV);
    hist("RecoVertex/" + originType + "_mu")->Fill(sumP4.M()/maxDR / GeV);
    hist("RecoVertex/" + originType + "_chi2")->Fill(Chi2(*secVtx)/nDoF(*secVtx));
    hist("RecoVertex/" + originType + "_dir")->Fill(dir);
    hist("RecoVertex/" + originType + "_charge")->Fill(charge);
    hist("RecoVertex/" + originType + "_H")->Fill(H / GeV);
    hist("RecoVertex/" + originType + "_HT")->Fill(HT / GeV);
    hist("RecoVertex/" + originType + "_minOpAng")->Fill(minOpAng);
    hist("RecoVertex/" + originType + "_maxOpAng")->Fill(maxOpAng);
    hist("RecoVertex/" + originType + "_mind0")->Fill(minD0);
    hist("RecoVertex/" + originType + "_maxd0")->Fill(maxD0);
    hist("RecoVertex/" + originType + "_maxdR")->Fill(maxDR);
  }

} // namespace CP
