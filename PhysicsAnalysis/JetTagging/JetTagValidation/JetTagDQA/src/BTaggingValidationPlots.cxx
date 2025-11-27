/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BTaggingValidationPlots.h"
#include "ParticleJetTools/JetFlavourInfo.h"
#include "xAODBTagging/BTaggingUtilities.h" 
#include "CLHEP/Units/SystemOfUnits.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"//TrkOrigin
#include "xAODTruth/TruthParticle.h"
#include "xAODJet/Jet.h"
#include "xAODTracking/Vertex.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "AthContainers/ConstAccessor.h"

#include <stdexcept>
#include <utility>

using CLHEP::GeV;
namespace JetTagDQA{
  BTaggingValidationPlots::BTaggingValidationPlots(PlotBase* pParent, 
                                                   const std::string& sDir, 
                                                   std::string sParticleType) :
                                                   PlotBase(pParent, sDir),
                                                   AthMessaging("BTaggingValidationPlots"),
                                                   m_sParticleType(std::move(sParticleType)),
                                                   m_JVT_defined(false),
                                                   m_JVTLargerEta_defined(false)
  {
    //std::cout << "m_sParticleType=" << m_sParticleType << std::endl;
  }     
  
  void BTaggingValidationPlots::setDetailLevel(const unsigned int& detailLevel){
    m_detailLevel = detailLevel;
  }

  // implement the setter function for the histogram definitions
  void BTaggingValidationPlots::setHistogramDefinitions( std::map< std::string, std::vector< std::string > > HistogramDefinitions){
    m_HistogramDefinitions = std::move(HistogramDefinitions);
  }
  
  // implement the setter function for the cuts that can be set in the config
  void BTaggingValidationPlots::setIsDataJVTCutsAndTMPCut(bool isData, float JVTCutAntiKt4EMTopoJets, float JVTCutLargerEtaAntiKt4EMTopoJets, float JVTCutAntiKt4EMPFlowJets, float truthMatchProbabilityCut){
    m_isData = isData;
    if (m_sParticleType=="antiKt4EMTopoJets"){ 
      m_JVT_defined = true; 
      m_JVT_cut = JVTCutAntiKt4EMTopoJets;
      m_JVTLargerEta_defined = true;
      m_JVTLargerEta_cut = JVTCutLargerEtaAntiKt4EMTopoJets;
    }
    if (m_sParticleType=="antiKt4EMPFlowJets"){
      m_JVT_defined = true; 
      m_JVT_cut = JVTCutAntiKt4EMPFlowJets;
    }
    m_truthMatchProbabilityCut = truthMatchProbabilityCut;
  }

  void BTaggingValidationPlots::setTaggerNames(
                 const std::string& GN2v01Name,
					       const std::string& GN3XPV01Name){
    m_GN2v01Name = GN2v01Name;
    m_GN3XPV01Name = GN3XPV01Name;
  }
  
  // implement the bookHistogram function using the histogram definitions
  TH1* BTaggingValidationPlots::bookHistogram(std::string histo_name, const std::string& var_name, const std::string& part, const std::string& prefix){
    // check if the var is in the histogram definitions
    if(m_HistogramDefinitions.find(var_name) == m_HistogramDefinitions.end()) {
      throw std::invalid_argument("var_name " + var_name + " not in HistogramDefinitions.");
    }
    if(m_HistogramDefinitions.at(var_name)[histo_type] != "TH1D") {
      throw std::invalid_argument("The variable " + var_name + " not defined as TH1D.");
    }
    
    // get the title
    std::string title = ""; 
    if(part != "") title += part + " - ";
    if(prefix != "") title += prefix + " ";
    title += m_HistogramDefinitions.at(var_name)[histo_title];
    // add the category to the name
    histo_name = m_HistogramDefinitions.at(var_name)[histo_path] + "_" + histo_name;
    // get the bins
    double xbins = std::stod(m_HistogramDefinitions.at(var_name)[histo_xbins]);
    double xmin = std::stod(m_HistogramDefinitions.at(var_name)[histo_xmin]);
    double xmax = std::stod(m_HistogramDefinitions.at(var_name)[histo_xmax]);
    // book and return the histo
    return Book1D(histo_name, title, xbins, xmin, xmax);
  }
   

  // util function to get track values
  int BTaggingValidationPlots::getTrackHits(const xAOD::TrackParticle& part, xAOD::SummaryType info) {
    uint8_t val;
    bool ok = part.summaryValue(val, info);
    if (!ok) throw std::logic_error("Problem getting track summary value.");
    return val;
  }

  // util function to fill the tagger discriminant related histograms
  void BTaggingValidationPlots::fillDiscriminantHistograms(const std::string& tagger_name, const double& discriminant_value, const std::map<std::string, double>& working_points, const int& truth_label, std::map<std::string, TH1*>::const_iterator hist_iter, std::map<std::string, int>::const_iterator label_iter, const bool& pass_nTracksCut, const double& jet_pT, const double& jet_Lxy, const bool& onZprime, const xAOD::EventInfo* event){
    // TODO: GN3XPV01 will not fill any WPs for now. This need to be adapted when WPs as a function of pT and mass are defined.
    if (tagger_name == "GN3XPV01_") {
      ATH_MSG_WARNING("GN3XPV01 has no WPs defined. Not filling any WP-related histograms.");
      return;
    }
    // check if the current histogram is to be filled with this tagger discriminant
    if((hist_iter->first).find(tagger_name) < 1 && (hist_iter->first).find("matched") < (hist_iter->first).length()){                        
      // check if the current histograms is to be filled with the discrinimant and the current truth label
      if( (hist_iter->first).find("_"+label_iter->first+"_") < (hist_iter->first).length() && truth_label == label_iter->second && (hist_iter->first).find("_weight") < (hist_iter->first).length()){
   
        // now fill it if it is the nTracksCut histogram
        if((hist_iter->first).find("_trackCuts") < (hist_iter->first).length()){
          if(pass_nTracksCut) (hist_iter->second)->Fill(discriminant_value, event->beamSpotWeight());
        }
        // else fill it without the nTracksCut
        else{
          (hist_iter->second)->Fill(discriminant_value, event->beamSpotWeight());
        }
      }
      // if it's not to be filled with the discriminant it's the jet pT with discriminant cut selection
      else if((hist_iter->first).find("_matched_pt") < (hist_iter->first).length()){
        for(std::map<std::string, double>::const_iterator working_points_iter = working_points.begin(); working_points_iter != working_points.end(); ++working_points_iter){
          // check if the current histogram is the right one
          if((hist_iter->first).find(working_points_iter->first) < (hist_iter->first).length() && (hist_iter->first).find("_"+label_iter->first+"_") < (hist_iter->first).length()){
            if(truth_label == label_iter->second && discriminant_value > working_points_iter->second){
              // check if we're on a Zprime sample)
              if((hist_iter->first).find("Zprime") < (hist_iter->first).length()){
                if(onZprime) (hist_iter->second)->Fill(jet_pT/GeV, event->beamSpotWeight());
              } else {
                if(!onZprime) (hist_iter->second)->Fill(jet_pT/GeV, event->beamSpotWeight());
              }
            }
          }
        }
      }    
      else if((hist_iter->first).find("_matched_Lxy") < (hist_iter->first).length()){
        for(std::map<std::string, double>::const_iterator working_points_iter = working_points.begin(); working_points_iter != working_points.end(); ++working_points_iter){
          // check if the current histogram is the right one
          if((hist_iter->first).find(working_points_iter->first) < (hist_iter->first).length() && (hist_iter->first).find("_"+label_iter->first+"_") < (hist_iter->first).length()){
            if(truth_label == label_iter->second && discriminant_value > working_points_iter->second){
              (hist_iter->second)->Fill(jet_Lxy, event->beamSpotWeight());
            }
          }
        }
      }    
    }
  }
   

  // util function to book the discriminant related vs pT plots
  void BTaggingValidationPlots::bookDiscriminantVsPTAndLxyHistograms(const std::string& tagger_name, const std::map<std::string, double>& workingPoints, const bool& isOldTagger, std::map<std::string, int>::const_iterator label_iter, const std::string& sParticleType){
    for(std::map<std::string, double>::const_iterator working_points_iter = workingPoints.begin(); working_points_iter != workingPoints.end(); ++working_points_iter){
      // book pT histogram normal
      std::string histo_name_matched = tagger_name + "_" + label_iter->first + "_" + working_points_iter->first + "_matched_pt_ttbar";
      std::string var_name = "pT_llrCut";  
      if(isOldTagger) var_name += "_old_taggers";
      TH1* histo_matched = bookHistogram(histo_name_matched, var_name, sParticleType, label_iter->first + "-jets" + ", for " + tagger_name + " llr > "+ std::to_string(working_points_iter->second) + ": " );    
      m_weight_histos.insert(std::make_pair(histo_name_matched, histo_matched));

      // book pT histogram for Zprime
      std::string histo_name_matched_Zprime = tagger_name + "_" + label_iter->first + "_" + working_points_iter->first + "_matched_pt_Zprime";
      std::string var_name_Zprime = "pT_llrCut_Zprime";  
      if(isOldTagger) var_name_Zprime += "_old_taggers";
      TH1* histo_matched_Zprime = bookHistogram(histo_name_matched_Zprime, var_name_Zprime, sParticleType, label_iter->first + "-jets" + ", for " + tagger_name + " llr > "+ std::to_string(working_points_iter->second) + ": " );    
      m_weight_histos.insert(std::make_pair(histo_name_matched_Zprime, histo_matched_Zprime));

      // book Lxy histogram
      std::string histo_name_matched_Lxy = tagger_name + "_" + label_iter->first + "_" + working_points_iter->first + "_matched_Lxy";
      std::string var_name_Lxy = "Lxy_llrCut";  
      if(isOldTagger) var_name_Lxy += "_old_taggers";
      TH1* histo_matched_Lxy = bookHistogram(histo_name_matched_Lxy, var_name_Lxy, sParticleType, label_iter->first + "-jets" + ", for " + tagger_name + " llr > "+ std::to_string(working_points_iter->second) + ": " );    
      m_weight_histos.insert(std::make_pair(histo_name_matched_Lxy, histo_matched_Lxy));

    }
  }


  template <class T>
  void BTaggingValidationPlots::fillHistoWithTruthCases(T value, TH1* histo_incl, TH1* histo_b, TH1* histo_c, TH1* histo_l, TH1* histo_muon, const int& truth_label, const bool& has_muon, const xAOD::EventInfo* event){

      // the inclusive ones in any case
      histo_incl -> Fill( value, event->beamSpotWeight() );

      // truth cases
      if(!m_isData && truth_label == 5) {
        histo_b -> Fill( value, event->beamSpotWeight() );
      }
      else if(!m_isData && truth_label == 4) {
        histo_c -> Fill( value, event->beamSpotWeight() );
      }
      else if(!m_isData && truth_label == 0) {
        histo_l -> Fill( value, event->beamSpotWeight() );
      }
      
      // muon
      if(has_muon) {
        histo_muon -> Fill( value, event->beamSpotWeight() );
      }
  }

