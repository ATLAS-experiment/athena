/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetVKalVxInJetTool/InDetTrkInJetType.h"
#include "PathResolver/PathResolver.h"
#include "TLorentzVector.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include "MVAUtils/BDT.h"
#include "TFile.h"
#include "TTree.h"

//
//-------------------------------------------------
namespace InDet {
//
//Constructor-------------------------------------------------------------- 
InDetTrkInJetType::InDetTrkInJetType(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent):
  base_class(type,name,parent)
  { }

//Destructor---------------------------------------------------------------
  InDetTrkInJetType::~InDetTrkInJetType(){
    ATH_MSG_DEBUG("InDetTrkInJetType destructor called");
  }

//Initialize---------------------------------------------------------------
   StatusCode InDetTrkInJetType::initialize(){
     m_initialised = 0;
//
//-- Calibration file
//
     std::string fullPathToFile = PathResolverFindCalibFile("InDetVKalVxInJetTool/"+m_calibFileName);
     if(m_useFivePtJetBinVersion) {fullPathToFile = PathResolverFindCalibFile("InDetVKalVxInJetTool/"+m_calibFileNameFivePtJetBin);}     
     std::unique_ptr<TFile> rootFile(TFile::Open(fullPathToFile.c_str(), "READ"));    
     if (!rootFile) {
        if(!m_useFivePtJetBinVersion) {ATH_MSG_ERROR("Can not retrieve TrackClassification calibration root file: " << m_calibFileName); }
        else {ATH_MSG_ERROR("Can not retrieve TrackClassification five ptjet bin calibration root file: " << m_calibFileNameFivePtJetBin); }
        return StatusCode::FAILURE;
     }
     if(!m_useFivePtJetBinVersion)
     {
      std::unique_ptr<TTree> training((TTree*)rootFile->Get("BDT"));
      m_vTrkClassBDT.push_back(std::make_unique<MVAUtils::BDT>(training.get()));
     }
     else
     {
      unsigned int nPtjetRanges=5;
      std::vector<std::string> v_BDTName(nPtjetRanges,"");
     
      v_BDTName.at(e_ptjet0to0p2TeV) = "BDTG_0_200GeV";
      v_BDTName.at(e_ptjet0p2to0p5TeV) = "BDTG_200_500GeV";
      v_BDTName.at(e_ptjet0p5to1TeV) = "BDTG_500_1000GeV";
      v_BDTName.at(e_ptjet1to2TeV) = "BDTG_1000_2000GeV";
      v_BDTName.at(e_ptjet2to7TeV) = "BDTG_gt2000GeV";

      for(const auto& bdtname : v_BDTName)
      {
        std::unique_ptr<TTree> training((TTree*)rootFile->Get(bdtname.c_str()));
        m_vTrkClassBDT.push_back(std::make_unique<MVAUtils::BDT>(training.get()));
      }
      if(m_vTrkClassBDT.size() != nPtjetRanges) {ATH_MSG_ERROR("TrackClassification five ptjet bin BDT model not correctly loaded."); }
     }
     if(!m_useFivePtJetBinVersion) {if(m_vTrkClassBDT.size() != 1) {ATH_MSG_ERROR("TrackClassification BDT model not correctly loaded."); }}
     //-------
     if (m_fitterSvc.retrieve().isFailure()) {
        ATH_MSG_DEBUG("Could not find Trk::TrkVKalVrtFitter");
        return StatusCode::SUCCESS;
     } else {
        ATH_MSG_DEBUG("InDetTrkInJetTool TrkVKalVrtFitter found");
     }
     m_fitSvc = dynamic_cast<Trk::TrkVKalVrtFitter*>(&(*m_fitterSvc));
     if(!m_fitSvc){
        ATH_MSG_DEBUG(" No implemented Trk::ITrkVKalVrtFitter interface");
        return StatusCode::SUCCESS;
     }

     if(!m_jetCollection.empty())
     {

     //from https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Event/xAOD/xAODTrackingCnv/src/TrackParticleCnvAlg.cxx
      m_trackWriteDecorKeyTCTScore = "InDetTrackParticles.TCTScore_"+m_jetCollection;
      m_trackWriteDecorKeyJetLink = "InDetTrackParticles.TCTJetLink_"+m_jetCollection;

      m_jetWriteDecorKeyTCTScore = m_jetCollection+ ".TCTScore";
      m_jetWriteDecorKeyTrackLink = m_jetCollection +".TCTTrackLink";
    
      //initialize SG WriteDecorHandleKeys needed for decorating tracks
      ATH_CHECK( m_trackWriteDecorKeyTCTScore.initialize());
      ATH_CHECK( m_trackWriteDecorKeyJetLink.initialize());

      //initialize SG WriteDecorHandleKeys needed for decorating jets
      ATH_CHECK( m_jetWriteDecorKeyTCTScore.initialize());
      ATH_CHECK( m_jetWriteDecorKeyTrackLink.initialize()); 
     }
     

     m_initialised = 1;          // Tool is initialised successfully.
//-----
     if(msgLvl(MSG::DEBUG)) ATH_CHECK(m_timingProfile.retrieve());
//-----
     return StatusCode::SUCCESS;
   }

