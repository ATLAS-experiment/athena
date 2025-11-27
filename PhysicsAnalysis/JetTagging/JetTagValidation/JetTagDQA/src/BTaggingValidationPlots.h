/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGDQA_BTagPLOTS_H
#define JETTAGDQA_BTagPLOTS_H
    
#include "xAODBase/IParticle.h"
#include "TrkValHistUtils/PlotBase.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "xAODBTagging/BTagging.h" //typedef
#include "xAODJet/JetFwd.h" //lightweight typedef
#include "xAODTracking/VertexFwd.h"//lightweight typedef
#include "xAODEventInfo/EventInfo.h" //typedef

#include <string>
#include <vector>
#include <map>

class TH1;

    
namespace JetTagDQA{
 
  class BTaggingValidationPlots : public PlotBase, public AthMessaging {
    public:
      // constructor
      BTaggingValidationPlots(PlotBase* pParent, const std::string& sDir, std::string sParticleType);

      // fill methods
      void fillJetKinVars(const xAOD::Jet* jet, const int& truth_label, const bool& onZprime, const xAOD::EventInfo* event);
      void fillDiscriminantVariables(const xAOD::Jet* jet, const double& jet_Lxy, const int& truth_label, const bool& onZprime, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event);
      void fillDiscriminantVariables_for_largeRjet(const xAOD::Jet* jet, const int& truth_label, const bool& onZprime, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event);
      void fillMultiplicities(const unsigned int& nJets, const unsigned int& nTracks, const int& nPrimVtx, const unsigned int& nTracksPrimVtx, const unsigned int& nJetsWithMuon, const unsigned int& nJetsWithSV, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event);
      void fillPVVariables(const double& PV_x, const double& PV_y, const double& PV_z, const xAOD::EventInfo* event);
      void fillOther(const xAOD::Jet* jet, bool& contains_muon, double& jet_Lxy, const int& truth_label, const xAOD::EventInfo* event); 
      void fillTrackVariables(const xAOD::Jet* jet, const xAOD::Vertex *myVertex, const std::map<const xAOD::TrackParticle*, int> & track_truth_associations, const bool& contains_muon, const int& truth_label, int& num_HF_tracks_in_jet, const xAOD::EventInfo* event); 
      void fillTrackVariables_for_largeRjet(const xAOD::Jet* jet, const xAOD::Vertex *myVertex, const int& truth_label, const xAOD::EventInfo* event); 
      void fillSVVariables(const xAOD::Jet* jet, const std::map<const xAOD::TrackParticle*, int> & track_truth_associations, const bool& contains_muon, const int& truth_label, const int& num_HF_tracks_in_jet, bool& contains_SV, const xAOD::EventInfo* event); 

      void bookNJetsThatPassedWPCutsHistos();
      void initializeNJetsThatPassedWPCutsMap(std::map<std::string, int>& nJetsThatPassedWPCuts);
      void updateNJetsThatPassedWPCutsMap(std::map<std::string, int>& nJetsThatPassedWPCuts, const double& GN2v01, const double& GN3XPV01);
      void fillNJetsThatPassedWPCutsHistos(std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event);

      void setTaggerInfos();    
      void bookEffHistos();

      // Reco only information
      std::string m_sParticleType;

      // multiplicities
      TH1* m_nJets = nullptr;
      TH1* m_nTracks = nullptr;
      TH1* m_nPrimVtx = nullptr;
      TH1* m_nTracksPrimVtx = nullptr;
      TH1* m_nJetsWithMuon = nullptr;
      TH1* m_nJetsWithSV = nullptr;
      TH1* m_fracJetsWithMuon = nullptr;
      TH1* m_fracJetsWithSV = nullptr;

      // PV variables
      TH1* m_PV_x = nullptr;
      TH1* m_PV_y = nullptr;
      TH1* m_PV_z = nullptr;

      // jet kinematic variables
      TH1* m_jet_e = nullptr;
      TH1* m_jet_e_Zprime = nullptr;
      TH1* m_jet_pt = nullptr;
      TH1* m_jet_pt_Zprime = nullptr;
      TH1* m_jet_eta = nullptr;
      TH1* m_jet_phi = nullptr;

