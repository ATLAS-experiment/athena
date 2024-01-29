/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


// local include(s)
#include "TauAnalysisTools/TauSelectionCuts.h"
#include "TauAnalysisTools/TauSelectionTool.h"

// framework include(s)
#include "AsgDataHandles/ReadHandle.h"

// EDM include(s)
#include "xAODMuon/MuonContainer.h"

using namespace TauAnalysisTools;

//______________________________________________________________________________
TauSelectionCut::TauSelectionCut(const std::string& sName, TauAnalysisTools::TauSelectionTool* tTST)
  : m_sName(sName)
  , m_hHistCutPre(nullptr)
  , m_hHistCut(nullptr)
  , m_tTST(tTST)
{
}

//______________________________________________________________________________
TauSelectionCut::~TauSelectionCut()
{
  // FIXME: could use unique_ptr
  delete m_hHistCutPre;
  delete m_hHistCut;
}

//______________________________________________________________________________
void TauSelectionCut::writeControlHistograms()
{
  m_hHistCutPre->Write();
  m_hHistCut->Write();
}

//______________________________________________________________________________
TH1F* TauSelectionCut::CreateControlPlot(const char* sName, const char* sTitle, int iBins, double dXLow, double dXUp)
{
  if (m_tTST->m_bCreateControlPlots)
  {
    TH1F* hHist = new TH1F(sName, sTitle, iBins, dXLow, dXUp);
    hHist->SetDirectory(0);
    return hHist;
  }

  return nullptr;
}

//______________________________________________________________________________
void TauSelectionCut::fillHistogramCutPre(const xAOD::TauJet& xTau)
{
  fillHistogram(xTau, *m_hHistCutPre);
}

//______________________________________________________________________________
void TauSelectionCut::fillHistogramCut(const xAOD::TauJet& xTau)
{
  fillHistogram(xTau, *m_hHistCut);
}


//______________________________________________________________________________
void TauSelectionCut::setProperty(const std::string& name, const std::string& value)
{
  std::map<std::string, std::string&>::iterator it = m_mProperties.find(name);
  if(it == m_mProperties.end() )
    throw std::runtime_error (("Undeclared property: " + name + "\n").c_str());
  it->second = value;
}

//______________________________________________________________________________
void TauSelectionCut::declareProperty(const std::string& name, std::string& loc)
{
  std::pair<std::string, std::string&> p(name, loc);
  m_mProperties.insert(p);
}

//______________________________________________________________________________
std::string TauSelectionCut::getProperty(const std::string& name)
{
  std::map<std::string, std::string&>::iterator it = m_mProperties.find(name);
  if(it == m_mProperties.end() )
    throw std::runtime_error (("Undeclared property: " + name + "\n").c_str());

  return it->second;
}


//_______________________________SelectionCutPt_________________________________
//______________________________________________________________________________
TauSelectionCutPt::TauSelectionCutPt(TauSelectionTool* tTST)
  : TauSelectionCut("CutPt", tTST)
{
  m_hHistCutPre = CreateControlPlot("hPt_pre","Pt_pre;#tau-p_{T} [GeV]; events",100,0,100);
  m_hHistCut = CreateControlPlot("hPt_cut","Pt_cut;#tau-p_{T} [GeV]; events",100,0,100);
}

//______________________________________________________________________________
void TauSelectionCutPt::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  hHist.Fill(xTau.pt()/1000.);
}