  template <class T>
  void BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(T value, TH1* histo_incl, TH1* histo_bb, TH1* histo_cc, TH1* histo_uu, TH1* histo_top, const int& truth_label, const xAOD::EventInfo* event){

      // the inclusive ones in any case
      histo_incl -> Fill( value, event->beamSpotWeight() );

      // truth cases
      //see definitions in https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/ParticleJetTools/ParticleJetTools/LargeRJetLabelEnum.h
      if(!m_isData && (truth_label == 3 || truth_label == 11)) { // H or Z->bb
        histo_bb -> Fill( value, event->beamSpotWeight() );
      }
      else if(!m_isData && (truth_label == 4 || truth_label == 12)) {// H or Z->cc
        histo_cc -> Fill( value, event->beamSpotWeight() );
      }
      else if(!m_isData && (truth_label == 5 || truth_label == 2 || truth_label == 6 || truth_label == 10)) { // Z or W -> qq or QCD
        histo_uu -> Fill( value, event->beamSpotWeight() );
      }
      else if(!m_isData && (truth_label == 1)) { // t -> bqq
        histo_top -> Fill( value, event->beamSpotWeight() );
      }
  }

   
  void BTaggingValidationPlots::initializePlots(){

    bookEffHistos();        

    // multiplicities
    m_nJets = bookHistogram("nJets", "nJets", m_sParticleType);
    m_nTracks = bookHistogram("nTracks", "nTracks", m_sParticleType);
    m_nPrimVtx = bookHistogram("nPrimVtx", "nPrimVtx");
    m_nTracksPrimVtx = bookHistogram("nTracksPrimVtx", "nTracksPrimVtx");
    m_nJetsWithMuon = bookHistogram("nJetsWithMuon", "nJetsWithMuon", m_sParticleType);
    m_nJetsWithSV = bookHistogram("nJetsWithSV", "nJetsWithSV", m_sParticleType);
    m_fracJetsWithMuon = bookHistogram("fracJetsWithMuon", "fracJetsWithMuon", m_sParticleType);
    m_fracJetsWithSV = bookHistogram("fracJetsWithSV", "fracJetsWithSV", m_sParticleType);

    // PV vars
    m_PV_x = bookHistogram("PV_x", "PV_x");
    m_PV_y = bookHistogram("PV_y", "PV_y");
    m_PV_z = bookHistogram("PV_z", "PV_z");

    // jet kinematc vars
    m_jet_e  = bookHistogram("jet_e", "jet_E", m_sParticleType);
    m_jet_e_Zprime  = bookHistogram("jet_e_Zprime", "jet_E_Zprime", m_sParticleType);
    m_jet_pt  = bookHistogram("jet_pt_ttbar", "jet_pT", m_sParticleType);
    m_jet_pt_Zprime  = bookHistogram("jet_pt_Zprime", "jet_pT_Zprime", m_sParticleType);
    m_jet_eta  = bookHistogram("jet_eta", "jet_eta", m_sParticleType);
    m_jet_phi  = bookHistogram("jet_phi", "jet_phi", m_sParticleType);

    // muon vars
    m_muon_pT_frac = bookHistogram("muon_pT_frac", "muon_pT_frac", m_sParticleType); 

    // truth info
    m_truthLabel  = bookHistogram("truthLabel", "truth_label", m_sParticleType);

    m_jet_pt_b  = bookHistogram("jet_pt_b_ttbar", "jet_pT", m_sParticleType, "b-jets - ");
    m_jet_pt_c  = bookHistogram("jet_pt_c_ttbar", "jet_pT", m_sParticleType, "c-jets - ");
    m_jet_pt_l  = bookHistogram("jet_pt_l_ttbar", "jet_pT", m_sParticleType, "l-jets - ");
    m_jet_pt_top  = bookHistogram("jet_pt_top_ttbar", "jet_pT", m_sParticleType, "top-jets - ");

    m_jet_pt_Zprime_b  = bookHistogram("jet_pt_b_Zprime", "jet_pT_Zprime", m_sParticleType, "b-jets - ");
    m_jet_pt_Zprime_c  = bookHistogram("jet_pt_c_Zprime", "jet_pT_Zprime", m_sParticleType, "c-jets - ");
    m_jet_pt_Zprime_l  = bookHistogram("jet_pt_l_Zprime", "jet_pT_Zprime", m_sParticleType, "l-jets - ");

    m_jet_eta_b  = bookHistogram("jet_eta_b", "jet_eta", m_sParticleType, "b-jets - ");
    m_jet_eta_c  = bookHistogram("jet_eta_c", "jet_eta", m_sParticleType, "c-jets - ");
    m_jet_eta_l  = bookHistogram("jet_eta_l", "jet_eta", m_sParticleType, "l-jets - ");
    m_jet_eta_top  = bookHistogram("jet_eta_top", "jet_eta", m_sParticleType, "top-jets - ");

    // SV1 related vars
    m_SV1_numSVs_incl = bookHistogram("SV1_numSVs_incl", "SV1_numSVs", m_sParticleType);
    m_SV1_masssvx_incl = bookHistogram("SV1_masssvx_incl", "SV1_masssvx", m_sParticleType);
    m_SV1_N2Tpair_incl = bookHistogram("SV1_N2Tpair_incl", "SV1_N2Tpair", m_sParticleType);
    m_SV1_efracsvx_incl = bookHistogram("SV1_efracsvx_incl", "SV1_efracsvx", m_sParticleType);
    m_SV1_deltaR_incl = bookHistogram("SV1_deltaR_incl", "SV1_deltaR", m_sParticleType);
    m_SV1_significance3d_incl = bookHistogram("SV1_significance3d_incl", "SV1_significance3d", m_sParticleType);
    m_SV1_energyTrkInJet_incl = bookHistogram("SV1_energyTrkInJet_incl", "SV1_energyTrkInJet", m_sParticleType);
    m_SV1_NGTinSvx_incl = bookHistogram("SV1_NGTinSvx_incl", "SV1_NGTinSvx", m_sParticleType);
    m_SV1_Lxy_incl = bookHistogram("SV1_Lxy_incl", "SV1_Lxy", m_sParticleType);
    m_SV1_purity_incl = bookHistogram("SV1_purity_incl", "SV1_purity", m_sParticleType);
    m_SV1_fracTracks_fromB_incl = bookHistogram("SV1_fracTracks_fromB_incl", "SV1_fracTracks_from_B", m_sParticleType);
    m_SV1_fracTracks_fromC_incl = bookHistogram("SV1_fracTracks_fromC_incl", "SV1_fracTracks_from_C", m_sParticleType);
    m_SV1_fracTracks_fromFragmentation_incl = bookHistogram("SV1_fracTracks_fromFragmentation_incl", "SV1_fracTracks_from_Fragmentation", m_sParticleType);
    m_SV1_fracTracks_fromSecondaries_incl = bookHistogram("SV1_fracTracks_fromSecondaries_incl", "SV1_fracTracks_from_Secondaries", m_sParticleType);
    m_SV1_fracTracks_fromPileup_incl = bookHistogram("SV1_fracTracks_fromPileup_incl", "SV1_fracTracks_from_Pileup", m_sParticleType);
    m_SV1_fracTracks_fromFake_incl = bookHistogram("SV1_fracTracks_fromFake_incl", "SV1_fracTracks_from_Fake", m_sParticleType);

    m_SV1_numSVs_b = bookHistogram("SV1_numSVs_b", "SV1_numSVs", m_sParticleType, "b-jets - ");
    m_SV1_masssvx_b = bookHistogram("SV1_masssvx_b", "SV1_masssvx", m_sParticleType, "b-jets - ");
    m_SV1_N2Tpair_b = bookHistogram("SV1_N2Tpair_b", "SV1_N2Tpair", m_sParticleType, "b-jets - ");
    m_SV1_efracsvx_b = bookHistogram("SV1_efracsvx_b", "SV1_efracsvx", m_sParticleType, "b-jets - ");
    m_SV1_deltaR_b = bookHistogram("SV1_deltaR_b", "SV1_deltaR", m_sParticleType, "b-jets - ");
    m_SV1_significance3d_b = bookHistogram("SV1_significance3d_b", "SV1_significance3d", m_sParticleType, "b-jets - ");
    m_SV1_energyTrkInJet_b = bookHistogram("SV1_energyTrkInJet_b", "SV1_energyTrkInJet", m_sParticleType, "b-jets - ");
    m_SV1_NGTinSvx_b = bookHistogram("SV1_NGTinSvx_b", "SV1_NGTinSvx", m_sParticleType, "b-jets - ");
    m_SV1_Lxy_b = bookHistogram("SV1_Lxy_b", "SV1_Lxy", m_sParticleType, "b-jets - ");
    m_SV1_purity_b = bookHistogram("SV1_purity_b", "SV1_purity", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromB_b = bookHistogram("SV1_fracTracks_fromB_b", "SV1_fracTracks_from_B", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromC_b = bookHistogram("SV1_fracTracks_fromC_b", "SV1_fracTracks_from_C", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromFragmentation_b = bookHistogram("SV1_fracTracks_fromFragmentation_b", "SV1_fracTracks_from_Fragmentation", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromSecondaries_b = bookHistogram("SV1_fracTracks_fromSecondaries_b", "SV1_fracTracks_from_Secondaries", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromPileup_b = bookHistogram("SV1_fracTracks_fromPileup_b", "SV1_fracTracks_from_Pileup", m_sParticleType, "b-jets - ");
    m_SV1_fracTracks_fromFake_b = bookHistogram("SV1_fracTracks_fromFake_b", "SV1_fracTracks_from_Fake", m_sParticleType, "b-jets - ");

    m_SV1_numSVs_c = bookHistogram("SV1_numSVs_c", "SV1_numSVs", m_sParticleType, "c-jets - ");
    m_SV1_masssvx_c = bookHistogram("SV1_masssvx_c", "SV1_masssvx", m_sParticleType, "c-jets - ");
    m_SV1_N2Tpair_c = bookHistogram("SV1_N2Tpair_c", "SV1_N2Tpair", m_sParticleType, "c-jets - ");
    m_SV1_efracsvx_c = bookHistogram("SV1_efracsvx_c", "SV1_efracsvx", m_sParticleType, "c-jets - ");
    m_SV1_deltaR_c = bookHistogram("SV1_deltaR_c", "SV1_deltaR", m_sParticleType, "c-jets - ");
    m_SV1_significance3d_c = bookHistogram("SV1_significance3d_c", "SV1_significance3d", m_sParticleType, "c-jets - ");
    m_SV1_energyTrkInJet_c = bookHistogram("SV1_energyTrkInJet_c", "SV1_energyTrkInJet", m_sParticleType, "c-jets - ");
    m_SV1_NGTinSvx_c = bookHistogram("SV1_NGTinSvx_c", "SV1_NGTinSvx", m_sParticleType, "c-jets - ");
    m_SV1_Lxy_c = bookHistogram("SV1_Lxy_c", "SV1_Lxy", m_sParticleType, "c-jets - ");
    m_SV1_purity_c = bookHistogram("SV1_purity_c", "SV1_purity", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromB_c = bookHistogram("SV1_fracTracks_fromB_c", "SV1_fracTracks_from_B", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromC_c = bookHistogram("SV1_fracTracks_fromC_c", "SV1_fracTracks_from_C", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromFragmentation_c = bookHistogram("SV1_fracTracks_fromFragmentation_c", "SV1_fracTracks_from_Fragmentation", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromSecondaries_c = bookHistogram("SV1_fracTracks_fromSecondaries_c", "SV1_fracTracks_from_Secondaries", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromPileup_c = bookHistogram("SV1_fracTracks_fromPileup_c", "SV1_fracTracks_from_Pileup", m_sParticleType, "c-jets - ");
    m_SV1_fracTracks_fromFake_c = bookHistogram("SV1_fracTracks_fromFake_c", "SV1_fracTracks_from_Fake", m_sParticleType, "c-jets - ");

    m_SV1_numSVs_l = bookHistogram("SV1_numSVs_l", "SV1_numSVs", m_sParticleType, "l-jets - ");
    m_SV1_masssvx_l = bookHistogram("SV1_masssvx_l", "SV1_masssvx", m_sParticleType, "l-jets - ");
    m_SV1_N2Tpair_l = bookHistogram("SV1_N2Tpair_l", "SV1_N2Tpair", m_sParticleType, "l-jets - ");
    m_SV1_efracsvx_l = bookHistogram("SV1_efracsvx_l", "SV1_efracsvx", m_sParticleType, "l-jets - ");
    m_SV1_deltaR_l = bookHistogram("SV1_deltaR_l", "SV1_deltaR", m_sParticleType, "l-jets - ");
    m_SV1_significance3d_l = bookHistogram("SV1_significance3d_l", "SV1_significance3d", m_sParticleType, "l-jets - ");
    m_SV1_energyTrkInJet_l = bookHistogram("SV1_energyTrkInJet_l", "SV1_energyTrkInJet", m_sParticleType, "l-jets - ");
    m_SV1_NGTinSvx_l = bookHistogram("SV1_NGTinSvx_l", "SV1_NGTinSvx", m_sParticleType, "l-jets - ");
    m_SV1_Lxy_l = bookHistogram("SV1_Lxy_l", "SV1_Lxy", m_sParticleType, "l-jets - ");
    m_SV1_purity_l = bookHistogram("SV1_purity_l", "SV1_purity", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromB_l = bookHistogram("SV1_fracTracks_fromB_l", "SV1_fracTracks_from_B", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromC_l = bookHistogram("SV1_fracTracks_fromC_l", "SV1_fracTracks_from_C", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromFragmentation_l = bookHistogram("SV1_fracTracks_fromFragmentation_l", "SV1_fracTracks_from_Fragmentation", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromSecondaries_l = bookHistogram("SV1_fracTracks_fromSecondaries_l", "SV1_fracTracks_from_Secondaries", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromPileup_l = bookHistogram("SV1_fracTracks_fromPileup_l", "SV1_fracTracks_from_Pileup", m_sParticleType, "l-jets - ");
    m_SV1_fracTracks_fromFake_l = bookHistogram("SV1_fracTracks_fromFake_l", "SV1_fracTracks_from_Fake", m_sParticleType, "l-jets - ");

    m_SV1_numSVs_muon = bookHistogram("SV1_numSVs_muon", "SV1_numSVs", m_sParticleType, "jets with muon - ");
    m_SV1_masssvx_muon = bookHistogram("SV1_masssvx_muon", "SV1_masssvx", m_sParticleType, "jets with muon - ");
    m_SV1_N2Tpair_muon = bookHistogram("SV1_N2Tpair_muon", "SV1_N2Tpair", m_sParticleType, "jets with muon - ");
    m_SV1_efracsvx_muon = bookHistogram("SV1_efracsvx_muon", "SV1_efracsvx", m_sParticleType, "jets with muon - ");
    m_SV1_deltaR_muon = bookHistogram("SV1_deltaR_muon", "SV1_deltaR", m_sParticleType, "jets with muon - ");
    m_SV1_significance3d_muon = bookHistogram("SV1_significance3d_muon", "SV1_significance3d", m_sParticleType, "jets with muon - ");
    m_SV1_energyTrkInJet_muon = bookHistogram("SV1_energyTrkInJet_muon", "SV1_energyTrkInJet", m_sParticleType, "jets with muon - ");
    m_SV1_NGTinSvx_muon = bookHistogram("SV1_NGTinSvx_muon", "SV1_NGTinSvx", m_sParticleType, "jets with muon - ");
    m_SV1_Lxy_muon = bookHistogram("SV1_Lxy_muon", "SV1_Lxy", m_sParticleType, "jets with muon - ");
    m_SV1_purity_muon = bookHistogram("SV1_purity_muon", "SV1_purity", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromB_muon = bookHistogram("SV1_fracTracks_fromB_muon", "SV1_fracTracks_from_B", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromC_muon = bookHistogram("SV1_fracTracks_fromC_muon", "SV1_fracTracks_from_C", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromFragmentation_muon = bookHistogram("SV1_fracTracks_fromFragmentation_muon", "SV1_fracTracks_from_Fragmentation", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromSecondaries_muon = bookHistogram("SV1_fracTracks_fromSecondaries_muon", "SV1_fracTracks_from_Secondaries", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromPileup_muon = bookHistogram("SV1_fracTracks_fromPileup_muon", "SV1_fracTracks_from_Pileup", m_sParticleType, "jets with muon - ");
    m_SV1_fracTracks_fromFake_muon = bookHistogram("SV1_fracTracks_fromFake_muon", "SV1_fracTracks_from_Fake", m_sParticleType, "jets with muon - ");

    m_SV1_fracHFTracksInJet_incl = bookHistogram("SV1_fracHFTracksInJet_incl", "SV1_fracHFTracksInJet", m_sParticleType);
    m_SV1_fracHFTracksInJet_b = bookHistogram("SV1_fracHFTracksInJet_b", "SV1_fracHFTracksInJet", m_sParticleType, "b-jets - ");
    m_SV1_fracHFTracksInJet_c = bookHistogram("SV1_fracHFTracksInJet_c", "SV1_fracHFTracksInJet", m_sParticleType, "c-jets - ");
    m_SV1_fracHFTracksInJet_l = bookHistogram("SV1_fracHFTracksInJet_l", "SV1_fracHFTracksInJet", m_sParticleType, "l-jets - ");
    m_SV1_fracHFTracksInJet_muon = bookHistogram("SV1_fracHFTracksInJet_muon", "SV1_fracHFTracksInJet", m_sParticleType, "jets with muon - ");

    if(m_detailLevel > 10){
      m_SV1_fracTracks_Secondaries_KshortDecay_incl = bookHistogram("SV1_fracTracks_Secondaries_KshortDecay_incl", "SV1_fracTracks_Secondaries_KshortDecay", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_KshortDecay_b = bookHistogram("SV1_fracTracks_Secondaries_KshortDecay_b", "SV1_fracTracks_Secondaries_KshortDecay", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_KshortDecay_c = bookHistogram("SV1_fracTracks_Secondaries_KshortDecay_c", "SV1_fracTracks_Secondaries_KshortDecay", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_KshortDecay_u = bookHistogram("SV1_fracTracks_Secondaries_KshortDecay_l", "SV1_fracTracks_Secondaries_KshortDecay", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_KshortDecay_muon = bookHistogram("SV1_fracTracks_Secondaries_KshortDecay_muon", "SV1_fracTracks_Secondaries_KshortDecay", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_Secondaries_LambdaDecay_incl = bookHistogram("SV1_fracTracks_Secondaries_LambdaDecay_incl", "SV1_fracTracks_Secondaries_LambdaDecay", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_LambdaDecay_b = bookHistogram("SV1_fracTracks_Secondaries_LambdaDecay_b", "SV1_fracTracks_Secondaries_LambdaDecay", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_LambdaDecay_c = bookHistogram("SV1_fracTracks_Secondaries_LambdaDecay_c", "SV1_fracTracks_Secondaries_LambdaDecay", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_LambdaDecay_u = bookHistogram("SV1_fracTracks_Secondaries_LambdaDecay_l", "SV1_fracTracks_Secondaries_LambdaDecay", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_LambdaDecay_muon = bookHistogram("SV1_fracTracks_Secondaries_LambdaDecay_muon", "SV1_fracTracks_Secondaries_LambdaDecay", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_Secondaries_GammaConversion_incl = bookHistogram("SV1_fracTracks_Secondaries_GammaConversion_incl", "SV1_fracTracks_Secondaries_GammaConversion", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_GammaConversion_b = bookHistogram("SV1_fracTracks_Secondaries_GammaConversion_b", "SV1_fracTracks_Secondaries_GammaConversion", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_GammaConversion_c = bookHistogram("SV1_fracTracks_Secondaries_GammaConversion_c", "SV1_fracTracks_Secondaries_GammaConversion", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_GammaConversion_u = bookHistogram("SV1_fracTracks_Secondaries_GammaConversion_l", "SV1_fracTracks_Secondaries_GammaConversion", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_GammaConversion_muon = bookHistogram("SV1_fracTracks_Secondaries_GammaConversion_muon", "SV1_fracTracks_Secondaries_GammaConversion", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_Secondaries_OtherDecay_incl = bookHistogram("SV1_fracTracks_Secondaries_OtherDecay_incl", "SV1_fracTracks_Secondaries_OtherDecay", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_OtherDecay_b = bookHistogram("SV1_fracTracks_Secondaries_OtherDecay_b", "SV1_fracTracks_Secondaries_OtherDecay", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherDecay_c = bookHistogram("SV1_fracTracks_Secondaries_OtherDecay_c", "SV1_fracTracks_Secondaries_OtherDecay", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherDecay_u = bookHistogram("SV1_fracTracks_Secondaries_OtherDecay_l", "SV1_fracTracks_Secondaries_OtherDecay", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherDecay_muon = bookHistogram("SV1_fracTracks_Secondaries_OtherDecay_muon", "SV1_fracTracks_Secondaries_OtherDecay", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_Secondaries_HadronicInteraction_incl = bookHistogram("SV1_fracTracks_Secondaries_HadronicInteraction_incl", "SV1_fracTracks_Secondaries_HadronicInteraction", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_HadronicInteraction_b = bookHistogram("SV1_fracTracks_Secondaries_HadronicInteraction_b", "SV1_fracTracks_Secondaries_HadronicInteraction", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_HadronicInteraction_c = bookHistogram("SV1_fracTracks_Secondaries_HadronicInteraction_c", "SV1_fracTracks_Secondaries_HadronicInteraction", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_HadronicInteraction_u = bookHistogram("SV1_fracTracks_Secondaries_HadronicInteraction_l", "SV1_fracTracks_Secondaries_HadronicInteraction", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_HadronicInteraction_muon = bookHistogram("SV1_fracTracks_Secondaries_HadronicInteraction_muon", "SV1_fracTracks_Secondaries_HadronicInteraction", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_Secondaries_OtherSecondary_incl = bookHistogram("SV1_fracTracks_Secondaries_OtherSecondary_incl", "SV1_fracTracks_Secondaries_OtherSecondary", m_sParticleType); 
      m_SV1_fracTracks_Secondaries_OtherSecondary_b = bookHistogram("SV1_fracTracks_Secondaries_OtherSecondary_b", "SV1_fracTracks_Secondaries_OtherSecondary", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherSecondary_c = bookHistogram("SV1_fracTracks_Secondaries_OtherSecondary_c", "SV1_fracTracks_Secondaries_OtherSecondary", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherSecondary_u = bookHistogram("SV1_fracTracks_Secondaries_OtherSecondary_l", "SV1_fracTracks_Secondaries_OtherSecondary", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_Secondaries_OtherSecondary_muon = bookHistogram("SV1_fracTracks_Secondaries_OtherSecondary_muon", "SV1_fracTracks_Secondaries_OtherSecondary", m_sParticleType, "jets with muon -"); 

      m_SV1_fracTracks_OtherOrigin_incl = bookHistogram("SV1_fracTracks_OtherOrigin_incl", "SV1_fracTracks_from_OtherOrigin", m_sParticleType); 
      m_SV1_fracTracks_OtherOrigin_b = bookHistogram("SV1_fracTracks_OtherOrigin_b", "SV1_fracTracks_from_OtherOrigin", m_sParticleType, "b-jets -"); 
      m_SV1_fracTracks_OtherOrigin_c = bookHistogram("SV1_fracTracks_OtherOrigin_c", "SV1_fracTracks_from_OtherOrigin", m_sParticleType, "c-jets -"); 
      m_SV1_fracTracks_OtherOrigin_u = bookHistogram("SV1_fracTracks_OtherOrigin_l", "SV1_fracTracks_from_OtherOrigin", m_sParticleType, "l-jets -"); 
      m_SV1_fracTracks_OtherOrigin_muon = bookHistogram("SV1_fracTracks_OtherOrigin_muon", "SV1_fracTracks_from_OtherOrigin", m_sParticleType, "jets with muon -"); 
    }

    // IPs and IP significances
    m_track_d0_incl = bookHistogram("d0_incl", "track_d0", m_sParticleType);
    m_track_z0_incl = bookHistogram("z0_incl", "track_z0", m_sParticleType); 
    m_track_sigd0_incl = bookHistogram("sigd0_incl", "track_sigd0", m_sParticleType);
    m_track_sigz0_incl = bookHistogram("sigz0_incl", "track_sigz0", m_sParticleType); 

    m_track_d0_b = bookHistogram("d0_b", "track_d0", m_sParticleType, "b-jets -");
    m_track_z0_b = bookHistogram("z0_b", "track_z0", m_sParticleType, "b-jets -"); 
    m_track_sigd0_b = bookHistogram("sigd0_b", "track_sigd0", m_sParticleType, "b-jets -");
    m_track_sigz0_b = bookHistogram("sigz0_b", "track_sigz0", m_sParticleType, "b-jets -"); 
    
    m_track_d0_c = bookHistogram("d0_c", "track_d0", m_sParticleType, "c-jets -");
    m_track_z0_c = bookHistogram("z0_c", "track_z0", m_sParticleType, "c-jets -"); 
    m_track_sigd0_c = bookHistogram("sigd0_c", "track_sigd0", m_sParticleType, "c-jets -");
    m_track_sigz0_c = bookHistogram("sigz0_c", "track_sigz0", m_sParticleType, "c-jets -"); 
    
    m_track_d0_u = bookHistogram("d0_l", "track_d0", m_sParticleType, "l-jets -");
    m_track_z0_u = bookHistogram("z0_l", "track_z0", m_sParticleType, "l-jets -"); 
    m_track_sigd0_u = bookHistogram("sigd0_l", "track_sigd0", m_sParticleType, "l-jets -");
    m_track_sigz0_u = bookHistogram("sigz0_l", "track_sigz0", m_sParticleType, "l-jets -"); 
    
    m_track_d0_muon = bookHistogram("d0_muon", "track_d0", m_sParticleType, "jets with muon -");
    m_track_z0_muon = bookHistogram("z0_muon", "track_z0", m_sParticleType, "jets with muon -"); 
    m_track_sigd0_muon = bookHistogram("sigd0_muon", "track_sigd0", m_sParticleType, "jets with muon -");
    m_track_sigz0_muon = bookHistogram("sigz0_muon", "track_sigz0", m_sParticleType, "jets with muon -"); 

    // pT_frac
    m_track_pT_frac_incl = bookHistogram("track_pT_frac_incl", "track_pT_frac", m_sParticleType); 
    m_track_pT_frac_b = bookHistogram("track_pT_frac_b", "track_pT_frac", m_sParticleType, "b-jets -"); 
    m_track_pT_frac_c = bookHistogram("track_pT_frac_c", "track_pT_frac", m_sParticleType, "c-jets -"); 
    m_track_pT_frac_u = bookHistogram("track_pT_frac_l", "track_pT_frac", m_sParticleType, "l-jets -"); 
    m_track_pT_frac_muon = bookHistogram("track_pT_frac_muon", "track_pT_frac", m_sParticleType, "jets with muon -"); 

    // DeltaR_jet_track
    m_DeltaR_jet_track_incl = bookHistogram("DeltaR_jet_track_incl", "DeltaR_jet_track", m_sParticleType); 
    m_DeltaR_jet_track_b = bookHistogram("DeltaR_jet_track_b", "DeltaR_jet_track", m_sParticleType, "b-jets -"); 
    m_DeltaR_jet_track_c = bookHistogram("DeltaR_jet_track_c", "DeltaR_jet_track", m_sParticleType, "c-jets -"); 
    m_DeltaR_jet_track_u = bookHistogram("DeltaR_jet_track_l", "DeltaR_jet_track", m_sParticleType, "l-jets -"); 
    m_DeltaR_jet_track_muon = bookHistogram("DeltaR_jet_track_muon", "DeltaR_jet_track", m_sParticleType, "jets with muon -"); 

    // numTracks_perJet 
    m_numTracks_perJet_incl = bookHistogram("numTracks_perJet_incl", "numTracks_perJet", m_sParticleType); 
    m_numTracks_perJet_b = bookHistogram("numTracks_perJet_b", "numTracks_perJet", m_sParticleType, "b-jets -"); 
    m_numTracks_perJet_c = bookHistogram("numTracks_perJet_c", "numTracks_perJet", m_sParticleType, "c-jets -"); 
    m_numTracks_perJet_u = bookHistogram("numTracks_perJet_l", "numTracks_perJet", m_sParticleType, "l-jets -"); 
    m_numTracks_perJet_top = bookHistogram("numTracks_perJet_top", "numTracks_perJet", m_sParticleType, "top-jets -"); 
    m_numTracks_perJet_muon = bookHistogram("numTracks_perJet_muon", "numTracks_perJet", m_sParticleType, "jets with muon -"); 

    // number of tracks from different origins
    m_numTracks_B_incl = bookHistogram("numTracks_B_incl", "numTracks_B", m_sParticleType); 
    m_numTracks_C_incl = bookHistogram("numTracks_C_incl", "numTracks_C", m_sParticleType); 
    m_numTracks_Fragmentation_incl = bookHistogram("numTracks_Fragmentation_incl", "numTracks_Fragmentation", m_sParticleType); 
    m_numTracks_Secondaries_incl = bookHistogram("numTracks_Secondaries_incl", "numTracks_Secondaries", m_sParticleType); 
    m_numTracks_Pileup_incl = bookHistogram("numTracks_Pileup_incl", "numTracks_Pileup", m_sParticleType); 
    m_numTracks_Fake_incl = bookHistogram("numTracks_Fake_incl", "numTracks_Fake", m_sParticleType); 

    m_numTracks_B_b = bookHistogram("numTracks_B_b", "numTracks_B", m_sParticleType, "b-jets -"); 
    m_numTracks_C_b = bookHistogram("numTracks_C_b", "numTracks_C", m_sParticleType, "b-jets -"); 
    m_numTracks_Fragmentation_b = bookHistogram("numTracks_Fragmentation_b", "numTracks_Fragmentation", m_sParticleType, "b-jets -"); 
    m_numTracks_Secondaries_b = bookHistogram("numTracks_Secondaries_b", "numTracks_Secondaries", m_sParticleType, "b-jets -"); 
    m_numTracks_Pileup_b = bookHistogram("numTracks_Pileup_b", "numTracks_Pileup", m_sParticleType, "b-jets -"); 
    m_numTracks_Fake_b = bookHistogram("numTracks_Fake_b", "numTracks_Fake", m_sParticleType, "b-jets -"); 

    m_numTracks_B_c = bookHistogram("numTracks_B_c", "numTracks_B", m_sParticleType, "c-jets -"); 
    m_numTracks_C_c = bookHistogram("numTracks_C_c", "numTracks_C", m_sParticleType, "c-jets -"); 
    m_numTracks_Fragmentation_c = bookHistogram("numTracks_Fragmentation_c", "numTracks_Fragmentation", m_sParticleType, "c-jets -"); 
    m_numTracks_Secondaries_c = bookHistogram("numTracks_Secondaries_c", "numTracks_Secondaries", m_sParticleType, "c-jets -"); 
    m_numTracks_Pileup_c = bookHistogram("numTracks_Pileup_c", "numTracks_Pileup", m_sParticleType, "c-jets -"); 
    m_numTracks_Fake_c = bookHistogram("numTracks_Fake_c", "numTracks_Fake", m_sParticleType, "c-jets -"); 

    m_numTracks_B_u = bookHistogram("numTracks_B_l", "numTracks_B", m_sParticleType, "l-jets -"); 
    m_numTracks_C_u = bookHistogram("numTracks_C_l", "numTracks_C", m_sParticleType, "l-jets -"); 
    m_numTracks_Fragmentation_u = bookHistogram("numTracks_Fragmentation_l", "numTracks_Fragmentation", m_sParticleType, "l-jets -"); 
    m_numTracks_Secondaries_u = bookHistogram("numTracks_Secondaries_l", "numTracks_Secondaries", m_sParticleType, "l-jets -"); 
    m_numTracks_Pileup_u = bookHistogram("numTracks_Pileup_l", "numTracks_Pileup", m_sParticleType, "l-jets -"); 
    m_numTracks_Fake_u = bookHistogram("numTracks_Fake_l", "numTracks_Fake", m_sParticleType, "l-jets -"); 

    m_numTracks_B_muon = bookHistogram("numTracks_B_muon", "numTracks_B", m_sParticleType, "jets with muon -"); 
    m_numTracks_C_muon = bookHistogram("numTracks_C_muon", "numTracks_C", m_sParticleType, "jets with muon -"); 
    m_numTracks_Fragmentation_muon = bookHistogram("numTracks_Fragmentation_muon", "numTracks_Fragmentation", m_sParticleType, "jets with muon -"); 
    m_numTracks_Secondaries_muon = bookHistogram("numTracks_Secondaries_muon", "numTracks_Secondaries", m_sParticleType, "jets with muon -"); 
    m_numTracks_Pileup_muon = bookHistogram("numTracks_Pileup_muon", "numTracks_Pileup", m_sParticleType, "jets with muon -"); 
    m_numTracks_Fake_muon = bookHistogram("numTracks_Fake_muon", "numTracks_Fake", m_sParticleType, "jets with muon -"); 

    if(m_detailLevel > 10){
      m_numTracks_Secondaries_KshortDecay_incl = bookHistogram("numTracks_Secondaries_KshortDecay_incl", "numTracks_Secondaries_KshortDecay", m_sParticleType); 
      m_numTracks_Secondaries_KshortDecay_b = bookHistogram("numTracks_Secondaries_KshortDecay_b", "numTracks_Secondaries_KshortDecay", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_KshortDecay_c = bookHistogram("numTracks_Secondaries_KshortDecay_c", "numTracks_Secondaries_KshortDecay", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_KshortDecay_u = bookHistogram("numTracks_Secondaries_KshortDecay_l", "numTracks_Secondaries_KshortDecay", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_KshortDecay_muon = bookHistogram("numTracks_Secondaries_KshortDecay_muon", "numTracks_Secondaries_KshortDecay", m_sParticleType, "jets with muon -"); 

      m_numTracks_Secondaries_LambdaDecay_incl = bookHistogram("numTracks_Secondaries_LambdaDecay_incl", "numTracks_Secondaries_LambdaDecay", m_sParticleType); 
      m_numTracks_Secondaries_LambdaDecay_b = bookHistogram("numTracks_Secondaries_LambdaDecay_b", "numTracks_Secondaries_LambdaDecay", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_LambdaDecay_c = bookHistogram("numTracks_Secondaries_LambdaDecay_c", "numTracks_Secondaries_LambdaDecay", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_LambdaDecay_u = bookHistogram("numTracks_Secondaries_LambdaDecay_l", "numTracks_Secondaries_LambdaDecay", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_LambdaDecay_muon = bookHistogram("numTracks_Secondaries_LambdaDecay_muon", "numTracks_Secondaries_LambdaDecay", m_sParticleType, "jets with muon -"); 

      m_numTracks_Secondaries_GammaConversion_incl = bookHistogram("numTracks_Secondaries_GammaConversion_incl", "numTracks_Secondaries_GammaConversion", m_sParticleType); 
      m_numTracks_Secondaries_GammaConversion_b = bookHistogram("numTracks_Secondaries_GammaConversion_b", "numTracks_Secondaries_GammaConversion", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_GammaConversion_c = bookHistogram("numTracks_Secondaries_GammaConversion_c", "numTracks_Secondaries_GammaConversion", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_GammaConversion_u = bookHistogram("numTracks_Secondaries_GammaConversion_l", "numTracks_Secondaries_GammaConversion", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_GammaConversion_muon = bookHistogram("numTracks_Secondaries_GammaConversion_muon", "numTracks_Secondaries_GammaConversion", m_sParticleType, "jets with muon -"); 

      m_numTracks_Secondaries_OtherDecay_incl = bookHistogram("numTracks_Secondaries_OtherDecay_incl", "numTracks_Secondaries_OtherDecay", m_sParticleType); 
      m_numTracks_Secondaries_OtherDecay_b = bookHistogram("numTracks_Secondaries_OtherDecay_b", "numTracks_Secondaries_OtherDecay", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_OtherDecay_c = bookHistogram("numTracks_Secondaries_OtherDecay_c", "numTracks_Secondaries_OtherDecay", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_OtherDecay_u = bookHistogram("numTracks_Secondaries_OtherDecay_l", "numTracks_Secondaries_OtherDecay", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_OtherDecay_muon = bookHistogram("numTracks_Secondaries_OtherDecay_muon", "numTracks_Secondaries_OtherDecay", m_sParticleType, "jets with muon -"); 

      m_numTracks_Secondaries_HadronicInteraction_incl = bookHistogram("numTracks_Secondaries_HadronicInteraction_incl", "numTracks_Secondaries_HadronicInteraction", m_sParticleType); 
      m_numTracks_Secondaries_HadronicInteraction_b = bookHistogram("numTracks_Secondaries_HadronicInteraction_b", "numTracks_Secondaries_HadronicInteraction", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_HadronicInteraction_c = bookHistogram("numTracks_Secondaries_HadronicInteraction_c", "numTracks_Secondaries_HadronicInteraction", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_HadronicInteraction_u = bookHistogram("numTracks_Secondaries_HadronicInteraction_l", "numTracks_Secondaries_HadronicInteraction", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_HadronicInteraction_muon = bookHistogram("numTracks_Secondaries_HadronicInteraction_muon", "numTracks_Secondaries_HadronicInteraction", m_sParticleType, "jets with muon -"); 

      m_numTracks_Secondaries_OtherSecondary_incl = bookHistogram("numTracks_Secondaries_OtherSecondary_incl", "numTracks_Secondaries_OtherSecondary", m_sParticleType); 
      m_numTracks_Secondaries_OtherSecondary_b = bookHistogram("numTracks_Secondaries_OtherSecondary_b", "numTracks_Secondaries_OtherSecondary", m_sParticleType, "b-jets -"); 
      m_numTracks_Secondaries_OtherSecondary_c = bookHistogram("numTracks_Secondaries_OtherSecondary_c", "numTracks_Secondaries_OtherSecondary", m_sParticleType, "c-jets -"); 
      m_numTracks_Secondaries_OtherSecondary_u = bookHistogram("numTracks_Secondaries_OtherSecondary_l", "numTracks_Secondaries_OtherSecondary", m_sParticleType, "l-jets -"); 
      m_numTracks_Secondaries_OtherSecondary_muon = bookHistogram("numTracks_Secondaries_OtherSecondary_muon", "numTracks_Secondaries_OtherSecondary", m_sParticleType, "jets with muon -"); 

      m_numTracks_OtherOrigin_incl = bookHistogram("numTracks_OtherOrigin_incl", "numTracks_OtherOrigin", m_sParticleType); 
      m_numTracks_OtherOrigin_b = bookHistogram("numTracks_OtherOrigin_b", "numTracks_OtherOrigin", m_sParticleType, "b-jets -"); 
      m_numTracks_OtherOrigin_c = bookHistogram("numTracks_OtherOrigin_c", "numTracks_OtherOrigin", m_sParticleType, "c-jets -"); 
      m_numTracks_OtherOrigin_u = bookHistogram("numTracks_OtherOrigin_l", "numTracks_OtherOrigin", m_sParticleType, "l-jets -"); 
      m_numTracks_OtherOrigin_muon = bookHistogram("numTracks_OtherOrigin_muon", "numTracks_OtherOrigin", m_sParticleType, "jets with muon -");
    }

    // tracker hits
    m_nInnHits_incl = bookHistogram("nInnHits_incl", "nInnHits", m_sParticleType); 
    m_nNextToInnHits_incl = bookHistogram("nNextToInnHits_incl", "nNextToInnHits", m_sParticleType);
    m_nBLHits_incl = bookHistogram("nBLHits_incl", "nBLHits", m_sParticleType);
    m_nsharedBLHits_incl = bookHistogram("nsharedBLHits_incl", "nsharedBLHits", m_sParticleType);
    m_nsplitBLHits_incl = bookHistogram("nsplitBLHits_incl", "nsplitBLHits", m_sParticleType);
    m_nPixHits_incl = bookHistogram("nPixHits_incl", "nPixHits", m_sParticleType);
    m_nPixHoles_incl = bookHistogram("nPixHoles_incl", "nPixHoles", m_sParticleType);
    m_nsharedPixHits_incl = bookHistogram("nsharedPixHits_incl", "nsharedPixHits", m_sParticleType);
    m_nsplitPixHits_incl = bookHistogram("nsplitPixHits_incl", "nsplitPixHits", m_sParticleType);
    m_nSCTHits_incl = bookHistogram("nSCTHits_incl", "nSCTHits", m_sParticleType);
    m_nSCTHoles_incl = bookHistogram("nSCTHoles_incl", "nSCTHoles", m_sParticleType);
    m_nsharedSCTHits_incl = bookHistogram("nsharedSCTHits_incl", "nsharedSCTHits", m_sParticleType);

    m_nInnHits_b = bookHistogram("nInnHits_b", "nInnHits", m_sParticleType, "b-jets -"); 
    m_nNextToInnHits_b = bookHistogram("nNextToInnHits_b", "nNextToInnHits", m_sParticleType, "b-jets -");
    m_nBLHits_b = bookHistogram("nBLHits_b", "nBLHits", m_sParticleType, "b-jets -");
    m_nsharedBLHits_b = bookHistogram("nsharedBLHits_b", "nsharedBLHits", m_sParticleType, "b-jets -");
    m_nsplitBLHits_b = bookHistogram("nsplitBLHits_b", "nsplitBLHits", m_sParticleType, "b-jets -");
    m_nPixHits_b = bookHistogram("nPixHits_b", "nPixHits", m_sParticleType, "b-jets -");
    m_nPixHoles_b = bookHistogram("nPixHoles_b", "nPixHoles", m_sParticleType, "b-jets -");
    m_nsharedPixHits_b = bookHistogram("nsharedPixHits_b", "nsharedPixHits", m_sParticleType, "b-jets -");
    m_nsplitPixHits_b = bookHistogram("nsplitPixHits_b", "nsplitPixHits", m_sParticleType, "b-jets -");
    m_nSCTHits_b = bookHistogram("nSCTHits_b", "nSCTHits", m_sParticleType, "b-jets -");
    m_nSCTHoles_b = bookHistogram("nSCTHoles_b", "nSCTHoles", m_sParticleType, "b-jets -");
    m_nsharedSCTHits_b = bookHistogram("nsharedSCTHits_b", "nsharedSCTHits", m_sParticleType, "b-jets -");

    m_nInnHits_c = bookHistogram("nInnHits_c", "nInnHits", m_sParticleType, "c-jets -"); 
    m_nNextToInnHits_c = bookHistogram("nNextToInnHits_c", "nNextToInnHits", m_sParticleType, "c-jets -");
    m_nBLHits_c = bookHistogram("nBLHits_c", "nBLHits", m_sParticleType, "c-jets -");
    m_nsharedBLHits_c = bookHistogram("nsharedBLHits_c", "nsharedBLHits", m_sParticleType, "c-jets -");
    m_nsplitBLHits_c = bookHistogram("nsplitBLHits_c", "nsplitBLHits", m_sParticleType, "c-jets -");
    m_nPixHits_c = bookHistogram("nPixHits_c", "nPixHits", m_sParticleType, "c-jets -");
    m_nPixHoles_c = bookHistogram("nPixHoles_c", "nPixHoles", m_sParticleType, "c-jets -");
    m_nsharedPixHits_c = bookHistogram("nsharedPixHits_c", "nsharedPixHits", m_sParticleType, "c-jets -");
    m_nsplitPixHits_c = bookHistogram("nsplitPixHits_c", "nsplitPixHits", m_sParticleType, "c-jets -");
    m_nSCTHits_c = bookHistogram("nSCTHits_c", "nSCTHits", m_sParticleType, "c-jets -");
    m_nSCTHoles_c = bookHistogram("nSCTHoles_c", "nSCTHoles", m_sParticleType, "c-jets -");
    m_nsharedSCTHits_c = bookHistogram("nsharedSCTHits_c", "nsharedSCTHits", m_sParticleType, "c-jets -");

    m_nInnHits_u = bookHistogram("nInnHits_l", "nInnHits", m_sParticleType, "l-jets -"); 
    m_nNextToInnHits_u = bookHistogram("nNextToInnHits_l", "nNextToInnHits", m_sParticleType, "l-jets -");
    m_nBLHits_u = bookHistogram("nBLHits_l", "nBLHits", m_sParticleType, "l-jets -");
    m_nsharedBLHits_u = bookHistogram("nsharedBLHits_l", "nsharedBLHits", m_sParticleType, "l-jets -");
    m_nsplitBLHits_u = bookHistogram("nsplitBLHits_l", "nsplitBLHits", m_sParticleType, "l-jets -");
    m_nPixHits_u = bookHistogram("nPixHits_l", "nPixHits", m_sParticleType, "l-jets -");
    m_nPixHoles_u = bookHistogram("nPixHoles_l", "nPixHoles", m_sParticleType, "l-jets -");
    m_nsharedPixHits_u = bookHistogram("nsharedPixHits_l", "nsharedPixHits", m_sParticleType, "l-jets -");
    m_nsplitPixHits_u = bookHistogram("nsplitPixHits_l", "nsplitPixHits", m_sParticleType, "l-jets -");
    m_nSCTHits_u = bookHistogram("nSCTHits_l", "nSCTHits", m_sParticleType, "l-jets -");
    m_nSCTHoles_u = bookHistogram("nSCTHoles_l", "nSCTHoles", m_sParticleType, "l-jets -");
    m_nsharedSCTHits_u = bookHistogram("nsharedSCTHits_l", "nsharedSCTHits", m_sParticleType, "l-jets -");

    m_nInnHits_muon = bookHistogram("nInnHits_muon", "nInnHits", m_sParticleType, "jets with muon -"); 
    m_nNextToInnHits_muon = bookHistogram("nNextToInnHits_muon", "nNextToInnHits", m_sParticleType, "jets with muon -");
    m_nBLHits_muon = bookHistogram("nBLHits_muon", "nBLHits", m_sParticleType, "jets with muon -");
    m_nsharedBLHits_muon = bookHistogram("nsharedBLHits_muon", "nsharedBLHits", m_sParticleType, "jets with muon -");
    m_nsplitBLHits_muon = bookHistogram("nsplitBLHits_muon", "nsplitBLHits", m_sParticleType, "jets with muon -");
    m_nPixHits_muon = bookHistogram("nPixHits_muon", "nPixHits", m_sParticleType, "jets with muon -");
    m_nPixHoles_muon = bookHistogram("nPixHoles_muon", "nPixHoles", m_sParticleType, "jets with muon -");
    m_nsharedPixHits_muon = bookHistogram("nsharedPixHits_muon", "nsharedPixHits", m_sParticleType, "jets with muon -");
    m_nsplitPixHits_muon = bookHistogram("nsplitPixHits_muon", "nsplitPixHits", m_sParticleType, "jets with muon -");
    m_nSCTHits_muon = bookHistogram("nSCTHits_muon", "nSCTHits", m_sParticleType, "jets with muon -");
    m_nSCTHoles_muon = bookHistogram("nSCTHoles_muon", "nSCTHoles", m_sParticleType, "jets with muon -");
    m_nsharedSCTHits_muon = bookHistogram("nsharedSCTHits_muon", "nsharedSCTHits", m_sParticleType, "jets with muon -");
    //features largeRjet tagger
    m_track_d0_top = bookHistogram("d0_top", "track_d0", m_sParticleType, "top-like jets -");
    m_track_z0_top = bookHistogram("z0_top", "track_z0", m_sParticleType, "top-like jets -"); 
    m_track_sigd0_top = bookHistogram("sigd0_top", "track_sigd0", m_sParticleType, "top-like jets -");
    m_track_sigz0_top = bookHistogram("sigz0_top", "track_sigz0", m_sParticleType, "top-like jets -"); 
    // pT_frac
    m_track_pT_frac_top = bookHistogram("track_pT_frac_top", "track_pT_frac", m_sParticleType, "top-like jets -"); 
    // DeltaR_jet_track
    m_DeltaR_jet_track_top = bookHistogram("DeltaR_jet_track_top", "DeltaR_jet_track", m_sParticleType, "top-like jets -"); 
    // tracker hits
    m_nInnHits_top = bookHistogram("nInnHits_top", "nInnHits", m_sParticleType, "top-like jets -"); 
    m_nNextToInnHits_top = bookHistogram("nNextToInnHits_top", "nNextToInnHits", m_sParticleType, "top-like jets -");
    m_nBLHits_top = bookHistogram("nBLHits_top", "nBLHits", m_sParticleType, "top-like jets -");
    m_nsharedBLHits_top = bookHistogram("nsharedBLHits_top", "nsharedBLHits", m_sParticleType, "top-like jets -");
    m_nsplitBLHits_top = bookHistogram("nsplitBLHits_top", "nsplitBLHits", m_sParticleType, "top-like jets -");
    m_nPixHits_top = bookHistogram("nPixHits_top", "nPixHits", m_sParticleType, "top-like jets -");
    m_nPixHoles_top = bookHistogram("nPixHoles_top", "nPixHoles", m_sParticleType, "top-like jets -");
    m_nsharedPixHits_top = bookHistogram("nsharedPixHits_top", "nsharedPixHits", m_sParticleType, "top-like jets -");
    m_nsplitPixHits_top = bookHistogram("nsplitPixHits_top", "nsplitPixHits", m_sParticleType, "top-like jets -");
    m_nSCTHits_top = bookHistogram("nSCTHits_top", "nSCTHits", m_sParticleType, "top-like jets -");
    m_nSCTHoles_top = bookHistogram("nSCTHoles_top", "nSCTHoles", m_sParticleType, "top-like jets -");
    m_nsharedSCTHits_top = bookHistogram("nsharedSCTHits_top", "nsharedSCTHits", m_sParticleType, "top-like jets -");

    // tagger
    m_GN2v01_pb = bookHistogram("GN2v01_pb", "GN2v01_pb", m_sParticleType);
    m_GN2v01_pc = bookHistogram("GN2v01_pc", "GN2v01_pc", m_sParticleType);
    m_GN2v01_pu = bookHistogram("GN2v01_pu", "GN2v01_pu", m_sParticleType);
    m_GN2v01_ptau = bookHistogram("GN2v01_ptau", "GN2v01_ptau", m_sParticleType);
    
    m_GN3XPV01_phtautauhad = bookHistogram("GN3XPV01_phtautauhad", "GN3XPV01_phtautauhad",m_sParticleType);
    m_GN3XPV01_phbb = bookHistogram("GN3XPV01_phbb", "GN3XPV01_phbb",m_sParticleType);
    m_GN3XPV01_phcc = bookHistogram("GN3XPV01_phcc", "GN3XPV01_phcc",m_sParticleType);
    m_GN3XPV01_ptop = bookHistogram("GN3XPV01_ptop", "GN3XPV01_ptop",m_sParticleType);
    m_GN3XPV01_pqcdbb = bookHistogram("GN3XPV01_pqcdbb", "GN3XPV01_pqcdbb",m_sParticleType);
    m_GN3XPV01_pqcdbx = bookHistogram("GN3XPV01_pqcdbx", "GN3XPV01_pqcdbx",m_sParticleType);
    m_GN3XPV01_pqcdcx = bookHistogram("GN3XPV01_pqcdcx", "GN3XPV01_pqcdcx",m_sParticleType);
    m_GN3XPV01_pqcdll = bookHistogram("GN3XPV01_pqcdll", "GN3XPV01_pqcdll",m_sParticleType);
    m_GN3XPV01_pwqq = bookHistogram("GN3XPV01_pwqq", "GN3XPV01_pwqq",m_sParticleType);


    // B hadron Lxy
    m_Truth_Lxy_b = bookHistogram("Truth_Lxy_b", "Truth_Lxy_b", m_sParticleType, "b-jets - ");
    m_Truth_Lxy_c = bookHistogram("Truth_Lxy_c", "Truth_Lxy_c", m_sParticleType, "c-jets - ");

    // B hadron deltaR wrt jet 
    m_deltaR_truthBHadron_jet_b = bookHistogram("deltaR_truthBHadronJet_b", "deltaR_truthBHadronJet_b", m_sParticleType, "b-jets - ");
    m_deltaR_truthCHadron_jet_c = bookHistogram("deltaR_truthCHadronJet_c", "deltaR_truthCHadronJet_c", m_sParticleType, "c-jets - ");

  }


  // a fill method for the multiplicities
  void BTaggingValidationPlots::fillMultiplicities(const unsigned int& nJets, const unsigned int& nTracks, const int& nPrimVtx, const unsigned int& nTracksPrimVtx, const unsigned int& nJetsWithMuon, const unsigned int& nJetsWithSV, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event){
    m_nJets->Fill(nJets, event->beamSpotWeight());
    m_nTracks->Fill(nTracks, event->beamSpotWeight());
    m_nPrimVtx->Fill(nPrimVtx, event->beamSpotWeight());
    m_nTracksPrimVtx->Fill(nTracksPrimVtx, event->beamSpotWeight());
    m_nJetsWithMuon->Fill(nJetsWithMuon, event->beamSpotWeight());
    m_nJetsWithSV->Fill(nJetsWithSV, event->beamSpotWeight());

    double fracJetsWithMuon = static_cast<double>(nJetsWithMuon) / nJets;
    double fracJetsWithSV = static_cast<double>(nJetsWithSV) / nJets;
    m_fracJetsWithMuon->Fill(fracJetsWithMuon, event->beamSpotWeight());
    m_fracJetsWithSV->Fill(fracJetsWithSV, event->beamSpotWeight());

    fillNJetsThatPassedWPCutsHistos(nJetsThatPassedWPCuts, event);
  }

  // a fill method for the PV variables
  void BTaggingValidationPlots::fillPVVariables(const double& PV_x, const double& PV_y, const double& PV_z, const xAOD::EventInfo* event){
    m_PV_x->Fill(PV_x, event->beamSpotWeight());
    m_PV_y->Fill(PV_y, event->beamSpotWeight());
    m_PV_z->Fill(PV_z, event->beamSpotWeight());
  }

  // a fill method for the jet kinematic vars
  void BTaggingValidationPlots::fillJetKinVars(const xAOD::Jet* jet, const int& truth_label, const bool& onZprime, const xAOD::EventInfo* event){
    TH1* dummy = nullptr;
    // jet kin vars
    // pT
    if(onZprime){
      BTaggingValidationPlots::fillHistoWithTruthCases(jet->pt()/GeV, m_jet_pt_Zprime, m_jet_pt_Zprime_b, m_jet_pt_Zprime_c, m_jet_pt_Zprime_l, dummy, truth_label, false, event);
      m_jet_e_Zprime->Fill(jet->e()/GeV, event->beamSpotWeight());
    } else {
      BTaggingValidationPlots::fillHistoWithTruthCases(jet->pt()/GeV, m_jet_pt, m_jet_pt_b, m_jet_pt_c, m_jet_pt_l, dummy, truth_label, false, event);
      m_jet_e->Fill(jet->e()/GeV, event->beamSpotWeight());
    }
    // eta and phi
    BTaggingValidationPlots::fillHistoWithTruthCases(jet->eta(), m_jet_eta, m_jet_eta_b, m_jet_eta_c, m_jet_eta_l, dummy, truth_label, false, event);
    m_jet_phi->Fill(jet->phi(), event->beamSpotWeight());

    // fill the truth label
    m_truthLabel->Fill(truth_label, event->beamSpotWeight());
  }

  // a fill method for variables that have no other home
  void BTaggingValidationPlots::fillOther(const xAOD::Jet* jet, bool& contains_muon, double& jet_Lxy, const int& truth_label, const xAOD::EventInfo* event){

    // Lxy: take value of B hadron for b jets, of C hadron for c jets and the SV1 Lxy value for light jets
    double Lxy(-1);
    if(!m_isData){

      // Lxy of B hadron for b jets
      if(truth_label == 5){
        // get the B hadrons
        std::vector<const xAOD::TruthParticle*> BHadrons;
        const std::string labelB = "ConeExclBHadronsFinal"; // = "GhostBHadronsFinal"; // alternative association scheme
        jet->getAssociatedObjects<xAOD::TruthParticle>(labelB, BHadrons);
        // get the Lxy
        if(BHadrons.size() >= 1){
          // check if the BHadron is not a null pointer
          if(! BHadrons[0]){
            ATH_MSG_WARNING("A BHadron in the 'ConeExclBHadronsFinal' collection is a null pointer. Might be related to ATLPHYSVAL-783.");
          } 
          else{
            // get the Lxy
            Lxy = BHadrons[0]->decayVtx()->perp();
            m_Truth_Lxy_b ->Fill(Lxy, event->beamSpotWeight());
            // get the deltaR wrt to the jet axis
            double dR = jet->p4().DeltaR(BHadrons[0]->p4());
            m_deltaR_truthBHadron_jet_b->Fill(dR, event->beamSpotWeight());
          }
        }
      }
  
      // Lxy of C hadron for c jets
      else if(truth_label == 4){
        // get the C hadrons
        std::vector<const xAOD::TruthParticle*> CHadrons;
        const std::string labelC = "ConeExclCHadronsFinal"; // = "GhostCHadronsFinal"; // alternative association scheme
        jet->getAssociatedObjects<xAOD::TruthParticle>(labelC, CHadrons);
        // get the Lxy
        if(CHadrons.size() >= 1){
          // check if the CHadron is not a null pointer
          if(! CHadrons[0]){
            ATH_MSG_WARNING("A CHadron in the 'ConeExclCHadronsFinal' collection is a null pointer. Might be related to ATLPHYSVAL-783.");
          } 
          else{
            // get the Lxy
            Lxy = CHadrons[0]->decayVtx()->perp();
            m_Truth_Lxy_c ->Fill(Lxy, event->beamSpotWeight());
            // get the deltaR wrt to the jet axis
            double dR = jet->p4().DeltaR(CHadrons[0]->p4());
            m_deltaR_truthCHadron_jet_c->Fill(dR, event->beamSpotWeight());
          }
        }
      }

      // SV1 Lxy for light jets
      else if(truth_label == 0){
        static const SG::ConstAccessor<float> SV1_LxyAcc("SV1_Lxy");
        if (SV1_LxyAcc.isAvailable(*jet)) Lxy = SV1_LxyAcc(*jet);
      }
    }

    // fill jet Lxy for the caller of the method
    jet_Lxy = Lxy;

    // check if there is a muon and store the relative muon pT
    bool has_muon = false;
    
    // get the muon link
    ElementLink<xAOD::MuonContainer> muonLink;
    static const SG::ConstAccessor< ElementLink<xAOD::MuonContainer> >
      softMuon_linkAcc("softMuon_link");
    if(softMuon_linkAcc.isAvailable(*jet)) {
      muonLink = softMuon_linkAcc(*jet); 
    }

    // fill bool and pT frac if there is a muon
    if ( muonLink.isValid() ) {
        const xAOD::Muon* muon=(*muonLink);
        if ( muon != 0 ) {
            has_muon = true;
            double muon_pT_frac = muon->pt() / jet->pt();
            m_muon_pT_frac->Fill(muon_pT_frac, event->beamSpotWeight());
        }
    }
    
    // fill contains_muon for the caller of this function
    contains_muon = has_muon;
  }


  // a fill method for track related vars
  void BTaggingValidationPlots::fillTrackVariables(const xAOD::Jet* jet, const xAOD::Vertex *myVertex, const std::map<const xAOD::TrackParticle*, int> & track_truth_associations, const bool& has_muon, const int& truth_label, int& num_HF_tracks_in_jet, const xAOD::EventInfo* event){

    // get the jet TLorentzVector
    TLorentzVector jet_tlv;
    jet_tlv.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());

    // get the assocated tracks
    static const SG::ConstAccessor< std::vector<ElementLink<xAOD::IParticleContainer> > >
      JetToTrackAssociatorAcc("TracksForBTagging");
    std::vector< ElementLink< xAOD::IParticleContainer > > assocTracks =
      JetToTrackAssociatorAcc(*jet);

    int numTracks_perJet = 0;
    int numTracks_fromB = 0;
    int numTracks_fromC = 0;
    int numTracks_fromFragmentation = 0;
    int numTracks_fromSecondaries = 0;
    int numTracks_fromPileup = 0;
    int numTracks_fromFake = 0;

    // these are only filled for detail level above 10
    int numTracks_Secondaries_KshortDecay = 0; 
    int numTracks_Secondaries_LambdaDecay = 0; 
    int numTracks_Secondaries_GammaConversion = 0; 
    int numTracks_Secondaries_OtherDecay = 0; 
    int numTracks_Secondaries_HadronicInteraction = 0; 
    int numTracks_Secondaries_OtherSecondary = 0; 
    int numTracks_OtherOrigin = 0; 

    // loop over tracks
    //for (unsigned int iT=0; iT<assocTracks.size(); iT++) {
    for (auto& assocTrack : assocTracks) {
      if (!assocTrack.isValid()) continue;

      // count the number of tracks per jet
      numTracks_perJet++;

      // get the curent track
      const xAOD::TrackParticle* track = static_cast<const xAOD::TrackParticle*>(*(assocTrack));       

      // get the track origin
      int track_origin = -1;
      if(!m_isData){
        track_origin = track_truth_associations.at(track);
      }

      // count the tracks sorted by origin (the TrkOrigin enum is defined here: https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/TrackingID/InDetTrackSystematicsTools/InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h#L14)
      if( InDet::TrkOrigin::isFromB(track_origin) ) numTracks_fromB++;
      if( InDet::TrkOrigin::isFromD(track_origin) ) numTracks_fromC++;
      if( InDet::TrkOrigin::isFragmentation(track_origin) ) numTracks_fromFragmentation++;
      if( InDet::TrkOrigin::isSecondary(track_origin) ) numTracks_fromSecondaries++;
      if( InDet::TrkOrigin::isPileup(track_origin) ) numTracks_fromPileup++;
      if( InDet::TrkOrigin::isFake(track_origin) ) numTracks_fromFake++;

      if(m_detailLevel > 10){
        if( track_origin & (0x1 << InDet::TrkOrigin::KshortDecay) ) numTracks_Secondaries_KshortDecay++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::LambdaDecay) ) numTracks_Secondaries_LambdaDecay++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::GammaConversion) ) numTracks_Secondaries_GammaConversion++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::OtherDecay) ) numTracks_Secondaries_OtherDecay++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::HadronicInteraction) ) numTracks_Secondaries_HadronicInteraction++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::OtherSecondary) ) numTracks_Secondaries_OtherSecondary++; 
        if( track_origin & (0x1 << InDet::TrkOrigin::OtherOrigin) ) numTracks_OtherOrigin++; 
      }

      // get the track also as TLorentzVector
      TLorentzVector track_tlv;
      track_tlv.SetPtEtaPhiE(track->pt(), track->eta(), track->phi(), track->e());

      // get the DeltaR between the track and the jet
      double DeltaR_jet_track = jet_tlv.DeltaR(track_tlv);

      // get the IPs and IP significances
      double d0(1000), z0(1000), sigma_d0(1000), sigma_z0(1000);                
      d0 = track->d0();
      z0 = track->z0() + track->vz() - myVertex->z(); //ANDREA: track->z0() is defined wrt the beam spot-> Get it wrt the PV
      //z0 = track->z0(); //Naive z0 wrt the beam spot
      sigma_d0 = sqrt(track->definingParametersCovMatrixVec().at(0)); 
      sigma_z0 = sqrt(track->definingParametersCovMatrixVec().at(2));

      // calculate pT_frac
      double pT_frac(2), pT_jet(-1), pT_track(-1);
      pT_jet = jet->pt();
      pT_track = track->pt();
      pT_frac = pT_track / pT_jet;

      // get the tracker hits, following this approach: https://gitlab.cern.ch/atlas-flavor-tagging-tools/FlavourTagPerformanceFramework/-/blob/50e0d05c4d855935ebac2bf07622cccf709eea18/btagAnalysis/src/TrackBranches.cxx#L331
      int nInnHits = getTrackHits(*track, xAOD::numberOfInnermostPixelLayerHits);
      int nNextToInnHits = getTrackHits(*track, xAOD::numberOfNextToInnermostPixelLayerHits);
      int nBLHits = getTrackHits(*track, xAOD::numberOfBLayerHits);
      int nsharedBLHits = getTrackHits(*track, xAOD::numberOfBLayerSharedHits);
      int nsplitBLHits = getTrackHits(*track, xAOD::numberOfBLayerSplitHits);
      int nPixHits = getTrackHits(*track, xAOD::numberOfPixelHits);
      int nPixHoles = getTrackHits(*track, xAOD::numberOfPixelHoles);
      int nsharedPixHits = getTrackHits(*track, xAOD::numberOfPixelSharedHits);
      int nsplitPixHits = getTrackHits(*track, xAOD::numberOfPixelSplitHits);
      int nSCTHits = getTrackHits(*track, xAOD::numberOfSCTHits);
      int nSCTHoles = getTrackHits(*track, xAOD::numberOfSCTHoles);
      int nsharedSCTHits = getTrackHits(*track, xAOD::numberOfSCTSharedHits);

      // fill the histogram
      BTaggingValidationPlots::fillHistoWithTruthCases(d0, m_track_d0_incl, m_track_d0_b, m_track_d0_c, m_track_d0_u, m_track_d0_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(z0, m_track_z0_incl, m_track_z0_b, m_track_z0_c, m_track_z0_u, m_track_z0_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(d0/sigma_d0, m_track_sigd0_incl, m_track_sigd0_b, m_track_sigd0_c, m_track_sigd0_u, m_track_sigd0_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(z0/sigma_z0, m_track_sigz0_incl, m_track_sigz0_b, m_track_sigz0_c, m_track_sigz0_u, m_track_sigz0_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(pT_frac, m_track_pT_frac_incl, m_track_pT_frac_b, m_track_pT_frac_c, m_track_pT_frac_u, m_track_pT_frac_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(DeltaR_jet_track, m_DeltaR_jet_track_incl, m_DeltaR_jet_track_b, m_DeltaR_jet_track_c, m_DeltaR_jet_track_u, m_DeltaR_jet_track_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nInnHits, m_nInnHits_incl, m_nInnHits_b, m_nInnHits_c, m_nInnHits_u, m_nInnHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nNextToInnHits, m_nNextToInnHits_incl, m_nNextToInnHits_b, m_nNextToInnHits_c, m_nNextToInnHits_u, m_nNextToInnHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nBLHits, m_nBLHits_incl, m_nBLHits_b, m_nBLHits_c, m_nBLHits_u, m_nBLHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nsharedBLHits, m_nsharedBLHits_incl, m_nsharedBLHits_b, m_nsharedBLHits_c, m_nsharedBLHits_u, m_nsharedBLHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nsplitBLHits, m_nsplitBLHits_incl, m_nsplitBLHits_b, m_nsplitBLHits_c, m_nsplitBLHits_u, m_nsplitBLHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nPixHits, m_nPixHits_incl, m_nPixHits_b, m_nPixHits_c, m_nPixHits_u, m_nPixHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nPixHoles, m_nPixHoles_incl, m_nPixHoles_b, m_nPixHoles_c, m_nPixHoles_u, m_nPixHoles_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nsharedPixHits, m_nsharedPixHits_incl, m_nsharedPixHits_b, m_nsharedPixHits_c, m_nsharedPixHits_u, m_nsharedPixHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nsplitPixHits, m_nsplitPixHits_incl, m_nsplitPixHits_b, m_nsplitPixHits_c, m_nsplitPixHits_u, m_nsplitPixHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nSCTHits, m_nSCTHits_incl, m_nSCTHits_b, m_nSCTHits_c, m_nSCTHits_u, m_nSCTHits_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nSCTHoles, m_nSCTHoles_incl, m_nSCTHoles_b, m_nSCTHoles_c, m_nSCTHoles_u, m_nSCTHoles_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(nsharedSCTHits, m_nsharedSCTHits_incl, m_nsharedSCTHits_b, m_nsharedSCTHits_c, m_nsharedSCTHits_u, m_nsharedSCTHits_muon, truth_label, has_muon, event);

    }    // end loop over tracks

    // store the number of tracks var
    BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_perJet, m_numTracks_perJet_incl, m_numTracks_perJet_b, m_numTracks_perJet_c, m_numTracks_perJet_u, m_numTracks_perJet_muon, truth_label, has_muon, event);

    if(!m_isData){
      num_HF_tracks_in_jet = numTracks_fromB + numTracks_fromC;
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromB, m_numTracks_B_incl, m_numTracks_B_b, m_numTracks_B_c, m_numTracks_B_u, m_numTracks_B_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromC, m_numTracks_C_incl, m_numTracks_C_b, m_numTracks_C_c, m_numTracks_C_u, m_numTracks_C_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromFragmentation, m_numTracks_Fragmentation_incl, m_numTracks_Fragmentation_b, m_numTracks_Fragmentation_c, m_numTracks_Fragmentation_u, m_numTracks_Fragmentation_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromSecondaries, m_numTracks_Secondaries_incl, m_numTracks_Secondaries_b, m_numTracks_Secondaries_c, m_numTracks_Secondaries_u, m_numTracks_Secondaries_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromPileup, m_numTracks_Pileup_incl, m_numTracks_Pileup_b, m_numTracks_Pileup_c, m_numTracks_Pileup_u, m_numTracks_Pileup_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_fromFake, m_numTracks_Fake_incl, m_numTracks_Fake_b, m_numTracks_Fake_c, m_numTracks_Fake_u, m_numTracks_Fake_muon, truth_label, has_muon, event);

      if(m_detailLevel > 10){
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_KshortDecay, m_numTracks_Secondaries_KshortDecay_incl, m_numTracks_Secondaries_KshortDecay_b, m_numTracks_Secondaries_KshortDecay_c, m_numTracks_Secondaries_KshortDecay_u, m_numTracks_Secondaries_KshortDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_LambdaDecay, m_numTracks_Secondaries_LambdaDecay_incl, m_numTracks_Secondaries_LambdaDecay_b, m_numTracks_Secondaries_LambdaDecay_c, m_numTracks_Secondaries_LambdaDecay_u, m_numTracks_Secondaries_LambdaDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_GammaConversion, m_numTracks_Secondaries_GammaConversion_incl, m_numTracks_Secondaries_GammaConversion_b, m_numTracks_Secondaries_GammaConversion_c, m_numTracks_Secondaries_GammaConversion_u, m_numTracks_Secondaries_GammaConversion_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_OtherDecay, m_numTracks_Secondaries_OtherDecay_incl, m_numTracks_Secondaries_OtherDecay_b, m_numTracks_Secondaries_OtherDecay_c, m_numTracks_Secondaries_OtherDecay_u, m_numTracks_Secondaries_OtherDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_HadronicInteraction, m_numTracks_Secondaries_HadronicInteraction_incl, m_numTracks_Secondaries_HadronicInteraction_b, m_numTracks_Secondaries_HadronicInteraction_c, m_numTracks_Secondaries_HadronicInteraction_u, m_numTracks_Secondaries_HadronicInteraction_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_Secondaries_OtherSecondary, m_numTracks_Secondaries_OtherSecondary_incl, m_numTracks_Secondaries_OtherSecondary_b, m_numTracks_Secondaries_OtherSecondary_c, m_numTracks_Secondaries_OtherSecondary_u, m_numTracks_Secondaries_OtherSecondary_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(numTracks_OtherOrigin, m_numTracks_OtherOrigin_incl, m_numTracks_OtherOrigin_b, m_numTracks_OtherOrigin_c, m_numTracks_OtherOrigin_u, m_numTracks_OtherOrigin_muon, truth_label, has_muon, event); 
      }
    }

  }

  // a fill method for track related vars
  void BTaggingValidationPlots::fillTrackVariables_for_largeRjet(const xAOD::Jet* jet, const xAOD::Vertex *myVertex, const int& truth_label, const xAOD::EventInfo* event){

    // get the jet TLorentzVector
    TLorentzVector jet_tlv;
    jet_tlv.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());

    // get the assocated tracks
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer> > >
      GhostTrackAcc("GhostTrack");
    std::vector< ElementLink< xAOD::IParticleContainer > > ghostTracks =
      GhostTrackAcc(*jet);

    int numTracks_perJet = 0;

    // loop over tracks
    for (unsigned int iT=0; iT<ghostTracks.size(); iT++) {
      if (!ghostTracks.at(iT).isValid()) continue;

      // count the number of tracks per jet
      numTracks_perJet++;

      // get the curent track
      const xAOD::TrackParticle* track = dynamic_cast<const xAOD::TrackParticle*>(*(ghostTracks.at(iT)));         
      
      // get the track also as TLorentzVector
      TLorentzVector track_tlv;
      track_tlv.SetPtEtaPhiE(track->pt(), track->eta(), track->phi(), track->e());

      // get the DeltaR between the track and the jet
      double DeltaR_jet_track = jet_tlv.DeltaR(track_tlv);

      // get the IPs and IP significances
      double d0(1000), z0(1000), sigma_d0(1000), sigma_z0(1000);                
      d0 = track->d0();
      z0 = track->z0() + track->vz() - myVertex->z(); //ANDREA: track->z0() is defined wrt the beam spot-> Get it wrt the PV
      //z0 = track->z0(); //Naive z0 wrt the beam spot
      sigma_d0 = sqrt(track->definingParametersCovMatrixVec().at(0)); 
      sigma_z0 = sqrt(track->definingParametersCovMatrixVec().at(2));
      // calculate pT_frac
      double pT_frac(2), pT_jet(-1), pT_track(-1);
      pT_jet = jet->pt();
      pT_track = track->pt();
      pT_frac = pT_track / pT_jet;

      // get the tracker hits, following this approach: https://gitlab.cern.ch/atlas-flavor-tagging-tools/FlavourTagPerformanceFramework/-/blob/50e0d05c4d855935ebac2bf07622cccf709eea18/btagAnalysis/src/TrackBranches.cxx#L331
      int nInnHits = getTrackHits(*track, xAOD::numberOfInnermostPixelLayerHits);
      int nNextToInnHits = getTrackHits(*track, xAOD::numberOfNextToInnermostPixelLayerHits);
      int nBLHits = getTrackHits(*track, xAOD::numberOfBLayerHits);
      int nsharedBLHits = getTrackHits(*track, xAOD::numberOfBLayerSharedHits);
      int nsplitBLHits = getTrackHits(*track, xAOD::numberOfBLayerSplitHits);
      int nPixHits = getTrackHits(*track, xAOD::numberOfPixelHits);
      int nPixHoles = getTrackHits(*track, xAOD::numberOfPixelHoles);
      int nsharedPixHits = getTrackHits(*track, xAOD::numberOfPixelSharedHits);
      int nsplitPixHits = getTrackHits(*track, xAOD::numberOfPixelSplitHits);
      int nSCTHits = getTrackHits(*track, xAOD::numberOfSCTHits);
      int nSCTHoles = getTrackHits(*track, xAOD::numberOfSCTHoles);
      int nsharedSCTHits = getTrackHits(*track, xAOD::numberOfSCTSharedHits);

      // fill the histogram
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(d0, m_track_d0_incl, m_track_d0_b, m_track_d0_c, m_track_d0_u, m_track_d0_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(z0, m_track_z0_incl, m_track_z0_b, m_track_z0_c, m_track_z0_u, m_track_z0_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(d0/sigma_d0, m_track_sigd0_incl, m_track_sigd0_b, m_track_sigd0_c, m_track_sigd0_u, m_track_sigd0_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(z0/sigma_z0, m_track_sigz0_incl, m_track_sigz0_b, m_track_sigz0_c, m_track_sigz0_u, m_track_sigz0_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(pT_frac, m_track_pT_frac_incl, m_track_pT_frac_b, m_track_pT_frac_c, m_track_pT_frac_u, m_track_pT_frac_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(DeltaR_jet_track, m_DeltaR_jet_track_incl, m_DeltaR_jet_track_b, m_DeltaR_jet_track_c, m_DeltaR_jet_track_u, m_DeltaR_jet_track_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nInnHits, m_nInnHits_incl, m_nInnHits_b, m_nInnHits_c, m_nInnHits_u, m_nInnHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nNextToInnHits, m_nNextToInnHits_incl, m_nNextToInnHits_b, m_nNextToInnHits_c, m_nNextToInnHits_u, m_nNextToInnHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nBLHits, m_nBLHits_incl, m_nBLHits_b, m_nBLHits_c, m_nBLHits_u, m_nBLHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nsharedBLHits, m_nsharedBLHits_incl, m_nsharedBLHits_b, m_nsharedBLHits_c, m_nsharedBLHits_u, m_nsharedBLHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nsplitBLHits, m_nsplitBLHits_incl, m_nsplitBLHits_b, m_nsplitBLHits_c, m_nsplitBLHits_u, m_nsplitBLHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nPixHits, m_nPixHits_incl, m_nPixHits_b, m_nPixHits_c, m_nPixHits_u, m_nPixHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nPixHoles, m_nPixHoles_incl, m_nPixHoles_b, m_nPixHoles_c, m_nPixHoles_u, m_nPixHoles_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nsharedPixHits, m_nsharedPixHits_incl, m_nsharedPixHits_b, m_nsharedPixHits_c, m_nsharedPixHits_u, m_nsharedPixHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nsplitPixHits, m_nsplitPixHits_incl, m_nsplitPixHits_b, m_nsplitPixHits_c, m_nsplitPixHits_u, m_nsplitPixHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nSCTHits, m_nSCTHits_incl, m_nSCTHits_b, m_nSCTHits_c, m_nSCTHits_u, m_nSCTHits_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nSCTHoles, m_nSCTHoles_incl, m_nSCTHoles_b, m_nSCTHoles_c, m_nSCTHoles_u, m_nSCTHoles_top, truth_label, event);
      BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(nsharedSCTHits, m_nsharedSCTHits_incl, m_nsharedSCTHits_b, m_nsharedSCTHits_c, m_nsharedSCTHits_u, m_nsharedSCTHits_top, truth_label, event);

    }    // end loop over tracks

    // store the number of tracks var
    BTaggingValidationPlots::fillHistoWithTruthCases_for_largeRjet(numTracks_perJet, m_numTracks_perJet_incl, m_numTracks_perJet_b, m_numTracks_perJet_c, m_numTracks_perJet_u, m_numTracks_perJet_top, truth_label, event);

  }


  // a fill method for SV related vars
  void BTaggingValidationPlots::fillSVVariables(const xAOD::Jet* jet, const std::map<const xAOD::TrackParticle*, int> & track_truth_associations, const bool& has_muon, const int& truth_label, const int& num_HF_tracks_in_jet, bool& contains_SV, const xAOD::EventInfo* event){
    // SV1

    // SV1 mass of the SV
    static const SG::ConstAccessor<float> SV1_masssvxAcc("SV1_masssvx");
    float SV1_masssvx = SV1_masssvxAcc.isAvailable(*jet) ? SV1_masssvxAcc(*jet) : -1;

    // SV1 number of two-track vertices
    static const SG::ConstAccessor<int> SV1_N2TpairAcc("SV1_N2Tpair");
    int SV1_N2Tpair = SV1_N2TpairAcc.isAvailable(*jet) ? SV1_N2TpairAcc(*jet) : -1;

    // SV1 energy fraction
    static const SG::ConstAccessor<float> SV1_efracsvxAcc("SV1_efracsvx");
    float SV1_efracsvx = SV1_efracsvxAcc.isAvailable(*jet) ? SV1_efracsvxAcc(*jet) : -1;

    // SV1 deltaR jex axis - PV-SV
    static const SG::ConstAccessor<float> SV1_deltaRAcc("SV1_deltaR");
    float SV1_deltaR = SV1_deltaRAcc(*jet);

    // SV1 significance 3d
    static const SG::ConstAccessor<float> SV1_significance3dAcc("SV1_significance3d");
    float SV1_significance3d = SV1_significance3dAcc(*jet);

    // SV1 energyTrkInJet
    static const SG::ConstAccessor<float> SV1_energyTrkInJetAcc("SV1_energyTrkInJet");
    float SV1_energyTrkInJet = SV1_energyTrkInJetAcc(*jet);

    // SV1 NGTinSvx 
    static const SG::ConstAccessor<int> SV1_NGTinSvxAcc("SV1_NGTinSvx");
    int SV1_NGTinSvx = SV1_NGTinSvxAcc(*jet);
    // SV1 Lxy 
    static const SG::ConstAccessor<float> SV1_LxyAcc("SV1_Lxy");
    float SV1_Lxy = SV1_LxyAcc(*jet);

    // SV1 track origing related variables (have them double such that taking a fraction doesn't result in an int)
    double SV1_numTracks = 0;
    double SV1_numTracks_fromB = 0;
    double SV1_numTracks_fromC = 0;
    double SV1_numTracks_fromFragmentation = 0;
    double SV1_numTracks_fromSecondaries = 0;
    double SV1_numTracks_fromPileup = 0;
    double SV1_numTracks_fromFake = 0;

    // these are only filled for detail level above 10
    double SV1_numTracks_Secondaries_KshortDecay = 0; 
    double SV1_numTracks_Secondaries_LambdaDecay = 0; 
    double SV1_numTracks_Secondaries_GammaConversion = 0; 
    double SV1_numTracks_Secondaries_OtherDecay = 0; 
    double SV1_numTracks_Secondaries_HadronicInteraction = 0; 
    double SV1_numTracks_Secondaries_OtherSecondary = 0; 
    double SV1_numTracks_OtherOrigin = 0; 

    // get the SV1 vertex
    static const SG::ConstAccessor<std::vector< ElementLink< xAOD::VertexContainer > > >
      SV1_verticesAcc("SV1_vertices");
    std::vector< ElementLink< xAOD::VertexContainer > > SV1_vertex =
      SV1_verticesAcc(*jet);

    if(SV1_vertex.size() >= 1) contains_SV = true;
    else contains_SV = false;

    // loop over the vertices
    for (unsigned int i = 0; i < SV1_vertex.size(); i++) {
      if (!SV1_vertex.at(i).isValid()) continue;
      const xAOD::Vertex* vtx = *(SV1_vertex.at(i));

      // get the track links
      std::vector<ElementLink<DataVector<xAOD::TrackParticle> > > tpLinks = vtx->trackParticleLinks();

      // loop over the tracks
      for (const ElementLink<DataVector<xAOD::TrackParticle> >& link : tpLinks) {
        const xAOD::TrackParticle* track = (*link);

        // get the track origin
        int track_origin = -1;
        if(!m_isData){
          track_origin = track_truth_associations.at(track);
        }

        // count the tracks sorted by origin (the TrkOrigin enum is defined here: https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/TrackingID/InDetTrackSystematicsTools/InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h#L14)
        if( InDet::TrkOrigin::isFromB(track_origin) ) SV1_numTracks_fromB++;
        if( InDet::TrkOrigin::isFromD(track_origin) ) SV1_numTracks_fromC++;
        if( InDet::TrkOrigin::isFragmentation(track_origin) ) SV1_numTracks_fromFragmentation++;
        if( InDet::TrkOrigin::isSecondary(track_origin) ) SV1_numTracks_fromSecondaries++;
        if( InDet::TrkOrigin::isPileup(track_origin) ) SV1_numTracks_fromPileup++;
        if( InDet::TrkOrigin::isFake(track_origin) ) SV1_numTracks_fromFake++;
        
        if(m_detailLevel > 10){
          if( track_origin & (0x1 << InDet::TrkOrigin::KshortDecay) ) SV1_numTracks_Secondaries_KshortDecay++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::LambdaDecay) ) SV1_numTracks_Secondaries_LambdaDecay++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::GammaConversion) ) SV1_numTracks_Secondaries_GammaConversion++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::OtherDecay) ) SV1_numTracks_Secondaries_OtherDecay++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::HadronicInteraction) ) SV1_numTracks_Secondaries_HadronicInteraction++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::OtherSecondary) ) SV1_numTracks_Secondaries_OtherSecondary++; 
          if( track_origin & (0x1 << InDet::TrkOrigin::OtherOrigin) ) SV1_numTracks_OtherOrigin++; 
        }

        // and count all tracks
        SV1_numTracks++;
      }
    }

    // fractions of tracks in the SV
    double SV1_frac_HF_tracks_in_jet = -1;
    double SV1_purity = -1;
    double SV1_fracTracks_fromB = -1;
    double SV1_fracTracks_fromC = -1;
    double SV1_fracTracks_fromFragmentation = -1;
    double SV1_fracTracks_fromSecondaries = -1;
    double SV1_fracTracks_fromPileup = -1;
    double SV1_fracTracks_fromFake = -1;

    // these are only filled for detail level above 10
    double SV1_fracTracks_Secondaries_KshortDecay = -1.; 
    double SV1_fracTracks_Secondaries_LambdaDecay = -1.; 
    double SV1_fracTracks_Secondaries_GammaConversion = -1.; 
    double SV1_fracTracks_Secondaries_OtherDecay = -1.; 
    double SV1_fracTracks_Secondaries_HadronicInteraction = -1.; 
    double SV1_fracTracks_Secondaries_OtherSecondary = -1.; 
    double SV1_fracTracks_OtherOrigin = -1.; 

    if(!m_isData){
      SV1_frac_HF_tracks_in_jet = (num_HF_tracks_in_jet != 0) ? ( SV1_numTracks_fromB + SV1_numTracks_fromC ) / num_HF_tracks_in_jet : -1;
      SV1_purity = (SV1_numTracks != 0) ? (SV1_numTracks_fromB + SV1_numTracks_fromC) / SV1_numTracks : -1;
      SV1_fracTracks_fromB = (SV1_numTracks != 0) ? SV1_numTracks_fromB / SV1_numTracks : -1.;
      SV1_fracTracks_fromC = (SV1_numTracks != 0) ? SV1_numTracks_fromC / SV1_numTracks : -1.;
      SV1_fracTracks_fromFragmentation = (SV1_numTracks != 0) ? SV1_numTracks_fromFragmentation / SV1_numTracks : -1.;
      SV1_fracTracks_fromSecondaries = (SV1_numTracks != 0) ? SV1_numTracks_fromSecondaries / SV1_numTracks : -1.;
      SV1_fracTracks_fromPileup = (SV1_numTracks != 0) ? SV1_numTracks_fromPileup / SV1_numTracks : -1.;
      SV1_fracTracks_fromFake = (SV1_numTracks != 0) ? SV1_numTracks_fromFake / SV1_numTracks : -1.;

      if(m_detailLevel > 10){
        SV1_fracTracks_Secondaries_KshortDecay = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_KshortDecay / SV1_numTracks : -1.; 
        SV1_fracTracks_Secondaries_LambdaDecay = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_LambdaDecay / SV1_numTracks : -1.; 
        SV1_fracTracks_Secondaries_GammaConversion = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_GammaConversion / SV1_numTracks : -1.; 
        SV1_fracTracks_Secondaries_OtherDecay = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_OtherDecay / SV1_numTracks : -1.; 
        SV1_fracTracks_Secondaries_HadronicInteraction = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_HadronicInteraction / SV1_numTracks : -1.; 
        SV1_fracTracks_Secondaries_OtherSecondary = (SV1_numTracks != 0) ? SV1_numTracks_Secondaries_OtherSecondary / SV1_numTracks : -1.; 
        SV1_fracTracks_OtherOrigin = (SV1_numTracks != 0) ? SV1_numTracks_OtherOrigin / SV1_numTracks : -1.; 
      }
    }

    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_masssvx/GeV, m_SV1_masssvx_incl, m_SV1_masssvx_b, m_SV1_masssvx_c, m_SV1_masssvx_l, m_SV1_masssvx_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_N2Tpair, m_SV1_N2Tpair_incl, m_SV1_N2Tpair_b, m_SV1_N2Tpair_c, m_SV1_N2Tpair_l, m_SV1_N2Tpair_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_efracsvx, m_SV1_efracsvx_incl, m_SV1_efracsvx_b, m_SV1_efracsvx_c, m_SV1_efracsvx_l, m_SV1_efracsvx_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_deltaR, m_SV1_deltaR_incl, m_SV1_deltaR_b, m_SV1_deltaR_c, m_SV1_deltaR_l, m_SV1_deltaR_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_significance3d, m_SV1_significance3d_incl, m_SV1_significance3d_b, m_SV1_significance3d_c, m_SV1_significance3d_l, m_SV1_significance3d_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_energyTrkInJet/GeV, m_SV1_energyTrkInJet_incl, m_SV1_energyTrkInJet_b, m_SV1_energyTrkInJet_c, m_SV1_energyTrkInJet_l, m_SV1_energyTrkInJet_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_NGTinSvx, m_SV1_NGTinSvx_incl, m_SV1_NGTinSvx_b, m_SV1_NGTinSvx_c, m_SV1_NGTinSvx_l, m_SV1_NGTinSvx_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_Lxy, m_SV1_Lxy_incl, m_SV1_Lxy_b, m_SV1_Lxy_c, m_SV1_Lxy_l, m_SV1_Lxy_muon, truth_label, has_muon, event);
    BTaggingValidationPlots::fillHistoWithTruthCases(SV1_vertex.size(), m_SV1_numSVs_incl, m_SV1_numSVs_b, m_SV1_numSVs_c, m_SV1_numSVs_l, m_SV1_numSVs_muon, truth_label, has_muon, event);
    if(!m_isData){
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_frac_HF_tracks_in_jet, m_SV1_fracHFTracksInJet_incl, m_SV1_fracHFTracksInJet_b, m_SV1_fracHFTracksInJet_c, m_SV1_fracHFTracksInJet_l, m_SV1_fracHFTracksInJet_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_purity, m_SV1_purity_incl, m_SV1_purity_b, m_SV1_purity_c, m_SV1_purity_l, m_SV1_purity_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromB, m_SV1_fracTracks_fromB_incl, m_SV1_fracTracks_fromB_b, m_SV1_fracTracks_fromB_c, m_SV1_fracTracks_fromB_l, m_SV1_fracTracks_fromB_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromC, m_SV1_fracTracks_fromC_incl, m_SV1_fracTracks_fromC_b, m_SV1_fracTracks_fromC_c, m_SV1_fracTracks_fromC_l, m_SV1_fracTracks_fromC_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromFragmentation, m_SV1_fracTracks_fromFragmentation_incl, m_SV1_fracTracks_fromFragmentation_b, m_SV1_fracTracks_fromFragmentation_c, m_SV1_fracTracks_fromFragmentation_l, m_SV1_fracTracks_fromFragmentation_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromSecondaries, m_SV1_fracTracks_fromSecondaries_incl, m_SV1_fracTracks_fromSecondaries_b, m_SV1_fracTracks_fromSecondaries_c, m_SV1_fracTracks_fromSecondaries_l, m_SV1_fracTracks_fromSecondaries_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromPileup, m_SV1_fracTracks_fromPileup_incl, m_SV1_fracTracks_fromPileup_b, m_SV1_fracTracks_fromPileup_c, m_SV1_fracTracks_fromPileup_l, m_SV1_fracTracks_fromPileup_muon, truth_label, has_muon, event);
      BTaggingValidationPlots::fillHistoWithTruthCases( SV1_fracTracks_fromFake, m_SV1_fracTracks_fromFake_incl, m_SV1_fracTracks_fromFake_b, m_SV1_fracTracks_fromFake_c, m_SV1_fracTracks_fromFake_l, m_SV1_fracTracks_fromFake_muon, truth_label, has_muon, event);

      if(m_detailLevel > 10){
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_KshortDecay, m_SV1_fracTracks_Secondaries_KshortDecay_incl, m_SV1_fracTracks_Secondaries_KshortDecay_b, m_SV1_fracTracks_Secondaries_KshortDecay_c, m_SV1_fracTracks_Secondaries_KshortDecay_u, m_SV1_fracTracks_Secondaries_KshortDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_LambdaDecay, m_SV1_fracTracks_Secondaries_LambdaDecay_incl, m_SV1_fracTracks_Secondaries_LambdaDecay_b, m_SV1_fracTracks_Secondaries_LambdaDecay_c, m_SV1_fracTracks_Secondaries_LambdaDecay_u, m_SV1_fracTracks_Secondaries_LambdaDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_GammaConversion, m_SV1_fracTracks_Secondaries_GammaConversion_incl, m_SV1_fracTracks_Secondaries_GammaConversion_b, m_SV1_fracTracks_Secondaries_GammaConversion_c, m_SV1_fracTracks_Secondaries_GammaConversion_u, m_SV1_fracTracks_Secondaries_GammaConversion_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_OtherDecay, m_SV1_fracTracks_Secondaries_OtherDecay_incl, m_SV1_fracTracks_Secondaries_OtherDecay_b, m_SV1_fracTracks_Secondaries_OtherDecay_c, m_SV1_fracTracks_Secondaries_OtherDecay_u, m_SV1_fracTracks_Secondaries_OtherDecay_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_HadronicInteraction, m_SV1_fracTracks_Secondaries_HadronicInteraction_incl, m_SV1_fracTracks_Secondaries_HadronicInteraction_b, m_SV1_fracTracks_Secondaries_HadronicInteraction_c, m_SV1_fracTracks_Secondaries_HadronicInteraction_u, m_SV1_fracTracks_Secondaries_HadronicInteraction_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_Secondaries_OtherSecondary, m_SV1_fracTracks_Secondaries_OtherSecondary_incl, m_SV1_fracTracks_Secondaries_OtherSecondary_b, m_SV1_fracTracks_Secondaries_OtherSecondary_c, m_SV1_fracTracks_Secondaries_OtherSecondary_u, m_SV1_fracTracks_Secondaries_OtherSecondary_muon, truth_label, has_muon, event); 
        BTaggingValidationPlots::fillHistoWithTruthCases(SV1_fracTracks_OtherOrigin, m_SV1_fracTracks_OtherOrigin_incl, m_SV1_fracTracks_OtherOrigin_b, m_SV1_fracTracks_OtherOrigin_c, m_SV1_fracTracks_OtherOrigin_u, m_SV1_fracTracks_OtherOrigin_muon, truth_label, has_muon, event); 
      }
    }

  }

  // a fill method for discriminant related vars
  void BTaggingValidationPlots::fillDiscriminantVariables(const xAOD::Jet* jet, const double& jet_Lxy, const int& truth_label, const bool& onZprime, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event){

    // get the GN2v01 vars
    double GN2v01_pb, GN2v01_pu, GN2v01_pc, GN2v01_ptau;
    SG::ConstAccessor<float> GN2pbAcc(m_GN2v01Name + "_pb");
    GN2v01_pb = GN2pbAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN2puAcc(m_GN2v01Name + "_pu");
    GN2v01_pu = GN2puAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN2pcAcc(m_GN2v01Name + "_pc");
    GN2v01_pc = GN2pcAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN2ptauAcc(m_GN2v01Name + "_ptau");
    GN2v01_ptau = GN2ptauAcc.withDefault(*jet, -1);

    m_GN2v01_pb->Fill(GN2v01_pb, event->beamSpotWeight());
    m_GN2v01_pu->Fill(GN2v01_pu, event->beamSpotWeight());
    m_GN2v01_pc->Fill(GN2v01_pc, event->beamSpotWeight());
    m_GN2v01_ptau->Fill(GN2v01_ptau, event->beamSpotWeight());
    // calculate the GN2 discriminant value
    double weight_GN2v01 = log( GN2v01_pb / ( GN2v01_pc * m_GN2v01_fc + GN2v01_pu * (1-m_GN2v01_fc-m_GN2v01_ftau) + GN2v01_ptau * m_GN2v01_ftau ));
  
    updateNJetsThatPassedWPCutsMap(nJetsThatPassedWPCuts,  weight_GN2v01, -9999);

    // fill the histograms with the tagger discriminants
    for(std::map<std::string, TH1*>::const_iterator hist_iter=m_weight_histos.begin(); hist_iter!=m_weight_histos.end(); ++hist_iter){
      for(std::map<std::string, int>::const_iterator label_iter = m_truthLabels.begin(); label_iter != m_truthLabels.end(); ++label_iter){

        // GN2v01 taggers
        bool pass_nTracksCut_GN2v01 = true;
        BTaggingValidationPlots::fillDiscriminantHistograms("GN2v01_", weight_GN2v01, m_GN2v01_workingPoints, truth_label, hist_iter, label_iter, pass_nTracksCut_GN2v01, jet->pt(), jet_Lxy, onZprime, event);

      }
    }
  }

  // a fill method for discriminant related vars
  void BTaggingValidationPlots::fillDiscriminantVariables_for_largeRjet(const xAOD::Jet* jet, const int& truth_label, const bool& onZprime, std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event){
    // get the GN3XPV01 vars
    double GN3XPV01_phtautauhad, GN3XPV01_phbb, GN3XPV01_phcc, GN3XPV01_ptop, GN3XPV01_pqcdbb, GN3XPV01_pqcdbx, GN3XPV01_pqcdcx, GN3XPV01_pqcdll, GN3XPV01_pwqq;

    SG::ConstAccessor<float> GN3XPV01_phtautauhadAcc(m_GN3XPV01Name + "_phtautauhad");
    GN3XPV01_phtautauhad = GN3XPV01_phtautauhadAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01phbbAcc(m_GN3XPV01Name + "_phbb");
    GN3XPV01_phbb = GN3XPV01phbbAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01phccAcc(m_GN3XPV01Name + "_phcc");
    GN3XPV01_phcc = GN3XPV01phccAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01ptopAcc(m_GN3XPV01Name + "_ptop");
    GN3XPV01_ptop = GN3XPV01ptopAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01pqcdbbAcc(m_GN3XPV01Name + "_pqcdbb");
    GN3XPV01_pqcdbb = GN3XPV01pqcdbbAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01pqcdbxAcc(m_GN3XPV01Name + "_pqcdbx");
    GN3XPV01_pqcdbx = GN3XPV01pqcdbxAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01pqcdcxAcc(m_GN3XPV01Name + "_pqcdcx");
    GN3XPV01_pqcdcx = GN3XPV01pqcdcxAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01pqcdllAcc(m_GN3XPV01Name + "_pqcdll");
    GN3XPV01_pqcdll = GN3XPV01pqcdllAcc.withDefault(*jet, -1);

    SG::ConstAccessor<float> GN3XPV01pwqqAcc(m_GN3XPV01Name + "_pWqq");
    GN3XPV01_pwqq = GN3XPV01pwqqAcc.withDefault(*jet, -1);

    m_GN3XPV01_phtautauhad->Fill(GN3XPV01_phtautauhad, event->beamSpotWeight());
    m_GN3XPV01_phbb->Fill(GN3XPV01_phbb, event->beamSpotWeight());
    m_GN3XPV01_phcc->Fill(GN3XPV01_phcc, event->beamSpotWeight());
    m_GN3XPV01_ptop->Fill(GN3XPV01_ptop, event->beamSpotWeight());
    m_GN3XPV01_pqcdbb->Fill(GN3XPV01_pqcdbb, event->beamSpotWeight());
    m_GN3XPV01_pqcdbx->Fill(GN3XPV01_pqcdbx, event->beamSpotWeight());
    m_GN3XPV01_pqcdcx->Fill(GN3XPV01_pqcdcx, event->beamSpotWeight());
    m_GN3XPV01_pqcdll->Fill(GN3XPV01_pqcdll, event->beamSpotWeight());
    m_GN3XPV01_pwqq->Fill(GN3XPV01_pwqq, event->beamSpotWeight());
    
    // TODO: This is a dummy WP, need to define proper WPs for large-R jets as a function of pT and mass.
    double weight_GN3XPV01 = log( GN3XPV01_phbb / ( ( GN3XPV01_phcc * m_GN3XPV01_hcc_fc ) + ( GN3XPV01_ptop * m_GN3XPV01_top_fc )  + GN3XPV01_pqcdbb * (1-m_GN3XPV01_top_fc - m_GN3XPV01_hcc_fc ) ) );
    updateNJetsThatPassedWPCutsMap(nJetsThatPassedWPCuts, -9999, weight_GN3XPV01);

    // TODO: Fill the different WPs as a function of pT and mass.
    // fill the histograms with the tagger discriminants 
    for(std::map<std::string, TH1*>::const_iterator hist_iter=m_weight_histos.begin(); hist_iter!=m_weight_histos.end(); ++hist_iter){
      for(std::map<std::string, int>::const_iterator label_iter = m_truthLabels.begin(); label_iter != m_truthLabels.end(); ++label_iter){

        // GN3XPV01 taggers
        bool pass_nTracksCut_GN3XPV01 = true;
        BTaggingValidationPlots::fillDiscriminantHistograms("GN3XPV01_", weight_GN3XPV01, m_GN3XPV01_workingPoints, truth_label, hist_iter, label_iter, pass_nTracksCut_GN3XPV01, jet->pt(), -1, onZprime, event);

      }
    }
  }



  void BTaggingValidationPlots::finalizePlots(){
  }


// methods for the num b-tagged jets
  void BTaggingValidationPlots::bookNJetsThatPassedWPCutsHistos(){
    // loop over the taggers
    for(std::vector<std::string>::const_iterator tag_iter = m_taggers.begin(); tag_iter != m_taggers.end(); ++tag_iter){
      // get the right working points
      std::map<std::string, double> workingPoints;
      if(*tag_iter == "GN2v01") workingPoints = m_GN2v01_workingPoints;
      else if(*tag_iter == "GN3XPV01") workingPoints = m_GN3XPV01_workingPoints;
      // loop over the working points
      for(std::map<std::string, double>::const_iterator working_points_iter = workingPoints.begin(); working_points_iter != workingPoints.end(); ++working_points_iter){
        std::string name = "nJetsThatPassedWPCuts_" + *tag_iter + "_" + working_points_iter->first; 
        // book the histo
        TH1* histo = bookHistogram(name, "nJetsThatPassedWPCuts", m_sParticleType, "for " + *tag_iter + " discriminat > "+ std::to_string(working_points_iter->second) + ": " );     
        // add the histo to map
        m_nJetsThatPassedWPCutsHistos.insert( std::make_pair(name, histo) );
      }
    }
  }

  void BTaggingValidationPlots::initializeNJetsThatPassedWPCutsMap(std::map<std::string, int>& nJetsThatPassedWPCuts){
    // loop over the taggers
    for(std::vector<std::string>::const_iterator tag_iter = m_taggers.begin(); tag_iter != m_taggers.end(); ++tag_iter){
      // get the right working points
      std::map<std::string, double> workingPoints;
      if(*tag_iter == "GN2v01") workingPoints = m_GN2v01_workingPoints;
      else if(*tag_iter == "GN3XPV01") workingPoints = m_GN3XPV01_workingPoints;
      // loop over the working points
      for(std::map<std::string, double>::const_iterator working_points_iter = workingPoints.begin(); working_points_iter != workingPoints.end(); ++working_points_iter){
        std::string name = "nJetsThatPassedWPCuts_" + *tag_iter + "_" + working_points_iter->first; 
        // pre-fill the njets value with zero
        nJetsThatPassedWPCuts.insert( std::make_pair(name, 0) );
      }
    }
  }

  void BTaggingValidationPlots::updateNJetsThatPassedWPCutsMap(std::map<std::string, int>& nJetsThatPassedWPCuts, const double& discr_GN2v01, const double& discr_GN3XPV01){
    // loop over the taggers
    for(std::vector<std::string>::const_iterator tag_iter = m_taggers.begin(); tag_iter != m_taggers.end(); ++tag_iter){
      // get the right working points and discriminant values
      std::map<std::string, double> workingPoints;
      double discriminant_value = 0;
      if(*tag_iter == "GN2v01"){ workingPoints = m_GN2v01_workingPoints; discriminant_value = discr_GN2v01; }
      else if(*tag_iter == "GN3XPV01"){ workingPoints = m_GN3XPV01_workingPoints; discriminant_value = discr_GN3XPV01; }
      // loop over the working points
      for(std::map<std::string, double>::const_iterator working_points_iter = workingPoints.begin(); working_points_iter != workingPoints.end(); ++working_points_iter){
        std::string name = "nJetsThatPassedWPCuts_" + *tag_iter + "_" + working_points_iter->first; 
        // update the njets value if the wp cut is passed
        if(discriminant_value > working_points_iter->second){
          nJetsThatPassedWPCuts.at(name) += 1;
        }
      }
    }
  }

  void BTaggingValidationPlots::fillNJetsThatPassedWPCutsHistos(std::map<std::string, int>& nJetsThatPassedWPCuts, const xAOD::EventInfo* event){
    // loop over the taggers
    for(std::vector<std::string>::const_iterator tag_iter = m_taggers.begin(); tag_iter != m_taggers.end(); ++tag_iter){
      // get the right working points
      std::map<std::string, double> workingPoints;
      if(*tag_iter == "GN2v01") workingPoints = m_GN2v01_workingPoints;
      else if(*tag_iter == "GN3XPV01") workingPoints = m_GN3XPV01_workingPoints;
      // loop over the working points
      for(std::map<std::string, double>::const_iterator working_points_iter = workingPoints.begin(); working_points_iter != workingPoints.end(); ++working_points_iter){
        std::string name = "nJetsThatPassedWPCuts_" + *tag_iter + "_" + working_points_iter->first; 
        // fill the histo
        m_nJetsThatPassedWPCutsHistos.at(name)->Fill( nJetsThatPassedWPCuts.at(name), event->beamSpotWeight() );
      }
    }
  }




  void BTaggingValidationPlots::setTaggerInfos(){
    // list of all taggers
    m_taggers.push_back("GN2v01");
    m_taggers.push_back("GN3XPV01");

    // list of all truth labels
    m_truthLabels.insert(std::make_pair("b", 5));
    m_truthLabels.insert(std::make_pair("c", 4));
    m_truthLabels.insert(std::make_pair("u", 0));
    m_truthLabels.insert(std::make_pair("tau", 15));

    // GN2v01   
    m_GN2v01_fc = 0.2;
    m_GN2v01_ftau = 0.01;
    m_GN2v01_workingPoints.insert(std::make_pair("70", 1.892));
    if(m_detailLevel > 10){
      m_GN2v01_workingPoints.insert(std::make_pair("65", 2.669));
      m_GN2v01_workingPoints.insert(std::make_pair("77", 0.844));
      m_GN2v01_workingPoints.insert(std::make_pair("85", -0.378));
      m_GN2v01_workingPoints.insert(std::make_pair("90", -1.34));
    }

    // GN3XPV01 // TODO: update WPs in the future as a function of pT and mass.
    m_GN3XPV01_hcc_fc = 0.02;
    m_GN3XPV01_top_fc = 0.25;
    m_GN3XPV01_workingPoints.insert(std::make_pair("60", 3.818)); // Dummy WP
    // if(m_detailLevel > 10) Add more WPs here in future. 
  }
 
 
  void BTaggingValidationPlots::bookEffHistos(){
    setTaggerInfos();
    for(std::vector<std::string>::const_iterator tag_iter = m_taggers.begin(); tag_iter != m_taggers.end(); 
        ++tag_iter){

      for(std::map<std::string, int>::const_iterator label_iter = m_truthLabels.begin(); label_iter != m_truthLabels.end(); 
        ++label_iter){

        // book discriminant histograms
        std::string histo_name_matched = *tag_iter+"_"+label_iter->first+"_matched_weight";
        std::string var_name_matched = "llr";
        if((*tag_iter).find("MV") < 1) var_name_matched += "_MV";
        TH1* histo_matched = bookHistogram(histo_name_matched, var_name_matched, m_sParticleType, label_iter->first + "-jets" + ", " + *tag_iter);    
        m_weight_histos.insert(std::make_pair(histo_name_matched, histo_matched));

        // book discriminant with nTracksCut histograms
        std::string histo_name_trackCuts = *tag_iter+"_"+label_iter->first+"_matched_weight_trackCuts";
        std::string var_name_trackCuts = "llr_nTracksCut";
        if((*tag_iter).find("MV") < 1) var_name_trackCuts += "_MV";
        TH1* histo_trackCuts = bookHistogram(histo_name_trackCuts, var_name_trackCuts, m_sParticleType, label_iter->first + "-jets" + ", " + *tag_iter);    
        m_weight_histos.insert(std::make_pair(histo_name_trackCuts, histo_trackCuts));
        

        // book the vs pT histograms (the bool in the argument says if it is an old tagger (for sub-folder sorting later))
        //GN2v01 
        if(*tag_iter == "GN2v01"){
          bookDiscriminantVsPTAndLxyHistograms("GN2v01", m_GN2v01_workingPoints, false, label_iter, m_sParticleType);
        }
        else if(*tag_iter == "GN3XPV01"){
          bookDiscriminantVsPTAndLxyHistograms("GN3XPV01", m_GN3XPV01_workingPoints, false, label_iter, m_sParticleType);
        }
      }
    }

    bookNJetsThatPassedWPCutsHistos();

  }
      
}