      // muon vars
      TH1* m_muon_pT_frac = nullptr;

      // truth info
      TH1* m_truthLabel = nullptr;

      TH1* m_jet_pt_b = nullptr;
      TH1* m_jet_pt_c = nullptr;
      TH1* m_jet_pt_l = nullptr;
      TH1* m_jet_pt_top = nullptr;
      TH1* m_jet_pt_Zprime_b = nullptr;
      TH1* m_jet_pt_Zprime_c = nullptr;
      TH1* m_jet_pt_Zprime_l = nullptr;
      TH1* m_jet_eta_b = nullptr;
      TH1* m_jet_eta_c = nullptr;
      TH1* m_jet_eta_l = nullptr;
      TH1* m_jet_eta_top = nullptr;


      // SV1 related vars
      TH1* m_SV1_numSVs_incl = nullptr;
      TH1* m_SV1_masssvx_incl = nullptr;
      TH1* m_SV1_N2Tpair_incl = nullptr;
      TH1* m_SV1_efracsvx_incl = nullptr;
      TH1* m_SV1_deltaR_incl = nullptr;
      TH1* m_SV1_significance3d_incl = nullptr;
      TH1* m_SV1_energyTrkInJet_incl = nullptr;
      TH1* m_SV1_NGTinSvx_incl = nullptr;
      TH1* m_SV1_Lxy_incl = nullptr;
      TH1* m_SV1_purity_incl = nullptr;
      TH1* m_SV1_numSVs_b = nullptr;
      TH1* m_SV1_masssvx_b = nullptr;
      TH1* m_SV1_N2Tpair_b = nullptr;
      TH1* m_SV1_efracsvx_b = nullptr;
      TH1* m_SV1_deltaR_b = nullptr;
      TH1* m_SV1_significance3d_b = nullptr;
      TH1* m_SV1_energyTrkInJet_b = nullptr;
      TH1* m_SV1_NGTinSvx_b = nullptr;
      TH1* m_SV1_Lxy_b = nullptr;
      TH1* m_SV1_purity_b = nullptr;
      TH1* m_SV1_numSVs_c = nullptr;
      TH1* m_SV1_masssvx_c = nullptr;
      TH1* m_SV1_N2Tpair_c = nullptr;
      TH1* m_SV1_efracsvx_c = nullptr;
      TH1* m_SV1_deltaR_c = nullptr;
      TH1* m_SV1_significance3d_c = nullptr;
      TH1* m_SV1_energyTrkInJet_c = nullptr;
      TH1* m_SV1_NGTinSvx_c = nullptr;
      TH1* m_SV1_Lxy_c = nullptr;
      TH1* m_SV1_purity_c = nullptr;
      TH1* m_SV1_numSVs_l = nullptr;
      TH1* m_SV1_masssvx_l = nullptr;
      TH1* m_SV1_N2Tpair_l = nullptr;
      TH1* m_SV1_efracsvx_l = nullptr;
      TH1* m_SV1_deltaR_l = nullptr;
      TH1* m_SV1_significance3d_l = nullptr;
      TH1* m_SV1_energyTrkInJet_l = nullptr;
      TH1* m_SV1_NGTinSvx_l = nullptr;
      TH1* m_SV1_Lxy_l = nullptr;
      TH1* m_SV1_purity_l = nullptr;
      TH1* m_SV1_numSVs_muon = nullptr;
      TH1* m_SV1_masssvx_muon = nullptr;
      TH1* m_SV1_N2Tpair_muon = nullptr;
      TH1* m_SV1_efracsvx_muon = nullptr;
      TH1* m_SV1_deltaR_muon = nullptr;
      TH1* m_SV1_significance3d_muon = nullptr;
      TH1* m_SV1_energyTrkInJet_muon = nullptr;
      TH1* m_SV1_NGTinSvx_muon = nullptr;
      TH1* m_SV1_Lxy_muon = nullptr;
      TH1* m_SV1_purity_muon = nullptr;