//______________________________________________________________________________
void TauSelectionCutPt::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "Pt",
               "Selection of taus according to their transverse momentum" );
}
//______________________________________________________________________________
bool TauSelectionCutPt::accept(const xAOD::TauJet& xTau,
                            asg::AcceptData& acceptData)
{
  // save tau pt in GeV
  double pt = xTau.pt() / 1000.;
  // in case of only one entry in vector, run for lower limits
  if (m_tTST->m_vPtRegion.size() == 1)
  {
    if ( pt >= m_tTST->m_vPtRegion.at(0) )
    {
      acceptData.setCutResult( "Pt", true );
      return true;
    }
  }
  unsigned int iNumPtRegion = m_tTST->m_vPtRegion.size()/2;
  for( unsigned int iPtRegion = 0; iPtRegion < iNumPtRegion; iPtRegion++ )
  {
    if ( pt >= m_tTST->m_vPtRegion.at(iPtRegion*2) and pt <= m_tTST->m_vPtRegion.at(iPtRegion*2+1))
    {
      acceptData.setCutResult( "Pt", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed pt requirement, tau pt [GeV]: " << pt << endmsg;
  return false;
}

//_____________________________SelectionCutAbsEta_______________________________
//______________________________________________________________________________
TauSelectionCutAbsEta::TauSelectionCutAbsEta(TauSelectionTool* tTST)
  : TauSelectionCut("CutAbsEta", tTST)
{
  m_hHistCutPre = CreateControlPlot("hEta_pre","Eta_pre;#tau-#eta; events",100,-3,3);
  m_hHistCut = CreateControlPlot("hEta_cut","Eta_cut;#tau-#eta; events",100,-3,3);
}

//______________________________________________________________________________
void TauSelectionCutAbsEta::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  hHist.Fill(xTau.eta());
}

//______________________________________________________________________________
void TauSelectionCutAbsEta::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "AbsEta",
               "Selection of taus according to their absolute pseudorapidity" );
}
//______________________________________________________________________________
bool TauSelectionCutAbsEta::accept(const xAOD::TauJet& xTau,
                                asg::AcceptData& acceptData)
{
  // check regions of eta, if tau is in one region then return true; false otherwise
  unsigned int iNumEtaRegion = m_tTST->m_vAbsEtaRegion.size()/2;
  for( unsigned int iEtaRegion = 0; iEtaRegion < iNumEtaRegion; iEtaRegion++ )
  {
    if ( std::abs( xTau.eta() ) >= m_tTST->m_vAbsEtaRegion.at(iEtaRegion*2) and std::abs( xTau.eta() ) <= m_tTST->m_vAbsEtaRegion.at(iEtaRegion*2+1))
    {
      acceptData.setCutResult( "AbsEta", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed eta requirement, tau eta: " << xTau.eta() << endmsg;
  return false;
}

//____________________________SelectionCutAbsCharge_____________________________
//______________________________________________________________________________
TauSelectionCutAbsCharge::TauSelectionCutAbsCharge(TauSelectionTool* tTST)
  : TauSelectionCut("CutAbsCharge", tTST)
{
  m_hHistCutPre = CreateControlPlot("hCharge_pre","Charge_pre;charge; events",7,-3.5,3.5);
  m_hHistCut = CreateControlPlot("hCharge_cut","Charge_cut;charge; events",7,-3.5,3.5);
}

//______________________________________________________________________________
void TauSelectionCutAbsCharge::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  hHist.Fill(xTau.charge());
}

//______________________________________________________________________________
void TauSelectionCutAbsCharge::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "AbsCharge",
               "Selection of taus according to their absolute charge" );
}
//______________________________________________________________________________
bool TauSelectionCutAbsCharge::accept(const xAOD::TauJet& xTau,
                            asg::AcceptData& acceptData)
{
  // check charge, if tau has one of the charges requiered then return true; false otherwise
  for( unsigned int iCharge = 0; iCharge < m_tTST->m_vAbsCharges.size(); iCharge++ )
  {
    if ( std::abs( xTau.charge() ) == m_tTST->m_vAbsCharges.at(iCharge) )
    {
      acceptData.setCutResult( "AbsCharge", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed charge requirement, tau charge: " << xTau.charge() << endmsg;
  return false;
}

//_____________________________SelectionCutNTracks______________________________
//______________________________________________________________________________
TauSelectionCutNTracks::TauSelectionCutNTracks(TauSelectionTool* tTST)
  : TauSelectionCut("CutNTrack", tTST)
{
  m_hHistCutPre = CreateControlPlot("hNTrack_pre","NTrack_pre;number of tracks; events",22,-1.5,20.5);
  m_hHistCut = CreateControlPlot("hNTrack_cut","NTrack_cut;number of tracks; events",22,-1.5,20.5);
}

//______________________________________________________________________________
void TauSelectionCutNTracks::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  hHist.Fill(xTau.nTracks());
}

//______________________________________________________________________________
void TauSelectionCutNTracks::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "NTrack",
               "Selection of taus according to their number of associated tracks" );
}
//______________________________________________________________________________
bool TauSelectionCutNTracks::accept(const xAOD::TauJet& xTau,
                                 asg::AcceptData& acceptData)
{
  // check track multiplicity, if tau has one of the number of tracks requiered then return true; false otherwise
  for( size_t iNumTrack = 0; iNumTrack < m_tTST->m_vNTracks.size(); iNumTrack++ )
  {
    if ( static_cast<unsigned> (xTau.nTracks()) == m_tTST->m_vNTracks.at(iNumTrack) )
    {
      acceptData.setCutResult( "NTrack", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed nTracks requirement, tau number of tracks: " << xTau.nTracks() << endmsg;
  return false;
}

//___________________________SelectionCutRNNJetScoreSigTrans____________________________
//______________________________________________________________________________
TauSelectionCutRNNJetScoreSigTrans::TauSelectionCutRNNJetScoreSigTrans(TauSelectionTool* tTST)
  : TauSelectionCut("CutJetRNNScoreSigTrans", tTST)
{
  m_hHistCutPre = CreateControlPlot("hJetRNNSigTrans_pre","JetRNNSigTrans_pre;RNNJetSigTransScore; events",100,0,1);
  m_hHistCut = CreateControlPlot("hJetRNNSigTrans_cut","JetRNNSigTrans_cut;RNNJetSigTransScore; events",100,0,1);
}
//______________________________________________________________________________
void TauSelectionCutRNNJetScoreSigTrans::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  hHist.Fill(xTau.discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans));
}
//______________________________________________________________________________
void TauSelectionCutRNNJetScoreSigTrans::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "JetRNNScoreSigTrans",
               "Selection of taus according to their JetRNNScore" );
}
//______________________________________________________________________________
bool TauSelectionCutRNNJetScoreSigTrans::accept(const xAOD::TauJet& xTau,
                                             asg::AcceptData& acceptData)
{
  // check JetRNNscore, if tau has a JetRNN score in one of the regions requiered then return true; false otherwise
  double dJetRNNScoreSigTrans = xTau.discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
  unsigned int iNumJetRNNSigTransRegion = m_tTST->m_vJetRNNSigTransRegion.size()/2;
  for( unsigned int iJetRNNSigTransRegion = 0; iJetRNNSigTransRegion < iNumJetRNNSigTransRegion; iJetRNNSigTransRegion++ )
  {
    if ( dJetRNNScoreSigTrans >= m_tTST->m_vJetRNNSigTransRegion.at(iJetRNNSigTransRegion*2) and dJetRNNScoreSigTrans <= m_tTST->m_vJetRNNSigTransRegion.at(iJetRNNSigTransRegion*2+1))
    {
      acceptData.setCutResult( "JetRNNScoreSigTrans", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed JetRNNScore requirement, tau JetRNNScore: " << dJetRNNScoreSigTrans << endmsg;
  return false;
}

//_____________________________SelectionCutJetIDWP______________________________
//______________________________________________________________________________
TauSelectionCutJetIDWP::TauSelectionCutJetIDWP(TauSelectionTool* tTST)
  : TauSelectionCut("CutJetIDWP", tTST)
{
  m_hHistCutPre = CreateControlPlot("hJetIDWP_pre","JetIDWP_pre;; events",8,-.5,7.5);
  m_hHistCut = CreateControlPlot("hJetIDWP_cut","JetIDWP_cut;; events",8,-.5,7.5);
  // only proceed if histograms are defined
  if (!m_hHistCutPre or !m_hHistCut)
    return;

  m_hHistCutPre->GetXaxis()->SetBinLabel(1,"!VeryLoose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(2,"VeryLoose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(3,"!Loose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(4,"Loose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(5,"!Medium");
  m_hHistCutPre->GetXaxis()->SetBinLabel(6,"Medium");
  m_hHistCutPre->GetXaxis()->SetBinLabel(7,"!Tight");
  m_hHistCutPre->GetXaxis()->SetBinLabel(8,"Tight");
  m_hHistCut->GetXaxis()->SetBinLabel(1,"!VeryLoose");
  m_hHistCut->GetXaxis()->SetBinLabel(2,"VeryLoose");
  m_hHistCut->GetXaxis()->SetBinLabel(3,"!Loose");
  m_hHistCut->GetXaxis()->SetBinLabel(4,"Loose");
  m_hHistCut->GetXaxis()->SetBinLabel(5,"!Medium");
  m_hHistCut->GetXaxis()->SetBinLabel(6,"Medium");
  m_hHistCut->GetXaxis()->SetBinLabel(7,"!Tight");
  m_hHistCut->GetXaxis()->SetBinLabel(8,"Tight");
}

//______________________________________________________________________________
void TauSelectionCutJetIDWP::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  // FIXME: should this be extended to deepset ID?
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::JetRNNSigVeryLoose)); 
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::JetRNNSigLoose)+2);
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::JetRNNSigMedium)+4);
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::JetRNNSigTight)+6);
}

