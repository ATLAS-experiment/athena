/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetTagTools/ClassifiedTrackTaggerTool.h"
#include "PathResolver/PathResolver.h"
#include "TLorentzVector.h"

#include "MVAUtils/BDT.h"
#include "TFile.h"
#include "TTree.h"
#include "GaudiKernel/IChronoStatSvc.h"
//
//-------------------------------------------------
namespace Analysis {
//
//Constructor-------------------------------------------------------------- 
  ClassifiedTrackTaggerTool::ClassifiedTrackTaggerTool(const std::string& type,
						       const std::string& name,
						       const IInterface* parent):
    base_class(type,name,parent),
    m_trackClassificator("InDet::InDetTrkInJetType/TrackClassificationTool",this),
    m_deltaRConeSize(0.4),
    m_useFivePtJetBinTCT(false),
    m_calibFileName("CTT_calib_v00.root"),
    m_jetCollection("AntiKt4EMPFlowJets")
  {
    declareProperty("TrackClassificationTool",  m_trackClassificator);
    declareProperty("deltaRConeSize", m_deltaRConeSize);
    declareProperty("useFivePtJetBinTCT",m_useFivePtJetBinTCT);
    declareProperty("JetCollection",m_jetCollection);
    m_timingProfile=nullptr;
  }

//Initialize---------------------------------------------------------------
  StatusCode ClassifiedTrackTaggerTool::initialize(){
    //retrieve calibration file and initialize the m_trkClassBDT
    std::string fullPathToFile = PathResolverFindCalibFile("BTagging/20221012track/"+m_calibFileName);

    std::string strBDTName = m_useFivePtJetBinTCT ?
      "CTTtrainedWithRetrainedTCT" : "CTTtrainedWithDefaultTCT";

    std::unique_ptr<TFile> rootFile(TFile::Open(fullPathToFile.c_str(), "READ"));
    if (!rootFile) {
      ATH_MSG_ERROR("Can not retrieve ClassifiedTrackTagger calibration root file: " << fullPathToFile);
      return StatusCode::FAILURE;
    }
    std::unique_ptr<TTree> training( (TTree*)rootFile->Get(strBDTName.c_str()) );
    m_CTTBDT = std::make_unique<MVAUtils::BDT>(training.get());

    //-------
    //check that the TrackClassificationTool can be accessed-> InDetTrkInJetType to get TCT weights per TrackParticle
    if (m_trackClassificator.retrieve().isFailure()) {
      ATH_MSG_DEBUG("Could not find InDet::InDetTrkInJetType - TrackClassificationTool");
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG("InDet::InDetTrkInJetType - TrackClassificationTool found");
    }
     
    //check that the fivePtJetBinTCT is actually used, if CTT is configured to do so
    if(m_useFivePtJetBinTCT){
      ATH_MSG_DEBUG("FivePtJetBin version of TCT is used");
      if(!m_trackClassificator->usesFivePtJetBinVersion())
	ATH_MSG_ERROR("FivePtJetBin TCT tool is not used, but required by CTT!");
    }
    else{
      ATH_MSG_DEBUG("Default version of TCT is used");
      if(m_trackClassificator->usesFivePtJetBinVersion())
	ATH_MSG_ERROR("FivePtJetBin TCT tool is used, but default version required by CTT!");
    }

    //SG::WriteDecorHandleKey for CTT jet decoration
    if(m_jetCollection.empty()) {ATH_MSG_FATAL("No JetCollection specified! ");}
    else {
      m_jetWriteDecorKey = m_jetCollection +".CTTScore";
      ATH_CHECK( m_jetWriteDecorKey.initialize());
    }

    if(msgLvl(MSG::DEBUG)) ATH_CHECK(service("ChronoStatSvc", m_timingProfile));
//-----
    return StatusCode::SUCCESS;
  }

  StatusCode ClassifiedTrackTaggerTool::finalize()
  {
    if(m_timingProfile)m_timingProfile->chronoPrint("ClassifiedTrackTaggerTool");
    ATH_MSG_DEBUG("ClassifiedTrackTaggerTool finalize()");
    return StatusCode::SUCCESS; 
  }