      TH1* m_SV1_fracHFTracksInJet_incl = nullptr;
      TH1* m_SV1_fracHFTracksInJet_b = nullptr;
      TH1* m_SV1_fracHFTracksInJet_c = nullptr;
      TH1* m_SV1_fracHFTracksInJet_l = nullptr;
      TH1* m_SV1_fracHFTracksInJet_muon = nullptr;
      
      TH1* m_SV1_fracTracks_fromB_incl = nullptr;
      TH1* m_SV1_fracTracks_fromB_b = nullptr;
      TH1* m_SV1_fracTracks_fromB_c = nullptr;
      TH1* m_SV1_fracTracks_fromB_l = nullptr;
      TH1* m_SV1_fracTracks_fromB_muon = nullptr;
      TH1* m_SV1_fracTracks_fromC_incl = nullptr;
      TH1* m_SV1_fracTracks_fromC_b = nullptr;
      TH1* m_SV1_fracTracks_fromC_c = nullptr;
      TH1* m_SV1_fracTracks_fromC_l = nullptr;
      TH1* m_SV1_fracTracks_fromC_muon = nullptr;
      TH1* m_SV1_fracTracks_fromFragmentation_incl = nullptr;
      TH1* m_SV1_fracTracks_fromFragmentation_b = nullptr;
      TH1* m_SV1_fracTracks_fromFragmentation_c = nullptr;
      TH1* m_SV1_fracTracks_fromFragmentation_l = nullptr;
      TH1* m_SV1_fracTracks_fromFragmentation_muon = nullptr;
      TH1* m_SV1_fracTracks_fromSecondaries_incl = nullptr;
      TH1* m_SV1_fracTracks_fromSecondaries_b = nullptr;
      TH1* m_SV1_fracTracks_fromSecondaries_c = nullptr;
      TH1* m_SV1_fracTracks_fromSecondaries_l = nullptr;
      TH1* m_SV1_fracTracks_fromSecondaries_muon = nullptr;
      TH1* m_SV1_fracTracks_fromPileup_incl = nullptr;
      TH1* m_SV1_fracTracks_fromPileup_b = nullptr;
      TH1* m_SV1_fracTracks_fromPileup_c = nullptr;
      TH1* m_SV1_fracTracks_fromPileup_l = nullptr;
      TH1* m_SV1_fracTracks_fromPileup_muon = nullptr;
      TH1* m_SV1_fracTracks_fromFake_incl = nullptr;
      TH1* m_SV1_fracTracks_fromFake_b = nullptr;
      TH1* m_SV1_fracTracks_fromFake_c = nullptr;
      TH1* m_SV1_fracTracks_fromFake_l = nullptr;
      TH1* m_SV1_fracTracks_fromFake_muon = nullptr;

      // these are only filled for detail level above 10
      TH1* m_SV1_fracTracks_Secondaries_KshortDecay_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_KshortDecay_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_KshortDecay_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_KshortDecay_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_KshortDecay_muon = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_LambdaDecay_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_LambdaDecay_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_LambdaDecay_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_LambdaDecay_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_LambdaDecay_muon = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_GammaConversion_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_GammaConversion_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_GammaConversion_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_GammaConversion_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_GammaConversion_muon = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherDecay_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherDecay_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherDecay_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherDecay_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherDecay_muon = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_HadronicInteraction_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_HadronicInteraction_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_HadronicInteraction_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_HadronicInteraction_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_HadronicInteraction_muon = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherSecondary_incl = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherSecondary_b = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherSecondary_c = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherSecondary_u = nullptr; 
      TH1* m_SV1_fracTracks_Secondaries_OtherSecondary_muon = nullptr; 
      TH1* m_SV1_fracTracks_OtherOrigin_incl = nullptr; 
      TH1* m_SV1_fracTracks_OtherOrigin_b = nullptr; 
      TH1* m_SV1_fracTracks_OtherOrigin_c = nullptr; 
      TH1* m_SV1_fracTracks_OtherOrigin_u = nullptr; 
      TH1* m_SV1_fracTracks_OtherOrigin_muon = nullptr; 

      // IPs and IP significances
      TH1* m_track_d0_incl = nullptr;
      TH1* m_track_z0_incl = nullptr;
      TH1* m_track_sigd0_incl = nullptr;
      TH1* m_track_sigz0_incl = nullptr;