//______________________________________________________________________________
void TauSelectionCutJetIDWP::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "JetIDWP",
               "Selection of taus according to their JetIDScore" );
}
//______________________________________________________________________________
bool TauSelectionCutJetIDWP::accept(const xAOD::TauJet& xTau,
                                 asg::AcceptData& acceptData)
{
  // check Jet ID working point, if tau passes JetID working point then return true; false otherwise
  bool bPass = false;
  switch (m_tTST->m_iJetIDWP)
  {
  case JETIDNONE:
    bPass = true;
    break;
  case JETIDNONEUNCONFIGURED:
    bPass = true;
    break;
  case JETIDRNNVERYLOOSE:
    if (xTau.isTau(xAOD::TauJetParameters::JetRNNSigVeryLoose)) bPass = true;
    break;
  case JETIDRNNLOOSE:
    if (xTau.isTau(xAOD::TauJetParameters::JetRNNSigLoose)) bPass = true;
    break;
  case JETIDRNNMEDIUM:
    if (xTau.isTau(xAOD::TauJetParameters::JetRNNSigMedium)) bPass = true;
    break;
  case JETIDRNNTIGHT:
    if (xTau.isTau(xAOD::TauJetParameters::JetRNNSigTight)) bPass = true;
    break;
  case JETIDDEEPSETVERYLOOSE:
    static const SG::AuxElement::ConstAccessor<char> acc_deepSetVeryLoose("JetDeepSetVeryLoose");
    if (!acc_deepSetVeryLoose.isAvailable(xTau)) m_tTST->msg() << MSG::WARNING << "DeepSet VeryLoose WP not available" << endmsg;
    else bPass = acc_deepSetVeryLoose(xTau);
    break;
  case JETIDDEEPSETLOOSE:
    static const SG::AuxElement::ConstAccessor<char> acc_deepSetLoose("JetDeepSetLoose");
    if (!acc_deepSetLoose.isAvailable(xTau)) m_tTST->msg() << MSG::WARNING << "DeepSet Loose WP not available" << endmsg;
    else bPass = acc_deepSetLoose(xTau);
    break;
  case JETIDDEEPSETMEDIUM:
    static const SG::AuxElement::ConstAccessor<char> acc_deepSetMedium("JetDeepSetMedium");
    if (!acc_deepSetMedium.isAvailable(xTau)) m_tTST->msg() << MSG::WARNING << "DeepSet Medium WP not available" << endmsg;
    else bPass = acc_deepSetMedium(xTau);
    break;
  case JETIDDEEPSETTIGHT:
    static const SG::AuxElement::ConstAccessor<char> acc_deepSetTight("JetDeepSetTight");
    if (!acc_deepSetTight.isAvailable(xTau)) m_tTST->msg() << MSG::WARNING << "DeepSet Tight WP not available" << endmsg;
    else bPass = acc_deepSetTight(xTau);
    break;
  default:
    m_tTST->msg() << MSG::WARNING << "The jet ID working point with the enum " << m_tTST->m_iJetIDWP << " is not available" << endmsg;
    break;
  }
  if (bPass)
  {
    acceptData.setCutResult( "JetIDWP", true );
    return true;
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed JetIDWP requirement, tau transformed RNN score: " << xTau.discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans) << endmsg;
  return false;
}

//___________________________SelectionCutRNNEleScore____________________________
//______________________________________________________________________________
TauSelectionCutRNNEleScore::TauSelectionCutRNNEleScore(TauSelectionTool* tTST)
  : TauSelectionCut("CutEleRNNScore", tTST)
{
  m_hHistCutPre = CreateControlPlot("hEleRNN_pre","EleRNN_pre;RNNEleScore; events",100,0,1);
  m_hHistCut = CreateControlPlot("hEleRNN_cut","EleRNN_cut;RNNEleScore; events",100,0,1);
}

//______________________________________________________________________________
void TauSelectionCutRNNEleScore::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
   if(m_tTST->m_iEleIDVersion!=0){  
      hHist.Fill(xTau.auxdataConst<float>("RNNEleScoreSigTrans_v"+std::to_string(m_tTST->m_iEleIDVersion)));
   } else {
      hHist.Fill(xTau.discriminant(xAOD::TauJetParameters::RNNEleScoreSigTrans));
   }
}