   StatusCode InDetTrkInJetType::finalize()
   {
    if(m_timingProfile)m_timingProfile->chronoPrint("InDet_TrkInJetType");
    ATH_MSG_DEBUG("InDetTrkInJetType finalize()");
    return StatusCode::SUCCESS; 
   }

   std::vector<float> InDetTrkInJetType::trkTypeWgts(const EventContext& ctx, const xAOD::TrackParticle * Trk, const xAOD::Vertex & PV, const TLorentzVector & Jet) const
   {  
//-- Track quality checks
      std::vector<float> safeReturn(3,0.);
      if( !m_initialised )                                  return safeReturn;
      double wrkJetPt= Jet.Perp() < m_jetMaxPtCut.value() ? Jet.Perp() : m_jetMaxPtCut.value(); // Jet above m_jetMaxPtCut is considered as having m_jetMaxPtCut
      if(Trk->pt() < m_trkMinPtCut)                         return safeReturn;  // Don't classify track below m_trkMinPtCut
      if(Trk->pt() > wrkJetPt)                              return safeReturn;  // Don't classify track with Pt above JetPt
      if(Trk->numberDoF() == 0)                             return safeReturn;  // Safety
      if(Trk->chiSquared()/Trk->numberDoF() > m_trkChi2Cut) return safeReturn;
      uint8_t PixelHits,SctHits;
      if( !(Trk->summaryValue(PixelHits,xAOD::numberOfPixelHits)) ) return safeReturn; // No Pixel hits. Bad.
      if( !(Trk->summaryValue(  SctHits,xAOD::numberOfSCTHits))   ) return safeReturn; // No SCT hits. Bad.
      if( PixelHits < m_trkPixelHitsCut )                   return safeReturn;
      if( SctHits   < m_trkSctHitsCut )                     return safeReturn;
      std::vector<double> Impact,ImpactError;
      float Sig3D=m_fitSvc->VKalGetImpact(ctx, Trk, PV.position(), 1, Impact, ImpactError);
      AmgVector(5) tmpPerigee = Trk->perigeeParameters().parameters(); 
      if( std::sin(tmpPerigee[2]-Jet.Phi())*Impact[0] < 0 ){ Impact[0] = -std::abs(Impact[0]);}
                                                       else{ Impact[0] =  std::abs(Impact[0]);}
      if(  (tmpPerigee[3]-Jet.Theta())*Impact[1] < 0 )     { Impact[1] = -std::abs(Impact[1]);}
                                                       else{ Impact[1] =  std::abs(Impact[1]);}
      double SignifR = Impact[0]/ std::sqrt(ImpactError[0]);
      double SignifZ = Impact[1]/ std::sqrt(ImpactError[2]);
//---Calibrated range selection
      if(Impact[1]<m_Z0_limLow || Impact[1]>m_Z0_limUpp) return safeReturn;  //Don't classify track far from PV in Z
      if( std::sqrt(SignifR*SignifR +SignifZ*SignifZ) < 1.)   return safeReturn;  //Don't classify track too close to PV
//---IBL/BL hits
      int hitIBL=0, hitBL=0; 
      uint8_t IBLhit,BLhit,IBLexp,BLexp;
      if(!Trk->summaryValue( IBLhit,  xAOD::numberOfInnermostPixelLayerHits) )        IBLhit = 0;
      if(!Trk->summaryValue(  BLhit,  xAOD::numberOfNextToInnermostPixelLayerHits) )   BLhit = 0;
      if(!Trk->summaryValue( IBLexp,  xAOD::expectInnermostPixelLayerHit) )           IBLexp = 0;
      if(!Trk->summaryValue(  BLexp,  xAOD::expectNextToInnermostPixelLayerHit) )      BLexp = 0;
      hitIBL=IBLhit; if( IBLexp==0 ) hitIBL=-1;
      hitBL = BLhit; if(  BLexp==0 ) hitBL =-1;
/*---PV constraint (doesn't improve rejection in first try)
      Amg::Vector3D     FitVrt;
      TLorentzVector    Momentum;
      long int  Charge=0;
      std::vector<double> ErrorMatrix, Chi2PerTrk;
      std::vector< std::vector<double> > TrkAtVrt;
      std::vector<const xAOD::TrackParticle *>  TrkForFit(1,Trk);
      std::vector<const xAOD::NeutralParticle*> netralDummy(0);
      m_fitSvc->setDefault();                                  //Reset VKalVrt settings
      std::vector<float> covPV=PV.covariance();
      m_fitSvc->setVertexForConstraint(PV.x(),PV.y(),PV.z());
      m_fitSvc->setCovVrtForConstraint(covPV[0],covPV[1],covPV[2],covPV[3],covPV[4],covPV[5]);
      m_fitSvc->setCnstType(6);                                // Set primary vertex constraint
      StatusCode sc=m_fitSvc->VKalVrtFit( TrkForFit, netralDummy, FitVrt, Momentum, Charge, ErrorMatrix, Chi2PerTrk, TrkAtVrt, Chi2);
      if(sc.isFailure())Chi2=std::exp(11.);
      if(Chi2>std::exp(11.))Chi2=std::exp(11.);
*/
//====================== BDT weights
     double coeffPt=10.;
     double pfrac=(Trk->pt()-m_trkMinPtCut)/sqrt(wrkJetPt);
     float prbP= pfrac/(coeffPt+pfrac);
     float etatrk=Trk->eta();
//---
     double coeffSig=1.0;
     if(Sig3D<coeffSig) return safeReturn;  // Safety
//---
     float d0=Impact[0];
     d0=std::max(d0, m_d0_limLow.value());
     d0=std::min(d0, m_d0_limUpp.value()); //Track
     float SigZ=SignifZ;
     float SigR=SignifR;
//---
     float ptjet=wrkJetPt>m_jetMinPtCut ? wrkJetPt : m_jetMinPtCut.value(); //Very low jet pt is replaced by Pt=35GeV
//---
     TLorentzVector TLV; 
     TLV.SetPtEtaPhiE(Trk->pt(),Trk->eta(),Trk->phi(),Trk->e());
     float pTvsJet=TLV.Perp(Jet.Vect());
//---
     if(m_timingProfile)m_timingProfile->chronoStart("InDet_TrkInJetType");
     //-----Use MVAUtils to save CPU
     std::vector<float> bdt_vars={Sig3D, prbP, pTvsJet, d0, SigR, SigZ, ptjet, (float)hitIBL, (float)hitBL, etatrk};
     std::vector<float> weights;
     if(!m_useFivePtJetBinVersion) {weights =m_vTrkClassBDT.at(0)->GetMultiResponse(bdt_vars,3);}
     else{
      e_ptjetRange e_ptjet;
      if(ptjet < 0.2e6) {e_ptjet = e_ptjet0to0p2TeV; }
      else if(ptjet > 0.2e6 && ptjet < 0.5e6) {e_ptjet = e_ptjet0p2to0p5TeV; }
      else if(ptjet > 0.5e6 && ptjet < 1.e6) {e_ptjet = e_ptjet0p5to1TeV; }
      else if(ptjet > 1.e6 && ptjet < 2.e6) {e_ptjet = e_ptjet1to2TeV; }
      else if(ptjet > 2.e6) {e_ptjet = e_ptjet2to7TeV; }
      else {ATH_MSG_ERROR("Not a valid ptjet value in five ptjet bin TrackClassification application: " << ptjet); return safeReturn; }
      weights = m_vTrkClassBDT.at(e_ptjet)->GetMultiResponse(bdt_vars,3); 
     }
     
     //-----
     if(m_timingProfile)m_timingProfile->chronoStop("InDet_TrkInJetType");
     return weights; //order: wgtB, wgtL, wgtG
   }