      TH1* m_track_d0_b = nullptr;
      TH1* m_track_z0_b = nullptr;
      TH1* m_track_sigd0_b = nullptr;
      TH1* m_track_sigz0_b = nullptr;

      TH1* m_track_d0_c = nullptr;
      TH1* m_track_z0_c = nullptr;
      TH1* m_track_sigd0_c = nullptr;
      TH1* m_track_sigz0_c = nullptr;

      TH1* m_track_d0_u = nullptr;
      TH1* m_track_z0_u = nullptr;
      TH1* m_track_sigd0_u = nullptr;
      TH1* m_track_sigz0_u = nullptr;

      TH1* m_track_d0_muon = nullptr;
      TH1* m_track_z0_muon = nullptr;
      TH1* m_track_sigd0_muon = nullptr;
      TH1* m_track_sigz0_muon = nullptr;

      // pT_frac
      TH1* m_track_pT_frac_incl = nullptr;
      TH1* m_track_pT_frac_b = nullptr;
      TH1* m_track_pT_frac_c = nullptr;
      TH1* m_track_pT_frac_u = nullptr;
      TH1* m_track_pT_frac_muon = nullptr;

      // DeltaR_jet_track
      TH1* m_DeltaR_jet_track_incl = nullptr;
      TH1* m_DeltaR_jet_track_b = nullptr;
      TH1* m_DeltaR_jet_track_c = nullptr;
      TH1* m_DeltaR_jet_track_u = nullptr;
      TH1* m_DeltaR_jet_track_muon = nullptr;

      // tracker hits
      TH1* m_nInnHits_incl = nullptr;
      TH1* m_nNextToInnHits_incl = nullptr;
      TH1* m_nBLHits_incl = nullptr;
      TH1* m_nsharedBLHits_incl = nullptr;
      TH1* m_nsplitBLHits_incl = nullptr;
      TH1* m_nPixHits_incl = nullptr;
      TH1* m_nPixHoles_incl = nullptr;
      TH1* m_nsharedPixHits_incl = nullptr;
      TH1* m_nsplitPixHits_incl = nullptr;
      TH1* m_nSCTHits_incl = nullptr;
      TH1* m_nSCTHoles_incl = nullptr;
      TH1* m_nsharedSCTHits_incl = nullptr;
      
      TH1* m_nInnHits_b = nullptr;
      TH1* m_nNextToInnHits_b = nullptr;
      TH1* m_nBLHits_b = nullptr;
      TH1* m_nsharedBLHits_b = nullptr;
      TH1* m_nsplitBLHits_b = nullptr;
      TH1* m_nPixHits_b = nullptr;
      TH1* m_nPixHoles_b = nullptr;
      TH1* m_nsharedPixHits_b = nullptr;
      TH1* m_nsplitPixHits_b = nullptr;
      TH1* m_nSCTHits_b = nullptr;
      TH1* m_nSCTHoles_b = nullptr;
      TH1* m_nsharedSCTHits_b = nullptr;
      
      TH1* m_nInnHits_c = nullptr;
      TH1* m_nNextToInnHits_c = nullptr;
      TH1* m_nBLHits_c = nullptr;
      TH1* m_nsharedBLHits_c = nullptr;
      TH1* m_nsplitBLHits_c = nullptr;
      TH1* m_nPixHits_c = nullptr;
      TH1* m_nPixHoles_c = nullptr;
      TH1* m_nsharedPixHits_c = nullptr;
      TH1* m_nsplitPixHits_c = nullptr;
      TH1* m_nSCTHits_c = nullptr;
      TH1* m_nSCTHoles_c = nullptr;
      TH1* m_nsharedSCTHits_c = nullptr;
      
      TH1* m_nInnHits_u = nullptr;
      TH1* m_nNextToInnHits_u = nullptr;
      TH1* m_nBLHits_u = nullptr;
      TH1* m_nsharedBLHits_u = nullptr;
      TH1* m_nsplitBLHits_u = nullptr;
      TH1* m_nPixHits_u = nullptr;
      TH1* m_nPixHoles_u = nullptr;
      TH1* m_nsharedPixHits_u = nullptr;
      TH1* m_nsplitPixHits_u = nullptr;
      TH1* m_nSCTHits_u = nullptr;
      TH1* m_nSCTHoles_u = nullptr;
      TH1* m_nsharedSCTHits_u = nullptr;
      