//______________________________________________________________________________
void TauSelectionCutRNNEleScore::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "EleRNNScore",
               "Selection of taus according to their EleRNNScore" );
}
//______________________________________________________________________________
bool TauSelectionCutRNNEleScore::accept(const xAOD::TauJet& xTau,
                                     asg::AcceptData& acceptData)
{
  double fEleRNNScore = 0.;
  if(m_tTST->m_iEleIDVersion!=0){
    fEleRNNScore = xTau.auxdataConst<float>("RNNEleScoreSigTrans_v"+std::to_string(m_tTST->m_iEleIDVersion));
  }else{
    fEleRNNScore = xTau.discriminant(xAOD::TauJetParameters::RNNEleScoreSigTrans);
  }
  unsigned int iNumEleRNNRegion = m_tTST->m_vEleRNNRegion.size()/2;
  for( unsigned int iEleRNNRegion = 0; iEleRNNRegion < iNumEleRNNRegion; iEleRNNRegion++ )
  {
    if ( fEleRNNScore >= m_tTST->m_vEleRNNRegion.at(iEleRNNRegion*2) and fEleRNNScore <= m_tTST->m_vEleRNNRegion.at(iEleRNNRegion*2+1))
    {
      acceptData.setCutResult("EleRNNScore", true );
      return true;
    }
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed EleRNNScore requirement, tau EleRNNScore: " << fEleRNNScore << endmsg;
  return false;
}

//____________________________SelectionCutEleIDWP______________________________
//______________________________________________________________________________
TauSelectionCutEleIDWP::TauSelectionCutEleIDWP(TauSelectionTool* tTST)
  : TauSelectionCut("CutEleIDWP", tTST)
{
  m_hHistCutPre = CreateControlPlot("hEleIDWP_pre","EleIDWP_pre;; events",6,-.5,5.5);
  m_hHistCut = CreateControlPlot("hEleIDWP_cut","EleIDWP_cut;; events",6,-.5,5.5);
  // only proceed if histograms are defined
  if (!m_hHistCutPre or !m_hHistCut)
    return;
  m_hHistCutPre->GetXaxis()->SetBinLabel(1,"!Loose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(2,"Loose");
  m_hHistCutPre->GetXaxis()->SetBinLabel(3,"!Medium");
  m_hHistCutPre->GetXaxis()->SetBinLabel(4,"Medium");
  m_hHistCutPre->GetXaxis()->SetBinLabel(5,"!Tight");
  m_hHistCutPre->GetXaxis()->SetBinLabel(6,"Tight");
  m_hHistCut->GetXaxis()->SetBinLabel(1,"!Loose");
  m_hHistCut->GetXaxis()->SetBinLabel(2,"Loose");
  m_hHistCut->GetXaxis()->SetBinLabel(3,"!Medium");
  m_hHistCut->GetXaxis()->SetBinLabel(4,"Medium");
  m_hHistCut->GetXaxis()->SetBinLabel(5,"!Tight");
  m_hHistCut->GetXaxis()->SetBinLabel(6,"Tight");
}

//___________________________________________________________________________
void TauSelectionCutEleIDWP::fillHistogram(const xAOD::TauJet& xTau, TH1F& hHist) const
{
  if (m_tTST->m_iEleIDVersion!=0){
  hHist.Fill((xTau.auxdataConst<char>("EleRNNLoose_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1));
  hHist.Fill((xTau.auxdataConst<char>("EleRNNMedium_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1)+2);
  hHist.Fill((xTau.auxdataConst<char>("EleRNNTight_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1)+4);
  }
  else{
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::EleRNNLoose));
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::EleRNNMedium)+2);
  hHist.Fill(xTau.isTau(xAOD::TauJetParameters::EleRNNTight)+4);
  }
}

//______________________________________________________________________________
void TauSelectionCutEleIDWP::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "EleIDWP",
               "Selection of taus according to their EleID working point" );
}
//______________________________________________________________________________
bool TauSelectionCutEleIDWP::accept(const xAOD::TauJet& xTau,
				 asg::AcceptData& acceptData)
{
  // check EleID WP, if tau passes EleID working point then return true; false otherwise
  bool bPass = false;
  switch (m_tTST->m_iEleIDWP)
  {
  case ELEIDNONE:
    bPass = true;
    break;
  case ELEIDNONEUNCONFIGURED:
    bPass = true;
    break;
  case ELEIDRNNLOOSE:
    if (m_tTST->m_iEleIDVersion!=0){ 
      if (xTau.auxdataConst<char>("EleRNNLoose_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1)bPass = true;
    }
    else if (xTau.isTau(xAOD::TauJetParameters::EleRNNLoose)) bPass = true;
    break;
  case ELEIDRNNMEDIUM:
    if (m_tTST->m_iEleIDVersion!=0){ 
      if (xTau.auxdataConst<char>("EleRNNMedium_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1)bPass = true;
    }
    else if (xTau.isTau(xAOD::TauJetParameters::EleRNNMedium)) bPass = true;
    break;
  case ELEIDRNNTIGHT:
    if (m_tTST->m_iEleIDVersion!=0){ 
      if (xTau.auxdataConst<char>("EleRNNTight_v"+std::to_string(m_tTST->m_iEleIDVersion)) == 1)bPass = true;
    }
    else if (xTau.isTau(xAOD::TauJetParameters::EleRNNTight)) bPass = true;
    break;
  default:
    m_tTST->msg() << MSG::WARNING << "The electron ID working point with the enum " << m_tTST->m_iEleIDWP << " is not available" << endmsg;
    break;
  }
    
  if (bPass)
  {
    acceptData.setCutResult( "EleIDWP", true );
    return true;
  }
  m_tTST->msg() << MSG::VERBOSE << "Tau failed EleID WP requirement" << endmsg;
  return false;
}

//added by Li-Gang Xia < ligang.xia@cern.ch >
//____________________________SelectionCutMuonOLR______________________________
//______________________________________________________________________________
TauSelectionCutMuonOLR::TauSelectionCutMuonOLR(TauSelectionTool* tTST)
  : TauSelectionCut("CutMuonOLR", tTST)
  , m_bTauMuonOLR(true)
{
  m_hHistCutPre = CreateControlPlot("hMuonOLR_pre","MuonOLR_pre;; events",2,-.5,1.5);
  m_hHistCut = CreateControlPlot("hMuonOLR_cut","MuonOLR_cut;; events",2,-.5,1.5);
  // only proceed if histograms are defined
  if (!m_hHistCutPre or !m_hHistCut)
    return;
  m_hHistCutPre->GetXaxis()->SetBinLabel(1,"!MuonOLR");
  m_hHistCutPre->GetXaxis()->SetBinLabel(2,"MuonOLR");
  m_hHistCut->GetXaxis()->SetBinLabel(1,"!MuonOLR");
  m_hHistCut->GetXaxis()->SetBinLabel(2,"MuonOLR");
}

//______________________________________________________________________________
void TauSelectionCutMuonOLR::fillHistogram(const xAOD::TauJet& /*xTau*/, TH1F& hHist) const
{
  hHist.Fill(m_bTauMuonOLR);
}

//______________________________________________________________________________
void TauSelectionCutMuonOLR::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "MuonOLR",
               "Selection of taus according to their MuonOLR" );
}
//______________________________________________________________________________
bool TauSelectionCutMuonOLR::accept(const xAOD::TauJet& xTau,
                                 asg::AcceptData& acceptData)
{
  if (!m_tTST->m_bMuonOLR)
  {
    acceptData.setCutResult( "MuonOLR", true );
    return true;
  }

  // MuonOLR : removing tau overlapped with muon satisfying pt>2GeV and not calo-tagged
  m_bTauMuonOLR = true;

  SG::ReadHandle<xAOD::MuonContainer> muonContainerHandle( m_tTST->m_muonContainerKey );
  if (!muonContainerHandle.isValid()) {
    m_tTST->msg() << MSG::ERROR << "Could not retrieve xAOD::MuonContainer with key " << muonContainerHandle.key() << endmsg;
    return false;
  }
  const xAOD::MuonContainer* muonContainer = muonContainerHandle.cptr();

  for( auto xMuon : *muonContainer )
  {
    if(xMuon->pt() < 2000.) continue; // pt > 2 GeV
    if(xMuon->muonType() == xAOD::Muon::CaloTagged) continue; // not calo-tagged
    if(xMuon->p4().DeltaR( xTau.p4() ) > 0.2 ) continue; // delta R < 0.2
    m_bTauMuonOLR = false; // muon-tau overlapped
    break;
  }
  if(m_bTauMuonOLR)
  {
    acceptData.setCutResult( "MuonOLR", true );
    return true;
  }

  m_tTST->msg() << MSG::VERBOSE << "Tau failed MuonOLR requirement" << endmsg;
  return false;
}