  float ClassifiedTrackTaggerTool::bJetWgts(const std::vector<const xAOD::TrackParticle*> & InpTrk, const xAOD::Vertex & PV, const TLorentzVector & Jet) const
   {
     std::vector<std::vector<float>> TCTweights;
     //for each track inside a cone of deltaR around the jet direction save the TCT output (wgtB, wgtL,wgtG)
     //if it was not rejected by the TCT track quality cuts
     for (const auto &itrk : InpTrk) {
       if((itrk->p4()).DeltaR(Jet)<=m_deltaRConeSize) {
	 std::vector<float> v_tctScore = m_trackClassificator->trkTypeWgts(itrk, PV, Jet);
	 bool b_zeroTCTScore = std::all_of(v_tctScore.begin(), v_tctScore.end(), [](float i) { return i==0; });
	 if(!b_zeroTCTScore) { TCTweights.push_back(v_tctScore); }
       }
     }
      
     ATH_MSG_DEBUG("[ClassifiedTrackTagger]: retrieved TCT score");
     int ntrk = TCTweights.size();
     if(ntrk< 3) {return -5; } //if less than three tracks passing quality cuts of TCT -> return default value of -5

     //get sorted indices w.r.t. wgtB (highest wgtB track -> first index in sorted_indices)
     std::vector<int> sorted_indices = GetSortedIndices(TCTweights);
     ATH_MSG_DEBUG("[ClassifiedTrackTagger]: ntrk = " << ntrk << " in jet");

     float ptjet = Jet.Pt();
     float trackMultiplicity = ( ((float)ntrk) / ptjet) * 1.e3;

     if(m_timingProfile)m_timingProfile->chronoStart("ClassifiedTrackTaggerTool");

     //-----Use MVAUtils to save CPU
     //order in which variables are given to the BDT wgtB_0,wgtG_0,wgtL_0,wgtB_1,wgtG_1,wgtL_1,wgtB_2,wgtG_2,wgtL_2, (ntrk/ptjet * 1.e3)
     // (0: track with highest wgtB), (1: track with 2nd highest wgtB), (2: track with 3rd highest wgtB)
     int iwgtB=0, iwgtL=1, iwgtG=2;
     ATH_MSG_DEBUG("[ClassifiedTrackTagger]: ordered signal TCT weights = " << TCTweights[sorted_indices[0]][iwgtB] << "," << TCTweights[sorted_indices[1]][iwgtB] << "," << TCTweights[sorted_indices[2]][iwgtB]);

     //change input variable ordering when final BDT model is chosen!
     std::vector<float> bdt_vars = {
       TCTweights[sorted_indices[0]][iwgtB], TCTweights[sorted_indices[0]][iwgtG], TCTweights[sorted_indices[0]][iwgtL],
       TCTweights[sorted_indices[1]][iwgtB], TCTweights[sorted_indices[1]][iwgtG], TCTweights[sorted_indices[1]][iwgtL],
       TCTweights[sorted_indices[2]][iwgtB], TCTweights[sorted_indices[2]][iwgtG], TCTweights[sorted_indices[2]][iwgtL],
       trackMultiplicity};
     float score=m_CTTBDT->GetGradBoostMVA(bdt_vars);

     ATH_MSG_DEBUG("[ClassifiedTrackTagger]: CTT classification score = " << score);

     if(m_timingProfile)m_timingProfile->chronoStop("ClassifiedTrackTaggerTool");
     return score;
   }


  void ClassifiedTrackTaggerTool::decorateJets(const std::vector<const xAOD::TrackParticle*> & InpTrk, const xAOD::Vertex & primVertex, const xAOD::JetContainer & jets) const
  {
    SG::WriteDecorHandle< xAOD::JetContainer, float > jetWriteDecorHandle (m_jetWriteDecorKey);
    for(const auto& curjet : jets){
      ATH_MSG_DEBUG( " Jet  pt: " << curjet->pt()<<" eta: "<<curjet->eta()<<" phi: "<< curjet->phi() );
      float CTTScore = bJetWgts(InpTrk, primVertex, curjet->p4());
      jetWriteDecorHandle(*curjet) = CTTScore;
    }
  }

  std::vector<int> ClassifiedTrackTaggerTool::GetSortedIndices(std::vector<std::vector<float>> unordered_vec) const
  {
    //from https://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
    int ntrk = unordered_vec.size();
    std::vector<int> indices;
    indices.clear();
    for(int i=0; i < ntrk; i++) indices.push_back(i);

    //sort the vector of indices, such that the index corresponding to the highest wgtB stands first, with the lowest last (unordered_vec[itrk][iwgt], wgtB-> iwgt=0)
    std::sort(std::begin(indices), std::end(indices),[&unordered_vec](size_t itrk1, size_t itrk2) {return unordered_vec[itrk1][0] > unordered_vec[itrk2][0];});

    return indices;
  }
   
}// close namespace