      TH1* m_nInnHits_muon = nullptr;
      TH1* m_nNextToInnHits_muon = nullptr;
      TH1* m_nBLHits_muon = nullptr;
      TH1* m_nsharedBLHits_muon = nullptr;
      TH1* m_nsplitBLHits_muon = nullptr;
      TH1* m_nPixHits_muon = nullptr;
      TH1* m_nPixHoles_muon = nullptr;
      TH1* m_nsharedPixHits_muon = nullptr;
      TH1* m_nsplitPixHits_muon = nullptr;
      TH1* m_nSCTHits_muon = nullptr;
      TH1* m_nSCTHoles_muon = nullptr;
      TH1* m_nsharedSCTHits_muon = nullptr;
      
      // numTracks_perJet
      TH1* m_numTracks_perJet_incl = nullptr;
      TH1* m_numTracks_perJet_b = nullptr;
      TH1* m_numTracks_perJet_c = nullptr;
      TH1* m_numTracks_perJet_u = nullptr;
      TH1* m_numTracks_perJet_top = nullptr;
      TH1* m_numTracks_perJet_muon = nullptr;

      // number of tracks variables
      TH1* m_numTracks_B_incl = nullptr; 
      TH1* m_numTracks_C_incl = nullptr; 
      TH1* m_numTracks_Fragmentation_incl = nullptr; 
      TH1* m_numTracks_Secondaries_incl = nullptr; 
      TH1* m_numTracks_Pileup_incl = nullptr; 
      TH1* m_numTracks_Fake_incl = nullptr; 

      TH1* m_numTracks_B_b = nullptr; 
      TH1* m_numTracks_C_b = nullptr; 
      TH1* m_numTracks_Fragmentation_b = nullptr; 
      TH1* m_numTracks_Secondaries_b = nullptr; 
      TH1* m_numTracks_Pileup_b = nullptr; 
      TH1* m_numTracks_Fake_b = nullptr; 

      TH1* m_numTracks_B_c = nullptr; 
      TH1* m_numTracks_C_c = nullptr; 
      TH1* m_numTracks_Fragmentation_c = nullptr; 
      TH1* m_numTracks_Secondaries_c = nullptr; 
      TH1* m_numTracks_Pileup_c = nullptr; 
      TH1* m_numTracks_Fake_c = nullptr; 

      TH1* m_numTracks_B_u = nullptr; 
      TH1* m_numTracks_C_u = nullptr; 
      TH1* m_numTracks_Fragmentation_u = nullptr; 
      TH1* m_numTracks_Secondaries_u = nullptr; 
      TH1* m_numTracks_Pileup_u = nullptr; 
      TH1* m_numTracks_Fake_u = nullptr; 

      TH1* m_numTracks_B_muon = nullptr; 
      TH1* m_numTracks_C_muon = nullptr; 
      TH1* m_numTracks_Fragmentation_muon = nullptr; 
      TH1* m_numTracks_Secondaries_muon = nullptr; 
      TH1* m_numTracks_Pileup_muon = nullptr; 
      TH1* m_numTracks_Fake_muon = nullptr; 

      // these are only filled for detail level above 10
      TH1* m_numTracks_Secondaries_KshortDecay_incl = nullptr; 
      TH1* m_numTracks_Secondaries_KshortDecay_b = nullptr; 
      TH1* m_numTracks_Secondaries_KshortDecay_c = nullptr; 
      TH1* m_numTracks_Secondaries_KshortDecay_u = nullptr; 
      TH1* m_numTracks_Secondaries_KshortDecay_muon = nullptr; 
  
      TH1* m_numTracks_Secondaries_LambdaDecay_incl = nullptr; 
      TH1* m_numTracks_Secondaries_LambdaDecay_b = nullptr; 
      TH1* m_numTracks_Secondaries_LambdaDecay_c = nullptr; 
      TH1* m_numTracks_Secondaries_LambdaDecay_u = nullptr; 
      TH1* m_numTracks_Secondaries_LambdaDecay_muon = nullptr; 
  