    void InDetTrkInJetType::decorateTrack(const EventContext& ctx, const xAOD::TrackParticle* trk, const xAOD::Vertex & PV, const xAOD::JetContainer & jets, const xAOD::Jet* curjet) const
    {
      if(m_jetCollection.empty()) {ATH_MSG_FATAL("No JetContainer chosen for decorating tracks!"); }

      SG::WriteDecorHandle< xAOD::TrackParticleContainer, std::vector<float> > trackWriteDecorHandleTCTScore (m_trackWriteDecorKeyTCTScore, ctx);
      SG::WriteDecorHandle< xAOD::TrackParticleContainer, ElementLink<xAOD::JetContainer> > trackWriteDecorHandleJetLink (m_trackWriteDecorKeyJetLink, ctx);
      std::vector<float> v_tctScore = trkTypeWgts(ctx,trk,PV,curjet->p4());
      trackWriteDecorHandleTCTScore(*trk) = std::move(v_tctScore);
      ElementLink< xAOD::JetContainer> linkJet;
      linkJet.toContainedElement(jets, curjet, ctx);
      trackWriteDecorHandleJetLink(*trk) = linkJet;
    }

   
    void InDetTrkInJetType::decorateJet(const EventContext& ctx, const std::vector<const xAOD::TrackParticle*> & trks, const xAOD::TrackParticleContainer& trkContainer, const xAOD::Vertex & PV, const xAOD::Jet* curjet) const
   {
    if(m_jetCollection.empty()) {ATH_MSG_FATAL("No JetContainer chosen for decorating tracks!"); }
    SG::WriteDecorHandle< xAOD::JetContainer, std::vector<std::vector<float>> > jetWriteDecorHandleTCTScore (m_jetWriteDecorKeyTCTScore, ctx);
    SG::WriteDecorHandle< xAOD::JetContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>> > jetWriteDecorHandleTrackLink (m_jetWriteDecorKeyTrackLink, ctx);

    for(const auto *itrk : trks)
    {
      std::vector<float> v_tctScore = trkTypeWgts(ctx,itrk,PV,curjet->p4());
      jetWriteDecorHandleTCTScore(*curjet).push_back(std::move(v_tctScore));
      //adapted from https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/PhysicsAnalysis/JetTagging/JetTagAlgs/BTagging/src/JetBTaggingAlg.cxx
      //line 253-255
      ElementLink< xAOD::TrackParticleContainer> linkTrack;
      linkTrack.toContainedElement(trkContainer, itrk, ctx);
      jetWriteDecorHandleTrackLink(*curjet).push_back(linkTrack);        
    } // end track loop
  }

}// close namespace

