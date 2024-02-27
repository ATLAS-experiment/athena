/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaKernel/Units.h"

#include "InDetSecVertexValidation/InDetSecVertexTruthMatchTool.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

using namespace InDetSecVertexTruthMatchUtils;
using Athena::Units::GeV;

InDetSecVertexTruthMatchTool::InDetSecVertexTruthMatchTool( const std::string & name ) : asg::AsgTool(name) {
  declareProperty("trackMatchProb", m_trkMatchProb = 0.5 );
  declareProperty("vertexMatchWeight", m_vxMatchWeight = 0.5 );
  declareProperty("trackPtCut", m_trkPtCut = 1000. );
  declareProperty("pdgIds", m_pdgIds = "36" );
  declareProperty("fillHist", m_fillHist = true );
  declareProperty("AugString", m_AugString = "" );
}

StatusCode InDetSecVertexTruthMatchTool::initialize() {
  ATH_MSG_INFO("Initializing InDetSecVertexTruthMatchTool");

  // build the vector of pdgIds from the input string
  std::stringstream ss(m_pdgIds);
  for (int i; ss >> i;) {
      m_pdgIdList.push_back(i);    
      if (ss.peek() == ',')
          ss.ignore();
  }

  // histograms
  ATH_CHECK( service("THistSvc",m_thistSvc) );

  ////////////////////////////////////////////
  ////// Seconvery Vertex Histograms /////////
  ////////////////////////////////////////////
  m_matchType                 = new TH1F("reco_matchType","Vertex Match Type",8,-0.5,7.5);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/matchType", m_matchType));

  for(auto type : allTypes) {
    std::string vertexType = "";
    switch (type)
    {
    case MATCHED:
      vertexType = "Matched";
      break;
    case MERGED:
      vertexType = "Merged";
      break;
    case SPLIT:
      vertexType = "Split";
      break;
    case LLP:
      vertexType = "LLP";
      break;
    case OTHER:
      vertexType = "Other";
      break;
    case FAKE:
      vertexType = "Fake";
      break;
    case ALL:
      vertexType = "All";
      break;
    default:
      break;
    }


    m_recoX[vertexType]                  = new TH1F(("recoZ" + vertexType).c_str(),"Reco vertex x [mm]",500,0,500); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/X", m_recoX[vertexType]));
    m_recoY[vertexType]                  = new TH1F(("recoY" + vertexType).c_str(),"Reco vertex y [mm]",500,0,500); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Y", m_recoY[vertexType]));
    m_recoZ[vertexType]                  = new TH1F(("recoZ" + vertexType).c_str(),"Reco vertex z [mm]",500,0,500); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Z", m_recoZ[vertexType]));
    m_recoR[vertexType]                  = new TH1F(("recoR" + vertexType).c_str(),"Reco vertex r [mm]",500,0,500); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/R", m_recoR[vertexType]));
    m_recoPt[vertexType]                 = new TH1F(("recoPt" + vertexType).c_str(),"Reco vertex Pt [GeV]",100,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Pt", m_recoPt[vertexType]));
    m_recoEta[vertexType]                = new TH1F(("recoEta" + vertexType).c_str(),"Reco vertex Eta ",100,-5,5); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Eta", m_recoEta[vertexType]));
    m_recoPhi[vertexType]                = new TH1F(("recoPhi" + vertexType).c_str(),"Reco vertex Phi ",64,-3.2,3.2);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Phi", m_recoPhi[vertexType]));
    m_recoMass[vertexType]               = new TH1F(("recoMass" + vertexType).c_str(),"Reco vertex Mass [GeV]",500,0,100);
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Mass", m_recoMass[vertexType]));
    m_recoMu[vertexType]                 = new TH1F(("recoMu" + vertexType).c_str(),"Reco vertex Red. Mass [GeV]",500,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Mu", m_recoMu[vertexType]));
    m_recoChi2[vertexType]               = new TH1F(("recoChi2" + vertexType).c_str(),"Reco vertex recoChi2",100,0,10); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Chi2", m_recoChi2[vertexType]));
    m_recoDir[vertexType]                = new TH1F(("recoDir" + vertexType).c_str(),"Reco vertex recoDirection",100,-1,1);
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Dir", m_recoDir[vertexType]));
    m_recoCharge[vertexType]             = new TH1F(("recoCharge" + vertexType).c_str(),"Reco vertex recoCharge",20,-10,10);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Charge", m_recoCharge[vertexType]));
    m_recoH[vertexType]                  = new TH1F(("recoH" + vertexType).c_str(),"Reco vertex H [GeV]",100,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/H", m_recoH[vertexType]));
    m_recoHt[vertexType]                 = new TH1F(("recoHt" + vertexType).c_str(),"Reco vertex Mass [GeV]",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Ht", m_recoHt[vertexType]));
    m_recoMinOpAng[vertexType]           = new TH1F(("recoMinOpAng" + vertexType).c_str(),"Reco vertex minOpAng",100,-1,1); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/MinOpAng", m_recoMinOpAng[vertexType]));
    m_recoMaxOpAng[vertexType]           = new TH1F(("recoMaxOpAng" + vertexType).c_str(),"Reco vertex MaxOpAng",100,-1,1); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/MaxOpAng", m_recoMaxOpAng[vertexType]));
    m_recoMaxDR[vertexType]              = new TH1F(("recoMaxDR" + vertexType).c_str(),"Reco vertex maxDR",100,0,10);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/MaxDR", m_recoMaxDR[vertexType]));
    m_recoMinD0[vertexType]              = new TH1F(("recoMinD0" + vertexType).c_str(),"Reco vertex min d0 [mm]",100,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/MinD0", m_recoMinD0[vertexType]));
    m_recoMaxD0[vertexType]              = new TH1F(("recoMaxD0" + vertexType).c_str(),"Reco vertex max d0 [mm]",100,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/MaxD0", m_recoMaxD0[vertexType]));
    m_recoNtrk[vertexType]               = new TH1F(("recoNtrk" + vertexType).c_str(),"Reco vertex n tracks",30,0,30);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Ntrk", m_recoNtrk[vertexType]));

    m_positionRes_R[vertexType]             = new TH1F(("positionRes_R" + vertexType).c_str(),"Position resolution for vertices matched to truth  decays",400,-20,20);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/positionRes_R", m_positionRes_R[vertexType]));
    m_positionRes_Z[vertexType]             = new TH1F(("positionRes_Z" + vertexType).c_str(),"Position resolution for vertices matched to truth  decays",400,-20,20);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/positionRes_Z", m_positionRes_Z[vertexType]));
    m_matchScore_weight[vertexType]          = new TH1F(("matchScore_weight" + vertexType).c_str(),"Vertex Match Score (weight)",101,0,1.01);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/matchScore_weight", m_matchScore_weight[vertexType]));
    m_matchScore_pt[vertexType]             = new TH1F(("matchScore_pt" + vertexType).c_str(),"Vertex Match Score (pT)",101,0,1.01);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/matchScore_pt", m_matchScore_pt[vertexType]));
    m_matchedTruthID[vertexType]            = new TH1F(("matchedTruthID" + vertexType).c_str(),"Vertex Truth Match ID",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/matchedTruthID", m_matchedTruthID[vertexType]));

    // tracks
    m_recoTrk_qOverP[vertexType]            = new TH1F(("recoTrk_qOverP" + vertexType).c_str(),"Reco track qOverP ",100,0,.01);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_qOverP", m_recoTrk_qOverP[vertexType]));
    m_recoTrk_theta[vertexType]             = new TH1F(("recoTrk_theta" + vertexType).c_str(),"Reco track theta ",64,0,3.2);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_theta", m_recoTrk_theta[vertexType]));
    m_recoTrk_E[vertexType]                 = new TH1F(("recoTrk_E" + vertexType).c_str(),"Reco track E ",100,0,100); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_E", m_recoTrk_E[vertexType]));
    m_recoTrk_M[vertexType]                 = new TH1F(("recoTrk_M" + vertexType).c_str(),"Reco track M ",100,0,10);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_M", m_recoTrk_M[vertexType]));
    m_recoTrk_Pt[vertexType]                = new TH1F(("recoTrk_Pt" + vertexType).c_str(),"Reco track Pt ",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Pt", m_recoTrk_Pt[vertexType]));
    m_recoTrk_Px[vertexType]                = new TH1F(("recoTrk_Px" + vertexType).c_str(),"Reco track Px ",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Px", m_recoTrk_Px[vertexType]));
    m_recoTrk_Py[vertexType]                = new TH1F(("recoTrk_Py" + vertexType).c_str(),"Reco track Py ",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Py", m_recoTrk_Py[vertexType]));
    m_recoTrk_Pz[vertexType]                = new TH1F(("recoTrk_Pz" + vertexType).c_str(),"Reco track Pz ",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Pz", m_recoTrk_Pz[vertexType]));
    m_recoTrk_Eta[vertexType]               = new TH1F(("recoTrk_Eta" + vertexType).c_str(),"Reco track Eta ",100,-5,5);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Eta", m_recoTrk_Eta[vertexType]));
    m_recoTrk_Phi[vertexType]               = new TH1F(("recoTrk_Phi" + vertexType).c_str(),"Reco track Phi ",63,-3.2,3.2); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Phi", m_recoTrk_Phi[vertexType]));
    m_recoTrk_D0[vertexType]                = new TH1F(("recoTrk_D0" + vertexType).c_str(),"Reco track D0 ",300,-300,300); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_D0", m_recoTrk_D0[vertexType]));
    m_recoTrk_Z0[vertexType]                = new TH1F(("recoTrk_Z0" + vertexType).c_str(),"Reco track Z0 ",500,-500,500); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Z0", m_recoTrk_Z0[vertexType]));
    m_recoTrk_errD0[vertexType]             = new TH1F(("recoTrk_errD0" + vertexType).c_str(),"Reco track errD0 ",300,0,30);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_errD0", m_recoTrk_errD0[vertexType]));
    m_recoTrk_errZ0[vertexType]             = new TH1F(("recoTrk_errZ0" + vertexType).c_str(),"Reco track errZ0 ",500,0,50);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_errZ0", m_recoTrk_errZ0[vertexType]));
    m_recoTrk_Chi2[vertexType]              = new TH1F(("recoTrk_Chi2" + vertexType).c_str(),"Reco track Chi2 ",100,0,10); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_Chi2", m_recoTrk_Chi2[vertexType]));
    m_recoTrk_nDoF[vertexType]              = new TH1F(("recoTrk_nDoF" + vertexType).c_str(),"Reco track nDoF ",100,0,100);  
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_nDoF", m_recoTrk_nDoF[vertexType]));
    m_recoTrk_charge[vertexType]            = new TH1F(("recoTrk_charge" + vertexType).c_str(),"Reco track charge ",3,-1.5,1.5); 
    ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/SecondaryVertex/" + vertexType + "/Trk_charge", m_recoTrk_charge[vertexType]));

  }

  ////////////////////////////////////////////
  //////// Truth Vertex Histograms ///////////
  ////////////////////////////////////////////
  m_truthX                    = new TH1F("truth_X","truth vertex x [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthX", m_truthX));
  m_truthY                    = new TH1F("truth_Y","truth vertex y [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthY", m_truthY));
  m_truthZ                    = new TH1F("truth_Z","truth vertex z [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthZ", m_truthZ));
  m_truthR                    = new TH1F("truth_R","truth vertex r [mm]",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthR", m_truthR));
  m_truthdistFromPV           = new TH1F("truth_distFromPV","truth vertex distFromPV [mm]",500,0,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthdistFromPV", m_truthdistFromPV));
  m_truthEta                  = new TH1F("truth_Eta","truth veEtatex Eta ",100,-5,5);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthEta", m_truthEta));
  m_truthPhi                  = new TH1F("truth_Phi","truth vePhitex Phi ",64,-3.2,3.2); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthPhi", m_truthPhi));
  m_truthNtrk_out             = new TH1F("truth_Ntrk_out","truth vertex n outgoing tracks",100,0,100);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthNtrk_out", m_truthNtrk_out));
  m_truthParent_E             = new TH1F("truth_Parent_E","Reco track E ",100,0,100); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_E", m_truthParent_E));    
  m_truthParent_M             = new TH1F("truth_Parent_M","Reco track M ",500,0,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_M", m_truthParent_M));
  m_truthParent_Pt            = new TH1F("truth_Parent_Pt","Reco track Pt ",100,0,100);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_Pt", m_truthParent_Pt));
  m_truthParent_Eta           = new TH1F("truth_Parent_Eta","Reco track Eta ",100,-5,5);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_Eta", m_truthParent_Eta));
  m_truthParent_Phi           = new TH1F("truth_Parent_Phi","Reco track Phi ",63,-3.2,3.2); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_Phi", m_truthParent_Phi));
  m_truthParent_charge        = new TH1F("truth_Parent_charge","Reco track charge ",3,-1,1); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParent_charge", m_truthParent_charge));
  m_truthParentProdX          = new TH1F("truth_ParentProdX","truthParentProd vertex x [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdX", m_truthParentProdX));
  m_truthParentProdY          = new TH1F("truth_ParentProdY","truthParentProd vertex y [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdY", m_truthParentProdY));
  m_truthParentProdZ          = new TH1F("truth_ParentProdZ","truthParentProd vertex z [mm]",500,-500,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdZ", m_truthParentProdZ));
  m_truthParentProdR          = new TH1F("truth_ParentProdR","truthParentProd vertex r [mm]",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdR", m_truthParentProdR));
  m_truthParentProddistFromPV = new TH1F("truth_ParentProddistFromPV","truthParentProd vertex distFromPV [mm]",500,0,500);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProddistFromPV", m_truthParentProddistFromPV));
  m_truthParentProdEta        = new TH1F("truth_ParentProdEta","truthParentProd veEtatex Eta ",100,-5,5);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdEta", m_truthParentProdEta));
  m_truthParentProdPhi        = new TH1F("truth_ParentProdPhi","truthParentProd vePhitex Phi ",64,-3.2,3.2); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/truthParentProdPhi", m_truthParentProdPhi));

  m_truthInclusive_r            = new TH1F("truth_R_Inclusive","Reconstructable Truth Vertices",6000,0,600); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Inclusive/R", m_truthInclusive_r));
  m_truthReconstructable_r      = new TH1F("truth_R_Reconstructable","Truth Vertices in detector acceptance",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Reconstructable/R", m_truthReconstructable_r));
  m_truthAccepted_r             = new TH1F("truth_R_Accepted","Truth Vertices in detector acceptance",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Accepted/R", m_truthAccepted_r));
  m_truthSeeded_r               = new TH1F("truth_R_Seeded","Seedable Truth Vertices",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Seeded/R", m_truthSeeded_r));
  m_truthReconstructed_r        = new TH1F("truth_R_Reconstructed","Vertex with Match Score > 0.5",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Reconstructed/R", m_truthReconstructed_r));
  m_truthSplit_r                = new TH1F("truth_R_Split","Split Vertex",6000,0,600);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Split/R", m_truthSplit_r));

  // TODO: implement these plots
  m_truth_Ntrk                  = new TH1F("truth_Ntrk","Truth vertex n track pass tracks",30,0,30);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Inclusive/Ntrk", m_truth_Ntrk));
  m_truthReconstructable_trkSel = new TH1F("truth_Ntrk_Seeded","Seedable Truth Vertices",30,0,30); 
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Reconstructable/Ntrk", m_truthReconstructable_trkSel));
  m_truthReconstructed_trkSel   = new TH1F("truth_Ntrk_Reconstructed","Vertex with Match Score > 0.5",30,0,30);  
  ATH_CHECK( m_thistSvc->regHist("/VTXPLOTS/" + name() + "/TruthVertex/Reconstructed/Ntrk", m_truthReconstructed_trkSel));
  return StatusCode::SUCCESS;
}

StatusCode InDetSecVertexTruthMatchTool::finalize()
{

  ATH_MSG_INFO("Finalizing InDetSecVertexTruthMatchTool");
  return StatusCode::SUCCESS;
}

namespace {
//Helper methods for this file only

//In the vector of match info, find the element corresponding to link and return its index; create a new one if necessary
size_t indexOfMatchInfo( std::vector<VertexTruthMatchInfo> & matches, const ElementLink<xAOD::TruthVertexContainer> & link ) {
  for ( size_t i = 0; i < matches.size(); ++i ) {
    if ( link.key() == std::get<0>(matches[i]).key() && link.index() == std::get<0>(matches[i]).index() )
      return i;
  }
  // This is the first time we've seen this truth vertex, so make a new entry
  matches.emplace_back( link, 0., 0. );
  return matches.size() - 1;
}

}

StatusCode InDetSecVertexTruthMatchTool::matchVertices( const xAOD::VertexContainer & vtxContainer,
                                                        const xAOD::TruthVertexContainer & truthVtxContainer) {

  ATH_MSG_DEBUG("Start vertex matching");

  //setup decorators for truth matching info
  static const xAOD::Vertex::Decorator<std::vector<VertexTruthMatchInfo> > matchInfoDecor("TruthVertexMatchingInfos");
  static const xAOD::Vertex::Decorator<VertexMatchType> matchTypeDecor("VertexMatchType");
  static const xAOD::Vertex::Decorator<std::vector<ElementLink<xAOD::VertexContainer> > > splitPartnerDecor("SplitPartners");
  static const xAOD::Vertex::Decorator<ElementLink<xAOD::TruthVertexContainer> > backLinkDecor("RecoToTruthLink");

  const xAOD::Vertex::Decorator<float> fakeScoreDecor("FakeScore");
  const xAOD::Vertex::Decorator<float> otherScoreDecor("OtherScore");

  //setup accessors
  // can switch to built in method in xAOD::Vertex once don't have to deal with changing names anymore
  xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
  xAOD::Vertex::ConstAccessor<std::vector<float> > weightAcc("trackWeights");

  xAOD::TrackParticle::ConstAccessor<ElementLink<xAOD::TruthParticleContainer> > trk_truthPartAcc("truthParticleLink");
  xAOD::TrackParticle::ConstAccessor<float> trk_truthProbAcc("truthMatchProbability");

  //some variables to store
  size_t ntracks;
  xAOD::VxType::VertexType vtxType;

  ATH_MSG_DEBUG("Starting Loop on Vertices");

  //=============================================================================
  //First loop over vertices: get tracks, then TruthParticles, and store relative
  //weights from each TruthVertex
  //=============================================================================
  size_t vtxEntry = 0;

  for ( const xAOD::Vertex* vtx : vtxContainer ) {

    vtxEntry++;
    ATH_MSG_DEBUG("Matching vertex number: " << vtxEntry << ".");

    vtxType = static_cast<xAOD::VxType::VertexType>( vtx->vertexType() );

    if(vtxType != xAOD::VxType::SecVtx ){
      ATH_MSG_DEBUG("Vertex not labeled as secondary");
      continue;
    }

    //create the vector we will add as matching info decoration later
    std::vector<VertexTruthMatchInfo> matchinfo;

    const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( *vtx );
    ntracks = trkParts.size();
    const std::vector<float> & trkWeights = weightAcc( *vtx );

    //if don't have track particles
    if (!trkAcc.isAvailable(*vtx) || !weightAcc.isAvailable(*vtx) ) {
      ATH_MSG_WARNING("trackParticles or trackWeights not available, vertex is missing info");
      continue;
    }
    if ( trkWeights.size() != ntracks ) {
      ATH_MSG_WARNING("Vertex without same number of tracks and trackWeights, vertex is missing info");
      continue;
    }

    ATH_MSG_DEBUG("Matching new vertex at (" << vtx->x() << ", " << vtx->y() << ", " << vtx->z() << ")" << " with " << ntracks << " tracks, at index: " << vtx->index());

    float totalWeight = 0.;
    float totalPt = 0; 
    float otherPt = 0;
    float fakePt = 0;

    //loop over the tracks in the vertex
    for ( size_t t = 0; t < ntracks; ++t ) {

      ATH_MSG_DEBUG("Checking track number " << t);

      if (!trkParts[t].isValid()) {
         ATH_MSG_DEBUG("Track " << t << " is bad!");
         continue;
      }
      const xAOD::TrackParticle & trk = **trkParts[t];

      // store the contribution to total weight and pT
      totalWeight += trkWeights[t];
      totalPt += trk.pt();

      // get the linked truth particle
      if (!trk_truthPartAcc.isAvailable(trk)) {
        ATH_MSG_DEBUG("The truth particle link decoration isn't available.");
        continue;
      }  
      const ElementLink<xAOD::TruthParticleContainer> & truthPartLink = trk_truthPartAcc( trk );
      float prob = trk_truthProbAcc( trk );
      ATH_MSG_DEBUG("Truth prob: " << prob);

      // check the truth particle origin
      if (truthPartLink.isValid()  && prob > m_trkMatchProb) {
        const xAOD::TruthParticle & truthPart = **truthPartLink;

        int barcode =  checkProduction(truthPart);

        //check if the truth particle is "good"
        if ( barcode != -1 ) {
          //track in vertex is linked to LLP descendant
          //create link to truth vertex and add to matchInfo
          auto it = std::find_if(truthVtxContainer.begin(), truthVtxContainer.end(), 
            [&](const auto& ele){ return ele->barcode() == barcode;} );

          if(it == truthVtxContainer.end()) {
            ATH_MSG_DEBUG("Truth vertex with barcode " << barcode << " not found!");
          }

          // get index for the elementLink
          size_t index = it - truthVtxContainer.begin();

          const ElementLink<xAOD::TruthVertexContainer> elLink = ElementLink<xAOD::TruthVertexContainer>( truthVtxContainer, index );

          size_t matchIdx = indexOfMatchInfo( matchinfo, elLink );

          std::get<1>(matchinfo[matchIdx]) += trkWeights[t];
          std::get<2>(matchinfo[matchIdx]) += trk.pt();
        } else {
          //truth particle failed cuts
          ATH_MSG_DEBUG("Truth particle not from LLP decay.");
          otherPt += trk.pt();
        }
      } else {
        //not valid or low matching probability
        ATH_MSG_DEBUG("Invalid or low prob truth link!");
        fakePt += trk.pt();
      }
    }//end loop over tracks in vertex

    

    // normalize by total weight and pT
    std::for_each( matchinfo.begin(), matchinfo.end(), [&](VertexTruthMatchInfo& link)
    {
      std::get<1>(link) /= totalWeight;
      std::get<2>(link) /= totalPt;
    });

    float fakeScore = fakePt/totalPt;
    float otherScore = otherPt/totalPt;

    matchInfoDecor ( *vtx ) = matchinfo;
    fakeScoreDecor ( *vtx ) = fakeScore;
    otherScoreDecor( *vtx ) = otherScore;
  }

  //After first loop, all vertices have been decorated with their vector of match info (link to TruthVertex paired with weight)
  //now we want to use that information from the whole collection to assign types
  //keep track of whether a type is assigned
  //useful since looking for splits involves a double loop, and then setting types ahead in the collection
  std::vector<bool> assignedType( vtxContainer.size(), false );
  static const xAOD::TruthVertex::Decorator<bool> isMatched("VertexMatchedToTruth");
  static const xAOD::TruthVertex::Decorator<bool> isSplit("VertexSplit");

  for ( size_t i = 0; i < vtxContainer.size(); ++i ) {

    if ( assignedType[i] ) {
      ATH_MSG_DEBUG("Vertex already defined as split.");
      if(m_fillHist) {
        ATH_CHECK( fillRecoPlots( *vtxContainer[i] ) );
      }
      continue; // make sure we don't reclassify vertices already found in the split loop below
    }

    std::vector<VertexTruthMatchInfo> & info = matchInfoDecor( *vtxContainer[i] );
    float fakeScore  = fakeScoreDecor( *vtxContainer[i] );

    if(fakeScore > m_vxMatchWeight) {
      ATH_MSG_DEBUG("Vertex is fake.");
      matchTypeDecor( *vtxContainer[i] ) = FAKE;
    } else if (info.size() == 1) {
      if(std::get<2>(info[0]) > m_vxMatchWeight ) { // one truth matched vertex, sufficient weight
        ATH_MSG_DEBUG("One true decay vertices matched with sufficient weight. Vertex is matched.");
        matchTypeDecor( *vtxContainer[i] ) = MATCHED;
        isMatched(**std::get<0>(info[0])) = true;
      }
      else {
        ATH_MSG_DEBUG("One true decay vertices matched with insufficient weight. Vertex is other.");
        matchTypeDecor( *vtxContainer[i] ) = OTHER;
      }
    } else if (info.size() >= 2 ) {                        // more than one true deacy vertices matched
      ATH_MSG_DEBUG("Multiple true decay vertices matched. Vertex is merged.");
      matchTypeDecor( *vtxContainer[i] ) = MERGED;
      std::for_each( info.begin(), info.end(), [](VertexTruthMatchInfo& link)
      {
        isMatched(**std::get<0>(link)) = true;
      });
    } else {                                // zero truth matched vertices, but not fake 
      ATH_MSG_DEBUG("Vertex is neither fake nor LLP. Marking as OTHER.");
      matchTypeDecor( *vtxContainer[i] ) = OTHER;
    }

    //check for splitting
    // TODO: decorate linked truth vertices with isSplit
    if ( matchTypeDecor( *vtxContainer[i] ) == MATCHED || matchTypeDecor( *vtxContainer[i] ) == MERGED ) {
      std::vector<size_t> foundSplits;
      for ( size_t j = i + 1; j < vtxContainer.size(); ++j ) {
        std::vector<VertexTruthMatchInfo> & info2 = matchInfoDecor( *vtxContainer[j] );
        //check second vertex is not dummy or fake, and that it has same elementlink as first vertex
        //equality test is in code but doesnt seem to work for ElementLinks that I have?
        //so i am just checking that the contianer key hash and the index are the same
        if (matchTypeDecor( *vtxContainer[j] ) == FAKE) continue;
        if (!info2.empty() && std::get<0>(info2[0]).isValid() && std::get<0>(info[0]).key() == std::get<0>(info2[0]).key() && std::get<0>(info[0]).index() == std::get<0>(info2[0]).index() ) {
          //add split links; first between first one found and newest one
          splitPartnerDecor( *vtxContainer[i] ).emplace_back( vtxContainer, j );
          splitPartnerDecor( *vtxContainer[j] ).emplace_back( vtxContainer, i );
          //then between any others we found along the way
          for ( auto k : foundSplits ) { //k is a size_t in the vector of splits
            splitPartnerDecor( *vtxContainer[k] ).emplace_back( vtxContainer, j );
            splitPartnerDecor( *vtxContainer[j] ).emplace_back( vtxContainer, k );
          }
          //then keep track that we found this one
          foundSplits.push_back(j);
          matchTypeDecor( *vtxContainer[i] ) = SPLIT;
          matchTypeDecor( *vtxContainer[j] ) = SPLIT;
          isSplit(**std::get<0>(info[0])) = true;
          assignedType[j] = true;
        } //if the two vertices match to same TruthVertex
      }//inner loop over vertices
    } //if matched or merged

    if(m_fillHist) {
      ATH_CHECK( fillRecoPlots( *vtxContainer[i] ) );
    }
  } //outer loop

  return StatusCode::SUCCESS;

}

StatusCode InDetSecVertexTruthMatchTool::labelTruthVertices( const xAOD::TruthVertexContainer & truthVtxContainer) {
  
  ATH_MSG_DEBUG("Labeling truth vertices");

  const xAOD::TrackParticleContainer * trackPartCont = nullptr;
  const xAOD::TrackParticleContainer * largeD0TrackPartCont = nullptr;

  ATH_CHECK( evtStore()->retrieve( trackPartCont, "InDetTrackParticles" ) );
  if ( evtStore()->contains<xAOD::TrackParticleContainer>( "InDetLargeD0TrackParticles" ) )
    ATH_CHECK( evtStore()->retrieve( largeD0TrackPartCont, "InDetLargeD0TrackParticles" ) );
  else
    ATH_MSG_WARNING("No LRT container in input! Using standard tracks only.");


  static const xAOD::Vertex::Decorator<TruthVertexMatchType> matchTypeDecor("TruthVertexMatchType");
  xAOD::TruthVertex::Decorator<bool> isMatched("VertexMatchedToTruth");
  xAOD::TruthVertex::Decorator<bool> isSplit("VertexSplit");

  for(const xAOD::TruthVertex* truthVtx : truthVtxContainer) {

    if(truthVtx->nIncomingParticles() != 1){continue;}
    const xAOD::TruthParticle* truthPart = truthVtx->incomingParticle(0);
    if(not truthPart) continue;
    if(std::find(m_pdgIdList.begin(), m_pdgIdList.end(), std::abs(truthPart->pdgId())) == m_pdgIdList.end()) continue;
    if(truthVtx->nOutgoingParticles()<2){continue;} //Skipping vertices with only 1 outgoing particle.
    ATH_MSG_DEBUG("Analysing Truth Vertex " << truthVtx );
    std::vector<const xAOD::TruthParticle*> reconstructibleParticles;
    int counter = 0;
    countReconstructibleDescendentParticles( *truthVtx, reconstructibleParticles, counter );

    // temporary solution for keeping track of particles in the vertex
    std::vector<int> particleInfo = {0,0,0};
    std::vector<int> vertexInfo = {0,0,0};

    for(size_t n = 0; n < reconstructibleParticles.size(); n++){
      ATH_MSG_DEBUG("Checking daughter no. " << n);
      const xAOD::TruthParticle* outPart = reconstructibleParticles.at(n);
      
      if (trackPartCont){
        particleInfo = checkParticle(*outPart, *trackPartCont);
        ATH_MSG_DEBUG(particleInfo);
      
        for(size_t h = 0; h < particleInfo.size(); h++){
          vertexInfo.at(h) += particleInfo.at(h);
        }
      }
      if (largeD0TrackPartCont){
        particleInfo = checkParticle(*outPart, *largeD0TrackPartCont);
        ATH_MSG_DEBUG(particleInfo);
        
        // skip first value in the tuple, we already counted it
        // in the first loop
        for(size_t h = 1; h < particleInfo.size(); h++){
          vertexInfo.at(h) += particleInfo.at(h);
        }
      }
    }
      
    ATH_MSG_DEBUG("Info for this LLP decay: " << vertexInfo);

    matchTypeDecor(*truthVtx) = INCLUSIVE;
    if( vertexInfo.at(0) > 1 &&  truthVtx->perp() <  320 && abs(truthVtx->z()) < 1500){
      ATH_MSG_DEBUG("Vertex is reconstructable and in Inner Det region");
      matchTypeDecor(*truthVtx) = RECONSTRUCTABLE;
    }
    if( matchTypeDecor(*truthVtx) == RECONSTRUCTABLE and vertexInfo.at(1) > 1){
      ATH_MSG_DEBUG("Vertex has at least two tracks");
      matchTypeDecor(*truthVtx) = ACCEPTED;
    }
    if(matchTypeDecor(*truthVtx) == ACCEPTED and vertexInfo.at(2) > 1){
      ATH_MSG_DEBUG("Vertex is has at least two tracks passing track selection: " << vertexInfo.at(2));
      matchTypeDecor(*truthVtx) = SEEDED;
    }
    if(matchTypeDecor(*truthVtx) == SEEDED and isMatched(*truthVtx)){
      ATH_MSG_DEBUG("Vertex is matched to a reconstructed secVtx");
      matchTypeDecor(*truthVtx) = RECONSTRUCTED;
    }
    if(matchTypeDecor(*truthVtx) == SEEDED and isSplit(*truthVtx)){
      ATH_MSG_DEBUG("Vertex is matched to multiple secVtx");
      matchTypeDecor(*truthVtx) = RECONSTRUCTEDSPLIT;
    }

    if(m_fillHist) {
      ATH_CHECK( fillTruthPlots(*truthVtx) );
    }

  }
  ATH_MSG_DEBUG("Done labeling truth vertices");

  return StatusCode::SUCCESS;
}

std::vector<int> InDetSecVertexTruthMatchTool::checkParticle(const xAOD::TruthParticle &truthPart, const xAOD::TrackParticleContainer &trkCont) const {

  xAOD::TrackParticle::ConstAccessor<char>  trackPass("is_selected"+m_AugString);
  xAOD::TrackParticle::ConstAccessor<ElementLink<xAOD::TruthParticleContainer> > trk_truthPartAcc("truthParticleLink");
  xAOD::TrackParticle::ConstAccessor<float> trk_truthProbAcc("truthMatchProbability");

  if(truthPart.pt() < m_trkPtCut){
    ATH_MSG_DEBUG("Insufficient pt to reconstruct the particle");
    return {0,0,0};
  }
  else{

    for(const xAOD::TrackParticle* trkPart : trkCont){
      const ElementLink<xAOD::TruthParticleContainer> & truthPartLink = trk_truthPartAcc( *trkPart );
      float matchProb = trk_truthProbAcc( *trkPart );

      if(truthPartLink.isValid() && matchProb > m_trkMatchProb) {
        const xAOD::TruthParticle& tmpPart = **truthPartLink;
        if(tmpPart.barcode() == truthPart.barcode()) {
          if(trackPass.isAvailable( *trkPart ) and trackPass( *trkPart )) {
            ATH_MSG_DEBUG("Particle has a track that passes track selection.");
            return {1,1,1};
          }
          else {
            ATH_MSG_DEBUG("Particle has a track, but did not pass track selection.");
            return {1,1,0};
          }
        }
      }
    }
    ATH_MSG_DEBUG("Particle has enough pt.");
    return {1,0,0};
    
  }
  return {0,0,0};
}

StatusCode InDetSecVertexTruthMatchTool::fillRecoPlots( const xAOD::Vertex& secVtx ) {
  
  // set of accessors for tracks and weights
  xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
  xAOD::Vertex::ConstAccessor<std::vector<float> > weightAcc("trackWeights");
  
  // set of decorators for truth matching info
  const xAOD::Vertex::Decorator<VertexMatchType> matchTypeDecor("VertexMatchType");
  const xAOD::Vertex::Decorator<std::vector<VertexTruthMatchInfo> > matchInfoDecor("TruthVertexMatchingInfos");

  std::string vertexType = "";
  switch (matchTypeDecor(secVtx))
  {
  case MATCHED:
    vertexType = "Matched";
    break;
  case MERGED:
    vertexType = "Merged";
    break;
  case SPLIT:
    vertexType = "Split";
    break;
  case OTHER:
    vertexType = "Other";
    break;
  case FAKE:
    vertexType = "Fake";
    break;
  default:
    break;
  }

  m_matchType->Fill(matchTypeDecor(secVtx));

  TVector3 reco_pos(secVtx.x(), secVtx.y(), secVtx.z());
  float reco_r = reco_pos.Perp();


  size_t ntracks;
  const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( secVtx );
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
      m_recoTrk_Chi2[vertexType]->Fill( Trk_Chi2(trk) / Trk_nDoF(trk));
      m_recoTrk_nDoF[vertexType]->Fill( Trk_nDoF(trk) ); 
    }
    m_recoTrk_charge[vertexType]->Fill(trk.charge());
    m_recoTrk_errD0[vertexType]->Fill(trk.definingParametersCovMatrix()(0,0));
    m_recoTrk_errZ0[vertexType]->Fill(trk.definingParametersCovMatrix()(1,1));

    m_recoTrk_theta[vertexType]->Fill(trk.definingParameters()[3]);
    m_recoTrk_qOverP[vertexType]->Fill(trk.definingParameters()[4]);
    m_recoTrk_E[vertexType]->Fill(trk.e()/GeV);
    m_recoTrk_M[vertexType]->Fill(trk.m()/GeV);
    m_recoTrk_Pt[vertexType]->Fill(trk.pt()/GeV);

    m_recoTrk_Px[vertexType]->Fill(trk.p4().Px()/GeV);
    m_recoTrk_Py[vertexType]->Fill(trk.p4().Py()/GeV);
    m_recoTrk_Pz[vertexType]->Fill(trk.p4().Pz()/GeV);

    m_recoTrk_Eta[vertexType]->Fill(trk.eta());
    m_recoTrk_Phi[vertexType]->Fill(trk.phi0());

    m_recoTrk_D0[vertexType]->Fill(trk_d0);
    m_recoTrk_Z0[vertexType]->Fill(trk_z0);
  } // end loop over tracks

  const double dir  = sumP4.Vect().Dot( reco_pos ) / sumP4.Vect().Mag() / reco_pos.Mag();

  xAOD::Vertex::ConstAccessor<float> Chi2("chiSquared");
  xAOD::Vertex::ConstAccessor<float> nDoF("numberDoF");
  m_recoX[vertexType]->Fill(secVtx.x());
  m_recoY[vertexType]->Fill(secVtx.y());
  m_recoZ[vertexType]->Fill(secVtx.z());
  m_recoR[vertexType]->Fill(reco_r);
  m_recoNtrk[vertexType]->Fill(ntracks);
  m_recoPt[vertexType]->Fill(sumP4.Pt() / GeV);
  m_recoEta[vertexType]->Fill(sumP4.Eta());
  m_recoPhi[vertexType]->Fill(sumP4.Phi());
  m_recoMass[vertexType]->Fill(sumP4.M() / GeV);
  m_recoMu[vertexType]->Fill(sumP4.M()/maxDR / GeV);
  m_recoChi2[vertexType]->Fill(Chi2(secVtx)/nDoF(secVtx));
  m_recoDir[vertexType]->Fill(dir);
  m_recoCharge[vertexType]->Fill(charge);
  m_recoH[vertexType]->Fill(H / GeV);
  m_recoHt[vertexType]->Fill(HT / GeV);
  m_recoMinOpAng[vertexType]->Fill(minOpAng);
  m_recoMaxOpAng[vertexType]->Fill(maxOpAng); 
  m_recoMinD0[vertexType]->Fill(minD0);
  m_recoMaxD0[vertexType]->Fill(maxD0); 
  m_recoMaxDR[vertexType]->Fill(maxDR); 

  m_recoX["All"]->Fill(secVtx.x());
  m_recoY["All"]->Fill(secVtx.y());
  m_recoZ["All"]->Fill(secVtx.z());
  m_recoR["All"]->Fill(reco_r);
  m_recoNtrk["All"]->Fill(ntracks);
  m_recoPt["All"]->Fill(sumP4.Pt() / GeV);
  m_recoEta["All"]->Fill(sumP4.Eta());
  m_recoPhi["All"]->Fill(sumP4.Phi());
  m_recoMass["All"]->Fill(sumP4.M() / GeV);
  m_recoMu["All"]->Fill(sumP4.M()/maxDR / GeV);
  m_recoChi2["All"]->Fill(Chi2(secVtx)/nDoF(secVtx));
  m_recoDir["All"]->Fill(dir);
  m_recoCharge["All"]->Fill(charge);
  m_recoH["All"]->Fill(H / GeV);
  m_recoHt["All"]->Fill(HT / GeV);
  m_recoMinOpAng["All"]->Fill(minOpAng);
  m_recoMaxOpAng["All"]->Fill(maxOpAng); 
  m_recoMinD0["All"]->Fill(minD0);
  m_recoMaxD0["All"]->Fill(maxD0); 
  m_recoMaxDR["All"]->Fill(maxDR); 

  std::vector<VertexTruthMatchInfo> truthmatchinfo;
  truthmatchinfo = matchInfoDecor(secVtx);

  // This includes all matched vertices, including splits
  if(not truthmatchinfo.empty()){
    float matchScore_weight = std::get<1>(truthmatchinfo.at(0));
    float matchScore_pt     = std::get<2>(truthmatchinfo.at(0));

    m_matchScore_weight[vertexType]->Fill(matchScore_weight);
    m_matchScore_pt[vertexType]->Fill(matchScore_pt);

    ATH_MSG_DEBUG("Match Score and probability: " << matchScore_weight << " " << matchScore_pt/0.01);

    const ElementLink<xAOD::TruthVertexContainer>& truthVertexLink = std::get<0>(truthmatchinfo.at(0));
    const xAOD::TruthVertex& truthVtx = **truthVertexLink ;

    m_positionRes_R[vertexType]->Fill(reco_r - truthVtx.perp());
    m_positionRes_Z[vertexType]->Fill(secVtx.z() - truthVtx.z());

    m_recoX["LLP"]->Fill(secVtx.x());
    m_recoY["LLP"]->Fill(secVtx.y());
    m_recoZ["LLP"]->Fill(secVtx.z());
    m_recoR["LLP"]->Fill(reco_r);
    m_recoNtrk["LLP"]->Fill(ntracks);
    m_recoPt["LLP"]->Fill(sumP4.Pt() / GeV);
    m_recoEta["LLP"]->Fill(sumP4.Eta());
    m_recoPhi["LLP"]->Fill(sumP4.Phi());
    m_recoMass["LLP"]->Fill(sumP4.M() / GeV);
    m_recoMu["LLP"]->Fill(sumP4.M()/maxDR / GeV);
    m_recoChi2["LLP"]->Fill(Chi2(secVtx)/nDoF(secVtx));
    m_recoDir["LLP"]->Fill(dir);
    m_recoCharge["LLP"]->Fill(charge);
    m_recoH["LLP"]->Fill(H / GeV);
    m_recoHt["LLP"]->Fill(HT / GeV);
    m_recoMinOpAng["LLP"]->Fill(minOpAng);
    m_recoMaxOpAng["LLP"]->Fill(maxOpAng); 
    m_recoMinD0["LLP"]->Fill(minD0);
    m_recoMaxD0["LLP"]->Fill(maxD0); 
    m_recoMaxDR["LLP"]->Fill(maxDR); 

    m_positionRes_R["LLP"]->Fill(reco_r - truthVtx.perp());
    m_positionRes_Z["LLP"]->Fill(secVtx.z() - truthVtx.z());
    m_matchScore_weight["LLP"]->Fill(matchScore_weight);
    m_matchScore_pt["LLP"]->Fill(matchScore_pt);
  }
  
  return StatusCode::SUCCESS;
}

StatusCode InDetSecVertexTruthMatchTool::fillTruthPlots( const xAOD::TruthVertex& truthVtx) {
  
  ATH_MSG_DEBUG("Plotting truth vertex");

  m_truthX->Fill(truthVtx.x());
  m_truthY->Fill(truthVtx.y());
  m_truthZ->Fill(truthVtx.z());
  m_truthR->Fill(truthVtx.perp());
  m_truthEta->Fill(truthVtx.eta());
  m_truthPhi->Fill(truthVtx.phi());
  m_truthNtrk_out->Fill(truthVtx.nOutgoingParticles());

  ATH_MSG_DEBUG("Plotting truth parent");
  const xAOD::TruthParticle& truthPart = *truthVtx.incomingParticle(0);

  m_truthParent_E->Fill(truthPart.e() / GeV);
  m_truthParent_M->Fill(truthPart.m() / GeV);
  m_truthParent_Pt->Fill(truthPart.pt() / GeV);
  m_truthParent_Phi->Fill(truthPart.phi());
  m_truthParent_Eta->Fill(truthPart.eta());
  m_truthParent_charge->Fill(truthPart.charge());

  ATH_MSG_DEBUG("Plotting truth prod vtx");
  if(truthPart.hasProdVtx()){
    const xAOD::TruthVertex & vertex = *truthPart.prodVtx();

    m_truthParentProdX->Fill(vertex.x());
    m_truthParentProdY->Fill(vertex.y());
    m_truthParentProdZ->Fill(vertex.z());
    m_truthParentProdR->Fill(vertex.perp());
    m_truthParentProdEta->Fill(vertex.eta());
    m_truthParentProdPhi->Fill(vertex.phi());
  }

  ATH_MSG_DEBUG("Plotting match types");
  static const xAOD::Vertex::Decorator<TruthVertexMatchType> matchTypeDecor("TruthVertexMatchType");
  
  m_truthInclusive_r->Fill(truthVtx.perp());

  if(matchTypeDecor(truthVtx) >= RECONSTRUCTABLE){
    m_truthReconstructable_r->Fill(truthVtx.perp());
  }
  if(matchTypeDecor(truthVtx) >= ACCEPTED){
    m_truthAccepted_r->Fill(truthVtx.perp());
  }
  if(matchTypeDecor(truthVtx) >= SEEDED){
    m_truthSeeded_r->Fill(truthVtx.perp());
  }
  if(matchTypeDecor(truthVtx) >= RECONSTRUCTED){
    m_truthReconstructed_r->Fill(truthVtx.perp());
  }
  if(matchTypeDecor(truthVtx) >= RECONSTRUCTEDSPLIT){
    m_truthSplit_r->Fill(truthVtx.perp());
  }
  return StatusCode::SUCCESS;
}


// check if truth particle originated from decay of particle in the pdgIdList
int InDetSecVertexTruthMatchTool::checkProduction( const xAOD::TruthParticle & truthPart ) const {

  if (truthPart.nParents() == 0){
    ATH_MSG_DEBUG("Particle has no parents (end of loop)");
    return -1;
  } 
  else{
    const xAOD::TruthParticle * parent = truthPart.parent(0);
    if(not parent) {
      ATH_MSG_DEBUG("Particle parent is null");
      return -1;
    }
    ATH_MSG_DEBUG("Parent ID: " << parent->pdgId());
        
    if(std::find(m_pdgIdList.begin(), m_pdgIdList.end(), std::abs(parent->pdgId())) != m_pdgIdList.end()) {
        ATH_MSG_DEBUG("Found LLP decay.");
        const xAOD::TruthVertex* vertex = parent->decayVtx();
        return vertex->barcode();
    }
    // recurse on parent
    return checkProduction(*parent);
  }
  return -1;
}

void InDetSecVertexTruthMatchTool::countReconstructibleDescendentParticles(const xAOD::TruthVertex& signalTruthVertex,
                                                                           std::vector<const xAOD::TruthParticle*>& set, int counter) const {

  counter++;

  for( size_t itrk = 0; itrk < signalTruthVertex.nOutgoingParticles(); itrk++) {
    const auto* particle = signalTruthVertex.outgoingParticle( itrk );
    if( !particle ) continue;
    // Recursively add descendents
    if( particle->hasDecayVtx() ) {
      
      TVector3 decayPos( particle->decayVtx()->x(), particle->decayVtx()->y(), particle->decayVtx()->z() );
      TVector3 prodPos ( particle->prodVtx()->x(),  particle->prodVtx()->y(),  particle->prodVtx()->z()  );
      
      auto isInside  = []( TVector3& v ) { return ( v.Perp() < 300. && std::abs( v.z() ) < 1500. ); };
      auto isOutside = []( TVector3& v ) { return ( v.Perp() > 563. || std::abs( v.z() ) > 2720. ); };
      
      const auto distance = (decayPos - prodPos).Mag();

      if (counter > 100) {
        ATH_MSG_WARNING("Vetoing particle that may be added recursively infinitely (potential loop in generator record");
        break;
      }
      
      // consider track reconstructible if it travels at least 10mm
      if( distance < 10.0 ) {
        countReconstructibleDescendentParticles( *particle->decayVtx(), set , counter);
      } else if( isInside ( prodPos  )  && isOutside( decayPos )  && particle->isCharged() ) {
        set.push_back( particle );
      } else if( particle->isElectron() || particle->isMuon() ) {
        set.push_back( particle );
      }
    } else {
      if( !(particle->isCharged()) ) continue;
      set.push_back( particle );
    }
  }
  
  }