      TH1* m_numTracks_Secondaries_GammaConversion_incl = nullptr; 
      TH1* m_numTracks_Secondaries_GammaConversion_b = nullptr; 
      TH1* m_numTracks_Secondaries_GammaConversion_c = nullptr; 
      TH1* m_numTracks_Secondaries_GammaConversion_u = nullptr; 
      TH1* m_numTracks_Secondaries_GammaConversion_muon = nullptr; 
  
      TH1* m_numTracks_Secondaries_OtherDecay_incl = nullptr; 
      TH1* m_numTracks_Secondaries_OtherDecay_b = nullptr; 
      TH1* m_numTracks_Secondaries_OtherDecay_c = nullptr; 
      TH1* m_numTracks_Secondaries_OtherDecay_u = nullptr; 
      TH1* m_numTracks_Secondaries_OtherDecay_muon = nullptr; 
  
      TH1* m_numTracks_Secondaries_HadronicInteraction_incl = nullptr; 
      TH1* m_numTracks_Secondaries_HadronicInteraction_b = nullptr; 
      TH1* m_numTracks_Secondaries_HadronicInteraction_c = nullptr; 
      TH1* m_numTracks_Secondaries_HadronicInteraction_u = nullptr; 
      TH1* m_numTracks_Secondaries_HadronicInteraction_muon = nullptr; 
  
      TH1* m_numTracks_Secondaries_OtherSecondary_incl = nullptr; 
      TH1* m_numTracks_Secondaries_OtherSecondary_b = nullptr; 
      TH1* m_numTracks_Secondaries_OtherSecondary_c = nullptr; 
      TH1* m_numTracks_Secondaries_OtherSecondary_u = nullptr; 
      TH1* m_numTracks_Secondaries_OtherSecondary_muon = nullptr; 
  
      TH1* m_numTracks_OtherOrigin_incl = nullptr; 
      TH1* m_numTracks_OtherOrigin_b = nullptr; 
      TH1* m_numTracks_OtherOrigin_c = nullptr; 
      TH1* m_numTracks_OtherOrigin_u = nullptr; 
      TH1* m_numTracks_OtherOrigin_muon = nullptr; 

      // features for largeRjet tagger
      TH1* m_track_d0_top = nullptr;
      TH1* m_track_z0_top = nullptr;
      TH1* m_track_sigd0_top = nullptr;
      TH1* m_track_sigz0_top = nullptr;
      // pT_frac
      TH1* m_track_pT_frac_top = nullptr;
      // DeltaR_jet_track
      TH1* m_DeltaR_jet_track_top = nullptr;
      // tracker hits
      TH1* m_nInnHits_top = nullptr;
      TH1* m_nNextToInnHits_top = nullptr;
      TH1* m_nBLHits_top = nullptr;
      TH1* m_nsharedBLHits_top = nullptr;
      TH1* m_nsplitBLHits_top = nullptr;
      TH1* m_nPixHits_top = nullptr;
      TH1* m_nPixHoles_top = nullptr;
      TH1* m_nsharedPixHits_top = nullptr;
      TH1* m_nsplitPixHits_top = nullptr;
      TH1* m_nSCTHits_top = nullptr;
      TH1* m_nSCTHoles_top = nullptr;
      TH1* m_nsharedSCTHits_top = nullptr;

      // tagger
      TH1* m_GN2v01_pb = nullptr;
      TH1* m_GN2v01_pc = nullptr;
      TH1* m_GN2v01_pu = nullptr;
      TH1* m_GN2v01_ptau = nullptr;

      TH1* m_GN3XPV01_phtautauhad = nullptr;
      TH1* m_GN3XPV01_phbb = nullptr;
      TH1* m_GN3XPV01_phcc = nullptr;
      TH1* m_GN3XPV01_ptop = nullptr;
      TH1* m_GN3XPV01_pqcdbb = nullptr;
      TH1* m_GN3XPV01_pqcdbx = nullptr;
      TH1* m_GN3XPV01_pqcdcx = nullptr;
      TH1* m_GN3XPV01_pqcdll = nullptr;
      TH1* m_GN3XPV01_pwqq = nullptr;


      // B hadron Lxy
      TH1* m_Truth_Lxy_b = nullptr;
      TH1* m_Truth_Lxy_c = nullptr;
      
      // B hadron deltaR wrt jet 
      TH1* m_deltaR_truthBHadron_jet_b = nullptr;
      TH1* m_deltaR_truthCHadron_jet_c = nullptr;
      
      std::vector<std::string> m_taggers;
      std::map<std::string, int> m_truthLabels;
      std::map<std::string, double> m_GN2v01_workingPoints;
      std::map<std::string, double> m_GN3XPV01_workingPoints; // TODO: Change this in the future since GN3 has WPs pT and mass dependent

      double m_GN2v01_fc = 0.0;
      double m_GN2v01_ftau = 0.0;
      double m_GN3XPV01_hcc_fc = 0.0;
      double m_GN3XPV01_top_fc = 0.0;
      std::map<std::string, TH1*> m_weight_histos; 

      std::map<std::string, TH1*> m_nJetsThatPassedWPCutsHistos; 

      // detail level
      void setDetailLevel(const unsigned int& detailLevel);

      // a setter for the HistogramDefinitions and the jvt and TMP cuts
      void setHistogramDefinitions( std::map< std::string, std::vector< std::string > > HistogramDefinitions);
      void setIsDataJVTCutsAndTMPCut(bool isData, float JVTCutAntiKt4EMTopoJets, float JVTCutLargerEtaAntiKt4EMTopoJets, float JVTCutAntiKt4EMPFlowJets, float truthMatchProbabilityCut);
      void setTaggerNames(const std::string& GN2v01Name, const std::string& GN3XPV01Name);

      // jvt variables 
      bool m_JVT_defined{};
      float m_JVT_cut = 0.0F;
      bool m_JVTLargerEta_defined;
      float m_JVTLargerEta_cut = 0.0F;

    private:
      virtual void initializePlots();     
      virtual void finalizePlots(); 

      // detail level
      unsigned int m_detailLevel = 0U;

      // map with histogram definitions and the corresponding enum
      std::map< std::string, std::vector< std::string > > m_HistogramDefinitions;
      enum position{histo_name, histo_title, histo_path, histo_xbins, histo_xmin, histo_xmax, histo_type, histo_ymin, histo_ymax};
      float m_truthMatchProbabilityCut = 0.0F;
      bool m_isData = false;
      // some helper functions
      TH1* bookHistogram(std::string histo_name, const std::string& var_name, const std::string& part = "", const std::string& prefix = "");
      int getTrackHits(const xAOD::TrackParticle& part, xAOD::SummaryType info);
      void fillDiscriminantHistograms(const std::string& tagger_name, const double& discriminant_value, const std::map<std::string, double>& working_points, const int& truth_label, std::map<std::string, TH1*>::const_iterator hist_iter, std::map<std::string, int>::const_iterator label_iter, const bool& pass_nTracksCut, const double& jet_pT, const double& jet_Lxy, const bool& onZprime, const xAOD::EventInfo* event);
      void bookDiscriminantVsPTAndLxyHistograms(const std::string& tagger_name, const std::map<std::string, double>& workingPoints, const bool& isOldTagger, std::map<std::string, int>::const_iterator label_iter, const std::string& m_sParticleType);
      template <class T>
      void fillHistoWithTruthCases(T value, TH1* histo_incl, TH1* histo_b, TH1* histo_c, TH1* histo_l, TH1* histo_muon, const int& truth_label, const bool& has_muon, const xAOD::EventInfo* event);
      template <class T>
      void fillHistoWithTruthCases_for_largeRjet(T value, TH1* histo_incl, TH1* histo_bb, TH1* histo_cc, TH1* histo_uu, TH1* histo_top, const int& truth_label, const xAOD::EventInfo* event);

      // tagger names
      std::string m_GN2v01Name;
      std::string m_GN3XPV01Name;
  
  };
    
}
    
#endif
