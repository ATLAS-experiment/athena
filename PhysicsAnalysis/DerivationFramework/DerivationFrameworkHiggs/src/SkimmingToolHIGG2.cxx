/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// SkimmingToolHIGG2.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Susumu Oda (Susumu.Oda@cern.ch)
// Based on DerivationFramework::SkimmingToolExample

#include "DerivationFrameworkHiggs/SkimmingToolHIGG2.h"
#include <vector>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "AthContainers/ConstAccessor.h"
#include "TruthUtils/ParticleConstants.h"


// Constructor
DerivationFramework::SkimmingToolHIGG2::SkimmingToolHIGG2(const std::string& t,
							  const std::string& n,
							  const IInterface* p) : 
  base_class(t, n, p),
  m_trigDecisionTool("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_ntot(0),
  m_npass(0)
{

  declareProperty("SkipTriggerRequirement", m_skipTriggerRequirement=false);

  declareProperty("FilterType", m_filterType="2L");

  declareProperty("ElectronContainerKey", m_electronSGKey="Electrons");
  declareProperty("MuonContainerKey", m_muonSGKey="Muons");
  declareProperty("JetContainerKey", m_jetSGKey="AntiKt4EMTopoJets");
  declareProperty("MergedJetContainerKey0", m_mergedJetSGKey[0]="AntiKt4EMTopoJets");
  declareProperty("MergedJetContainerKey1", m_mergedJetSGKey[1]="AntiKt10LCTopoJets");
  declareProperty("MergedJetContainerKey2", m_mergedJetSGKey[2]="CamKt12LCTopoJets");
  declareProperty("PhotonContainerKey", m_photonSGKey="Photons");
  declareProperty("TrackContainerKey", m_trackSGKey="InDetTrackParticles");

  declareProperty("NumberOfLeptons", m_nLeptons=2);
  declareProperty("NumberOfElectrons", m_nElectrons=0);
  declareProperty("NumberOfMuons", m_nMuons=0);
  declareProperty("NumberOfJets", m_nJets=0);
  declareProperty("NumberOfMergedJets0", m_nMergedJets[0]=0);
  declareProperty("NumberOfMergedJets1", m_nMergedJets[1]=0);
  declareProperty("NumberOfMergedJets2", m_nMergedJets[2]=0);
  declareProperty("NumberOfPhotons", m_nPhotons=0);
  declareProperty("NumberOfTracks", m_nTracks=0);

  declareProperty("ElectronQuality", m_electronQual="any");
  declareProperty("MuonQuality", m_muonQual="inMS");
  declareProperty("TightElectronQuality", m_tightElectronQual="DFCommonElectronsLHVeryLoose");
  declareProperty("TightMuonQuality", m_tightMuonQual="DFCommonGoodMuon");
  declareProperty("JetQuality", m_jetQual="any");
  declareProperty("MergedJetQuality0", m_mergedJetQual[0]="any");
  declareProperty("MergedJetQuality1", m_mergedJetQual[1]="any");
  declareProperty("MergedJetQuality2", m_mergedJetQual[2]="any");
  declareProperty("PhotonQuality", m_photonQual="any");

  declareProperty("PrimaryElectronQuality4L", m_primaryElectronQual4L="DFCommonElectronsLHLoose");

  declareProperty("Trigger2L", m_trigger2L=std::vector<std::string>()); 
  declareProperty("TriggerTP", m_triggerTP=std::vector<std::string>()); 
  declareProperty("Trigger2L2Q", m_trigger2L2Q=std::vector<std::string>()); 
  declareProperty("TriggerJPSI", m_triggerJPSI=std::vector<std::string>()); 
  declareProperty("TriggerPHI", m_triggerPHI=std::vector<std::string>()); 

  declareProperty("ElectronEtCut", m_electronEtCut=10.*CLHEP::GeV); 
  declareProperty("MuonPtCut", m_muonPtCut=10.*CLHEP::GeV); 
  declareProperty("TightElectronEtCut", m_tightElectronEtCut=15.*CLHEP::GeV); 
  declareProperty("TightMuonPtCut", m_tightMuonPtCut=15.*CLHEP::GeV); 
  declareProperty("JetPtCut", m_jetPtCut=15.*CLHEP::GeV); 
  declareProperty("MergedJetPtCut0", m_mergedJetPtCut[0]=100.*CLHEP::GeV); 
  declareProperty("MergedJetPtCut1", m_mergedJetPtCut[1]=150.*CLHEP::GeV); 
  declareProperty("MergedJetPtCut2", m_mergedJetPtCut[2]=150.*CLHEP::GeV); 
  declareProperty("PhotonPtCut", m_photonPtCut=15.*CLHEP::GeV); 
  declareProperty("TrackPtCut", m_trackPtCut=20.*CLHEP::GeV); 

  declareProperty("ElectronEtaCut", m_electronEtaCut=9999.); 
  declareProperty("MuonEtaCut", m_muonEtaCut=9999.); 
  declareProperty("CaloMuonEtaCut", m_caloMuonEtaCut=0.2); 
  declareProperty("JetEtaCut", m_jetEtaCut=2.6); 
  declareProperty("MergedJetEtaCut0", m_mergedJetEtaCut[0]=2.6); 
  declareProperty("MergedJetEtaCut1", m_mergedJetEtaCut[1]=2.6); 
  declareProperty("MergedJetEtaCut2", m_mergedJetEtaCut[2]=2.6); 
  declareProperty("PhotonEtaCut", m_photonEtaCut=2.5); 

  declareProperty("InvariantMassCut", m_invariantMassCut=0.*CLHEP::GeV); 
  declareProperty("InvariantMassJpsiLowCut", m_invariantMassJpsiLowCut=2.0*CLHEP::GeV); 
  declareProperty("InvariantMassJpsiUpCut", m_invariantMassJpsiUpCut=4.3*CLHEP::GeV); 
  declareProperty("InvariantMassUpsilonLowCut", m_invariantMassUpsilonLowCut=8.0*CLHEP::GeV); 
  declareProperty("InvariantMassUpsilonUpCut", m_invariantMassUpsilonUpCut=12.0*CLHEP::GeV); 
  declareProperty("InvariantMassPhiLowCut", m_invariantMassPhiLowCut=2.*s_MKplus); 
  declareProperty("InvariantMassPhiUpCut", m_invariantMassPhiUpCut=1.2*CLHEP::GeV); 

  declareProperty("LeadingElectronEtCut", m_leadingElectronEtCut=0.*CLHEP::GeV); 
  declareProperty("LeadingMuonPtCut", m_leadingMuonPtCut=0.*CLHEP::GeV); 

  declareProperty("DRElectronJetCut", m_dRElectronJetCut=0.05);   

  declareProperty("CheckLArError", m_checkLArError=true);	
  declareProperty("UseDefaultElectronFourMomentum", m_defaultElectronFourMomentum=true);

  declareProperty("UseDFCommonJetFourMomentum", m_DFCommonJetFourMomentum=true);

  declareProperty("RequireTightLeptons", m_requireTightLeptons=false);
}
  
// Destructor
DerivationFramework::SkimmingToolHIGG2::~SkimmingToolHIGG2() {
}  

// Athena initialize and finalize
StatusCode DerivationFramework::SkimmingToolHIGG2::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  // if(m_defaultElectronFourMomentum==true and m_DFCommonElectronFourMomentum==true) {
  //   ATH_MSG_FATAL("Wrong setting both m_defaultElectronFourMomentum and m_DFCommonElectronFourMomentum are true.");
  //   return StatusCode::FAILURE;
  // }

  // trigger decision tool
  if(m_trigDecisionTool.retrieve(DisableTool{m_skipTriggerRequirement}).isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool: " << m_trigDecisionTool);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Retrieved tool: " << m_trigDecisionTool);

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::SkimmingToolHIGG2::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed " << m_ntot << " events, " << m_npass << " events passed filter ");
  return StatusCode::SUCCESS;
}

// The filter itself
bool DerivationFramework::SkimmingToolHIGG2::eventPassesFilter() const
{
  m_ntot++;
  bool acceptEvent(false);
  StatusCode sc(StatusCode::SUCCESS);

  // Retrieve EventInfo
  const xAOD::EventInfo *eventInfo(nullptr);
  ATH_CHECK(evtStore()->retrieve(eventInfo), false);

  // LAr error check
  if(m_checkLArError) {
    if(eventInfo->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error) {
      return acceptEvent; 
    }
  }

  DerivationFramework::SkimmingToolHIGG2::Candidates evt;
  // Electrons
  if(m_nElectrons>0 or m_nLeptons>0) {
    const xAOD::ElectronContainer *electrons(nullptr); 
    ATH_CHECK(evtStore()->retrieve(electrons, m_electronSGKey), false);
    for(const auto *el: *electrons) {
      if(this->checkElectronQuality(el)) evt.goodElectrons.push_back(el);
    } 
  }

  // Muons
  if(m_nMuons>0 or m_nLeptons>0) {
    const xAOD::MuonContainer *muons(nullptr); 
    ATH_CHECK(evtStore()->retrieve(muons, m_muonSGKey), false);
    for(const auto *mu: *muons) {
      if(this->checkMuonQuality(mu)) evt.goodMuons.push_back(mu);
    } 
  }

  // Jets
  if(m_nJets>0) {
    const xAOD::JetContainer *jets(nullptr); 
    ATH_CHECK(evtStore()->retrieve(jets, m_jetSGKey), false);
    for(const auto *jet: *jets) {
      if(this->checkJetQuality(jet)) evt.goodJets.push_back(jet);
     }
  }
  for(unsigned int type(0); type<NUMBER_OF_MERGED_JET_TYPES; type++) {
    if(m_nMergedJets[type]>0) {
      const xAOD::JetContainer *jets(nullptr);
      ATH_CHECK(evtStore()->retrieve(jets, m_mergedJetSGKey[type]), false);
      for(const auto *jet: *jets) {
	if(this->checkMergedJetQuality(jet, type)) evt.goodMergedJets[type].push_back(jet);
      }
    }
  }

  // Photons
  if(m_nPhotons>0) {
    const xAOD::PhotonContainer *photons(nullptr); 
    ATH_CHECK(evtStore()->retrieve(photons, m_photonSGKey), false);
    for(const auto *ph: *photons) {
      if(this->checkPhotonQuality(ph)) evt.goodPhotons.push_back(ph);
    }
  }

  // Tracks
  if(m_nTracks>0) {
    const xAOD::TrackParticleContainer *tracks(nullptr);
    ATH_CHECK(evtStore()->retrieve(tracks, m_trackSGKey), false);
    for(const auto *trk: *tracks) {
      if(this->checkTrackQuality(trk)) evt.goodTracks.push_back(trk);
    }
  }

  // Check conditions of filters
  if(m_filterType=="2L") {
    if(this->check2L(evt)) acceptEvent = true;
  } else if(m_filterType=="4L") {
    if(this->check4L(evt)) acceptEvent = true;
  } else if(m_filterType=="TP") {
    if(this->checkTP(evt)) acceptEvent = true;
  } else if(m_filterType=="2L2Q") {
    if(this->check2L2Q(evt)) acceptEvent = true;
  } else if(m_filterType=="JPSI") {
    if(this->checkJPSI(evt) or this->checkPHI(evt)) acceptEvent = true;
  }

  if(acceptEvent) m_npass++;
  return acceptEvent; 
}  

bool DerivationFramework::SkimmingToolHIGG2::checkElectronQuality(const xAOD::Electron *el, const bool isTight) const 
{
  if(!el) return false;

  const std::string electronQual(isTight ? m_tightElectronQual : m_electronQual);
  const double electronEtCut(isTight ? m_tightElectronEtCut : m_electronEtCut);

  if(electronQual!="any") {
    bool value(false);
    bool defined(false);
    if(electronQual=="DFCommonElectronsLHVeryLoose" or
       electronQual=="DFCommonElectronsLHLoose"     or
       electronQual=="DFCommonElectronsLHMedium"    or
       electronQual=="DFCommonElectronsLHTight"     or
       electronQual=="DFCommonElectronsML") {
      // Use Derivation Framework variable to pickup possible new correction result
      // If the same electron likelihood configuration is used in AOD creation and DAOD creation, 
      // DFCommonElectronsLHLoose and LHLoose are identical.
      SG::ConstAccessor<char> qualAcc(electronQual);
      if(qualAcc.isAvailable(*el)) {
	defined = true;
	value = static_cast<bool>(qualAcc(*el));
      }
    } else {
      // Electron menu is defined in http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Reconstruction/egamma/egammaTools/python/EMPIDBuilderBase.py
      defined = el->passSelection(value, electronQual);
    }
    if(not(value and defined)) return false;
  }

  const xAOD::TrackParticle *trackParticle(el->trackParticle());
  if(!trackParticle) {
    ATH_MSG_DEBUG("xAOD::TrackParticle pointer is null");
    return false;
  }
  uint8_t numberOfPixelHits(0);
  if(!trackParticle->summaryValue(numberOfPixelHits, xAOD::numberOfPixelHits)) {
    ATH_MSG_WARNING("xAOD::TrackParticle does not give summaryValue correctly for xAOD::numberOfPixelHits");
    return false;
  }
  uint8_t numberOfSCTHits(0);
  if(!trackParticle->summaryValue(numberOfSCTHits, xAOD::numberOfSCTHits)) {
    ATH_MSG_WARNING("xAOD::TrackParticle does not give summaryValue correctly for xAOD::numberOfSCTHits");
    return false;
  }
  if(numberOfPixelHits+numberOfSCTHits==0) return false; 

  const xAOD::CaloCluster *caloCluster(el->caloCluster());
  if(!caloCluster) {
    ATH_MSG_WARNING("xAOD::CaloCluster pointer is null");
    return false;
  }

  double eta(trackParticle->eta());
  if(fabs(eta)>10.) return false;
  double et(caloCluster->e()/cosh(eta));

  if(et<electronEtCut) return false;
  if(fabs(eta)>m_electronEtaCut) return false;

  return true;
}

bool DerivationFramework::SkimmingToolHIGG2::checkMuonQuality(const xAOD::Muon *mu, const bool isTight) const 
{
  if(!mu) return false;

  const std::string muonQual(isTight ? m_tightMuonQual : m_muonQual);
  const double muonPtCut(isTight ? m_tightMuonPtCut : m_muonPtCut);

  if(muonQual=="any") {
    // do nothing
  } else if(muonQual=="combined") {
    if(mu->muonType()!=xAOD::Muon::Combined) return false;
  } else if(muonQual=="standalone") {
    if(mu->muonType()!=xAOD::Muon::MuonStandAlone) return false;
  } else if(muonQual=="lowpt") {
    if(mu->muonType()!=xAOD::Muon::SegmentTagged) return false;
  } else if(muonQual=="combined+lowpt") {
    if(mu->muonType()!=xAOD::Muon::Combined and mu->muonType()!=xAOD::Muon::SegmentTagged) return false;
  } else if(muonQual=="inMS") {
    if(mu->muonType()==xAOD::Muon::MuonStandAlone and fabs(fabs(mu->eta())-2.6)>0.12) return false;
  } else if(muonQual=="DFCommonGoodMuon") { // Derivation Framework variable
    static const SG::ConstAccessor<char> DFCommonGoodMuonAcc("DFCommonGoodMuon");
    if(!DFCommonGoodMuonAcc.withDefault(*mu, false)) return false;
  } else if(muonQual=="DFCommonMuonsLoose") { // Derivation Framework variable
    static const SG::ConstAccessor<char> DFCommonMuonsLooseAcc("DFCommonMuonsLoose");
    if(!DFCommonMuonsLooseAcc.withDefault(*mu, false)) return false;
  } else if(muonQual=="DFCommonMuonsMedium") { // Derivation Framework variable
    static const SG::ConstAccessor<char> DFCommonMuonsMediumAcc("DFCommonMuonsMedium");
    if(!DFCommonMuonsMediumAcc.withDefault(*mu, false)) return false;
  } else if(muonQual=="DFCommonMuonsTight") { // Derivation Framework variable
    static const SG::ConstAccessor<char> DFCommonMuonsTightAcc("DFCommonMuonsTight");
    if(!DFCommonMuonsTightAcc.withDefault(*mu, false)) return false;
  } else {
    ATH_MSG_WARNING("Muon quality " << muonQual << "is not defined");
    return false; 
  }

  if(mu->pt()<muonPtCut) return false;
  if(fabs(mu->eta())>m_muonEtaCut) return false;
  if(muonQual!="DFCommonGoodMuon" and
     (mu->muonType()==xAOD::Muon::CaloTagged and fabs(mu->eta())>m_caloMuonEtaCut)) return false;

  return true;
}
  
bool DerivationFramework::SkimmingToolHIGG2::checkJetQuality(const xAOD::Jet *jet) const 
{
  if(!jet) return false;

  if(m_jetQual!="any") {
    // do something
  }

  TLorentzVector tlv(this->jetFourMomentum(jet));
  if(tlv.Pt()<m_jetPtCut) return false;
  if(fabs(tlv.Eta())>m_jetEtaCut) return false;

  return true;
}

bool DerivationFramework::SkimmingToolHIGG2::checkMergedJetQuality(const xAOD::Jet *jet, const unsigned int type) const 
{
  if(!jet) return false;

  if(m_mergedJetQual[type]!="any") {
    // do something
  }

  if(jet->pt()<m_mergedJetPtCut[type]) return false;
  if(fabs(jet->eta())>m_mergedJetEtaCut[type]) return false;

  return true;
}

bool DerivationFramework::SkimmingToolHIGG2::checkPhotonQuality(const xAOD::Photon *ph) const 
{
  if(!ph) return false;

  if(m_photonQual!="any") {
    bool value(false);
    if(!ph->passSelection(value, m_photonQual)) {
      ATH_MSG_WARNING("xAOD::Photon does not have menu of " << m_photonQual);
      return false;
    } 
    if(!value) return false;
  }

  if(ph->pt()<m_photonPtCut) return false;
  if(fabs(ph->eta())>m_photonEtaCut) return false;

  return true;
}

bool DerivationFramework::SkimmingToolHIGG2::checkTrackQuality(const xAOD::TrackParticle *trk) const 
{
  if(!trk) return false;

  if(trk->pt()<m_trackPtCut) return false;

  return true;
}

bool DerivationFramework::SkimmingToolHIGG2::check2L(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  if(!(m_nLeptons>0 and evt.goodElectrons.size()+evt.goodMuons.size()>=m_nLeptons)) return false;
  if(!(evt.goodJets.size()>=m_nJets and evt.goodPhotons.size()>=m_nPhotons)) return false;

  bool isTriggerFired(m_trigger2L.empty() or m_skipTriggerRequirement);
  for(unsigned int i(0); i<m_trigger2L.size(); i++) {
    if(m_trigDecisionTool->isPassed(m_trigger2L.at(i))) {
      isTriggerFired = true;
      break;
    }
  }
  if(!isTriggerFired) return false; 

  unsigned int nGoodLeptons(evt.goodElectrons.size()+evt.goodMuons.size());
  std::vector<TLorentzVector> v_tlv(nGoodLeptons);
  std::vector<bool> v_isElectron(nGoodLeptons);
  std::vector<bool> v_isTight(nGoodLeptons);
  
  for(unsigned int el_i(0); el_i<evt.goodElectrons.size(); el_i++) {
    const xAOD::Electron *el(evt.goodElectrons.at(el_i));
    TLorentzVector tlv(this->electronFourMomentum(el));
    v_tlv.at(el_i) = tlv;
    v_isElectron.at(el_i) = true;
    v_isTight.at(el_i) = this->checkElectronQuality(el, true);
  }
  
  for(unsigned int mu_i(0); mu_i<evt.goodMuons.size(); mu_i++) {
    const xAOD::Muon *mu(evt.goodMuons.at(mu_i));
    TLorentzVector tlv(DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(mu));
    unsigned int mu_j(evt.goodElectrons.size()+mu_i);
    v_tlv.at(mu_j) = tlv;
    v_isElectron.at(mu_j) = false;
    v_isTight.at(mu_j) = this->checkMuonQuality(mu, true);
  }
  
  for(unsigned int i0(0); i0<nGoodLeptons; i0++) {
    for(unsigned int i1(i0+1); i1<nGoodLeptons; i1++) {
      if(m_requireTightLeptons and (not (v_isTight.at(i0) or v_isTight.at(i1)))) continue; 

      TLorentzVector tlv_2lep(v_tlv.at(i0) + v_tlv.at(i1));
      // Check two lepton mass
      if(tlv_2lep.M()>m_invariantMassCut) return true;
    }
  }

  return false;
}

bool DerivationFramework::SkimmingToolHIGG2::check4L(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  if(!(m_nLeptons>0 and evt.goodElectrons.size()+evt.goodMuons.size()>=m_nLeptons)) return false;
  if(!(evt.goodJets.size()>=m_nJets and evt.goodPhotons.size()>=m_nPhotons)) return false;

  unsigned int nGoodLeptons(evt.goodElectrons.size()+evt.goodMuons.size());
  std::vector<TLorentzVector> v_tlv(nGoodLeptons);
  std::vector<bool> v_pid(nGoodLeptons);
  
  for(unsigned int el_i(0); el_i<evt.goodElectrons.size(); el_i++) {
    const xAOD::Electron *el(evt.goodElectrons.at(el_i));
    TLorentzVector tlv(this->electronFourMomentum(el));
    v_tlv.at(el_i) = tlv;

    bool value(false);
    bool defined(false);
    if(m_primaryElectronQual4L.empty()) {
      // In the case of no identification requirement
      defined = true;
      value = true;
    } else if(m_primaryElectronQual4L=="DFCommonElectronsLHVeryLoose" or
	      m_primaryElectronQual4L=="DFCommonElectronsLHLoose"     or
	      m_primaryElectronQual4L=="DFCommonElectronsLHMedium"    or
	      m_primaryElectronQual4L=="DFCommonElectronsLHTight"     or
	      m_primaryElectronQual4L=="DFCommonElectronsML") {
      // Use Derivation Framework variable to pickup possible new correction result
      // If the same electron likelihood configuration is used in AOD creation and DAOD creation, 
      // DFCommonElectronsLHLoose and LHLoose are identical.
      SG::ConstAccessor<char> primEleAcc(m_primaryElectronQual4L);
      if(primEleAcc.isAvailable(*el)) {
	defined = true;
	value = static_cast<bool>(primEleAcc(*el));
      }
    } else {
      // Electron menu is defined in http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Reconstruction/egamma/egammaTools/python/EMPIDBuilderBase.py
      defined = el->passSelection(value, m_primaryElectronQual4L);
    }

    v_pid.at(el_i) = (value and defined);
  }
  
  for(unsigned int mu_i(0); mu_i<evt.goodMuons.size(); mu_i++) {
    const xAOD::Muon *mu(evt.goodMuons.at(mu_i));
    TLorentzVector tlv(DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(mu));
    unsigned int mu_j(evt.goodElectrons.size()+mu_i);
    v_tlv.at(mu_j) = tlv;
    v_pid.at(mu_j) = true;
  }
  
  for(unsigned int i0(0); i0<nGoodLeptons; i0++) {
    for(unsigned int i1(i0+1); i1<nGoodLeptons; i1++) {
      for(unsigned int i2(i1+1); i2<nGoodLeptons; i2++) {
	for(unsigned int i3(i2+1); i3<nGoodLeptons; i3++) {
	  TLorentzVector tlv_4lep(v_tlv.at(i0) + v_tlv.at(i1) + v_tlv.at(i2) + v_tlv.at(i3));
	  // Check four lepton mass
	  if(tlv_4lep.M()<m_invariantMassCut) continue;
	  
	  // Check primary dilepton's electron PID (muon PID is always true)
	  if(fabs((v_tlv.at(i0)+v_tlv.at(i1)).M()-s_MZ)<fabs((v_tlv.at(i2)+v_tlv.at(i3)).M()-s_MZ)) {
	    if(v_pid.at(i0) and v_pid.at(i1)) return true; 
	  } else {
	    if(v_pid.at(i2) and v_pid.at(i3)) return true;
	  }
	  if(fabs((v_tlv.at(i0)+v_tlv.at(i2)).M()-s_MZ)<fabs((v_tlv.at(i1)+v_tlv.at(i3)).M()-s_MZ)) {
	    if(v_pid.at(i0) and v_pid.at(i2)) return true;
	  } else {
	    if(v_pid.at(i1) and v_pid.at(i3)) return true;
	  }
	  if(fabs((v_tlv.at(i0)+v_tlv.at(i3)).M()-s_MZ)<fabs((v_tlv.at(i1)+v_tlv.at(i2)).M()-s_MZ)) {
	    if(v_pid.at(i0) and v_pid.at(i3)) return true;
	  } else {
	    if(v_pid.at(i1) and v_pid.at(i2)) return true;
	  }
	}
      }
    } 
  }

  return false;
}

bool DerivationFramework::SkimmingToolHIGG2::checkTP(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  if(!(m_nLeptons>0 and evt.goodElectrons.size()+evt.goodMuons.size()>=m_nLeptons)) return false;

  bool isTriggerFired(m_triggerTP.empty() or m_skipTriggerRequirement);
  for(unsigned int i(0); i<m_triggerTP.size(); i++) {
    if(m_trigDecisionTool->isPassed(m_triggerTP.at(i))) {
      isTriggerFired = true;
      break;
    }
  }
  if(!isTriggerFired) return false; 

  unsigned int nGoodLeptons(evt.goodElectrons.size()+evt.goodMuons.size());
  std::vector<TLorentzVector> v_tlv(nGoodLeptons);
  std::vector<bool> v_isElectron(nGoodLeptons);
  
  for(unsigned int el_i(0); el_i<evt.goodElectrons.size(); el_i++) {
    const xAOD::Electron *el(evt.goodElectrons.at(el_i));
    TLorentzVector tlv(this->electronFourMomentum(el));
    v_tlv.at(el_i) = tlv;
    v_isElectron.at(el_i) = true;
  }
  
  for(unsigned int mu_i(0); mu_i<evt.goodMuons.size(); mu_i++) {
    const xAOD::Muon *mu(evt.goodMuons.at(mu_i));
    TLorentzVector tlv(DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(mu));
    unsigned int mu_j(evt.goodElectrons.size()+mu_i);
    v_tlv.at(mu_j) = tlv;
    v_isElectron.at(mu_j) = false;
  }
  
  for(unsigned int i0(0); i0<nGoodLeptons; i0++) {
    for(unsigned int i1(i0+1); i1<nGoodLeptons; i1++) {
      if(v_isElectron.at(i0)!=v_isElectron.at(i1)) continue;
      
      TLorentzVector tlv_2lep(v_tlv.at(i0) + v_tlv.at(i1));
      // Check two lepton mass
      if(tlv_2lep.M()<m_invariantMassCut) continue;

      // Leading lepton ET/pT cut
      if((v_tlv.at(i0).Pt()>v_tlv.at(i1).Pt() ? v_tlv.at(i0).Pt() : v_tlv.at(i1).Pt()) > (v_isElectron.at(i0) ? m_leadingElectronEtCut : m_leadingMuonPtCut)) return true;
    }
  }

  return false;
}

bool DerivationFramework::SkimmingToolHIGG2::check2L2Q(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  if(!(m_nLeptons>0 and evt.goodElectrons.size()+evt.goodMuons.size()>=m_nLeptons)) return false;

  bool isTriggerFired(m_trigger2L2Q.empty() or m_skipTriggerRequirement);
  for(unsigned int i(0); i<m_trigger2L2Q.size(); i++) {
    if(m_trigDecisionTool->isPassed(m_trigger2L2Q.at(i))) {
      isTriggerFired = true;
      break;
    }
  }
  if(!isTriggerFired) return false;

  bool checkGoodJets(evt.goodJets.size()>=m_nJets and m_nJets>0);
  for(unsigned int type(0); type<NUMBER_OF_MERGED_JET_TYPES; type++) {
    if(m_nMergedJets[type]>0) {
      checkGoodJets = (checkGoodJets or (evt.goodMergedJets[type].size()>=m_nMergedJets[type]));
    }
  }
  if(!checkGoodJets) return false;

  unsigned int nGoodLeptons(evt.goodElectrons.size()+evt.goodMuons.size());
  std::vector<TLorentzVector> v_tlv(nGoodLeptons);
  std::vector<bool> v_isElectron(nGoodLeptons);
  std::vector<bool> v_isTight(nGoodLeptons);
  
  for(unsigned int el_i(0); el_i<evt.goodElectrons.size(); el_i++) {
    const xAOD::Electron *el(evt.goodElectrons.at(el_i));
    TLorentzVector tlv(this->electronFourMomentum(el));
    v_tlv.at(el_i) = tlv;
    v_isElectron.at(el_i) = true;
    v_isTight.at(el_i) = this->checkElectronQuality(el, true);
  }
  
  for(unsigned int mu_i(0); mu_i<evt.goodMuons.size(); mu_i++) {
    const xAOD::Muon *mu(evt.goodMuons.at(mu_i));
    TLorentzVector tlv(DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(mu));
    unsigned int mu_j(evt.goodElectrons.size()+mu_i);
    v_tlv.at(mu_j) = tlv;
    v_isElectron.at(mu_j) = false;
    v_isTight.at(mu_j) = this->checkMuonQuality(mu, true);
  }
  
  for(unsigned int i0(0); i0<nGoodLeptons; i0++) {
    for(unsigned int i1(i0+1); i1<nGoodLeptons; i1++) {
      //      if(v_isElectron.at(i0)!=v_isElectron.at(i1)) continue; // Events with N(e)=1 and N(mu)=1 are kept. 
      if(m_requireTightLeptons and (not (v_isTight.at(i0) or v_isTight.at(i1)))) continue;
      
      TLorentzVector tlv_2lep(v_tlv.at(i0) + v_tlv.at(i1));
      // Check two lepton mass
      if(tlv_2lep.M()<m_invariantMassCut) continue;

      // dR(e-jet)>0.05 is required for at least two jets
      if(v_isElectron.at(i0)) {
	unsigned int nGoodJetsWithDRCut(0);
	unsigned int nGoodJets(evt.goodJets.size());
	for(unsigned int j(0); j<nGoodJets; j++) {
	  const xAOD::Jet *jet(evt.goodJets.at(j));
	  TLorentzVector jet_tlv(this->jetFourMomentum(jet));

	  double dR_0(DerivationFramework::SkimmingToolHIGG2::getDeltaR(v_tlv.at(i0).Eta(), v_tlv.at(i0).Phi(), jet_tlv.Eta(), jet_tlv.Phi()));
	  if(dR_0<m_dRElectronJetCut) continue;
	  
	  double dR_1(DerivationFramework::SkimmingToolHIGG2::getDeltaR(v_tlv.at(i1).Eta(), v_tlv.at(i1).Phi(), jet_tlv.Eta(), jet_tlv.Phi()));
	  if(dR_1<m_dRElectronJetCut) continue;
	  
	  nGoodJetsWithDRCut++;
	}

	bool checkGoodJetsWithDRCut(nGoodJetsWithDRCut>=m_nJets and m_nJets>0);

	for(unsigned int type(0); type<NUMBER_OF_MERGED_JET_TYPES; type++) {
	  if(m_nMergedJets[type]>0) {
	    unsigned int nGoodMergedJetsWithDRCut(0);
	    unsigned int nGoodMergedJets(evt.goodMergedJets[type].size());
	    for(unsigned int j(0); j<nGoodMergedJets; j++) {
	      const xAOD::Jet *jet(evt.goodMergedJets[type].at(j));
	      
	      double dR_0(DerivationFramework::SkimmingToolHIGG2::getDeltaR(v_tlv.at(i0).Eta(), v_tlv.at(i0).Phi(), jet->eta(), jet->phi()));
	      if(dR_0<m_dRElectronJetCut) continue;
	      
	      double dR_1(DerivationFramework::SkimmingToolHIGG2::getDeltaR(v_tlv.at(i1).Eta(), v_tlv.at(i1).Phi(), jet->eta(), jet->phi()));
	      if(dR_1<m_dRElectronJetCut) continue;
	      
	      nGoodMergedJetsWithDRCut++;
	    }

	    checkGoodJetsWithDRCut = (checkGoodJetsWithDRCut or (nGoodMergedJetsWithDRCut>=m_nMergedJets[type]));
	  }
	}
	if(!checkGoodJetsWithDRCut) return false; 
      }

      // Leading lepton ET/pT cut
      if((v_tlv.at(i0).Pt()>v_tlv.at(i1).Pt() ? v_tlv.at(i0).Pt() : v_tlv.at(i1).Pt()) > (v_isElectron.at(i0) ? m_leadingElectronEtCut : m_leadingMuonPtCut)) return true;
    }
  }

  return false;
}

bool DerivationFramework::SkimmingToolHIGG2::checkJPSI(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  if(!(m_nMuons>0 and evt.goodMuons.size()>=m_nMuons)) return false;
  if(!(evt.goodPhotons.size()>=m_nPhotons)) return false;

  bool isTriggerFired(m_triggerJPSI.empty() or m_skipTriggerRequirement);
  for(unsigned int i(0); i<m_triggerJPSI.size(); i++) {
    if(m_trigDecisionTool->isPassed(m_triggerJPSI.at(i))) {
      isTriggerFired = true;
      break;
    }
  }
  if(!isTriggerFired) return false; 

  std::vector<TLorentzVector> v_tlv(evt.goodMuons.size());
  
  for(unsigned int mu_i(0); mu_i<evt.goodMuons.size(); mu_i++) {
    const xAOD::Muon *mu(evt.goodMuons.at(mu_i));
    TLorentzVector tlv(DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(mu));
    v_tlv.at(mu_i) = tlv;
  }
  
  unsigned int nGoodLeptons(v_tlv.size());
  for(unsigned int i0(0); i0<nGoodLeptons; i0++) {
    for(unsigned int i1(i0+1); i1<nGoodLeptons; i1++) {
      TLorentzVector tlv_2lep(v_tlv.at(i0) + v_tlv.at(i1));

      // Check di-muon mass
      if((tlv_2lep.M()<m_invariantMassJpsiLowCut    or tlv_2lep.M()>m_invariantMassJpsiUpCut) and
	 (tlv_2lep.M()<m_invariantMassUpsilonLowCut or tlv_2lep.M()>m_invariantMassUpsilonUpCut)) continue;

      // Check leading muon pT
      if((v_tlv.at(i0).Pt()>v_tlv.at(i1).Pt() ? v_tlv.at(i0).Pt() : v_tlv.at(i1).Pt())>m_leadingMuonPtCut) return true;
    }
  }

  return false;
}

bool DerivationFramework::SkimmingToolHIGG2::checkPHI(const DerivationFramework::SkimmingToolHIGG2::Candidates& evt) const
{
  // Check if there are candidates
  if(!(m_nTracks>0 and evt.goodTracks.size()>=m_nTracks)) return false;
  if(!(evt.goodPhotons.size()>=m_nPhotons)) return false;

  // Check if triggers are OK
  bool isTriggerFired(m_triggerPHI.empty() or m_skipTriggerRequirement);
  for(unsigned int i(0); i<m_triggerPHI.size(); i++) {
    if(m_trigDecisionTool->isPassed(m_triggerPHI.at(i))) {
      isTriggerFired = true;
      break;
    }
  }
  if(!isTriggerFired) return false; 

  // Get 4-momentum of tracks (=charged kaons)
  std::vector<TLorentzVector> v_tlv[2];
  for(unsigned int trk_i(0); trk_i<evt.goodTracks.size(); trk_i++) {
    const xAOD::TrackParticle *trk(evt.goodTracks.at(trk_i));
    TLorentzVector tlv;
    tlv.SetPtEtaPhiM(trk->pt(), trk->eta(), trk->phi(), s_MKplus); // Kaon is assumed. 
    v_tlv[trk->charge()>0. ? 0 : 1].push_back(tlv); // 0 is positive, 1 is negative
  }
  
  // Check track pair mass
  unsigned int nGoodTracks[2];
  nGoodTracks[0] = v_tlv[0].size();
  nGoodTracks[1] = v_tlv[1].size();
  for(unsigned int i0(0); i0<nGoodTracks[0]; i0++) {
    for(unsigned int i1(0); i1<nGoodTracks[1]; i1++) {
      TLorentzVector tlv_2trk(v_tlv[0].at(i0) + v_tlv[1].at(i1));
      if(tlv_2trk.M()<m_invariantMassPhiLowCut or tlv_2trk.M()>m_invariantMassPhiUpCut) continue;
      return true; // One candidate is found. 
    }
  }

  return false; // No candidate is found. 
}

TLorentzVector DerivationFramework::SkimmingToolHIGG2::electronFourMomentum(const xAOD::Electron *el) const
{
  TLorentzVector tlv;
  tlv.SetPtEtaPhiE(el->pt(), el->eta(), el->phi(), el->e());
  if(m_defaultElectronFourMomentum) return tlv;

  const xAOD::TrackParticle *trackParticle(el->trackParticle());
  if(!trackParticle) {
    ATH_MSG_WARNING("xAOD::TrackParticle pointer is null");
    return tlv;
  }
  const xAOD::CaloCluster *caloCluster(el->caloCluster());
  if(!caloCluster) {
    ATH_MSG_WARNING("xAOD::CaloCluster pointer is null");
    return tlv;
  }
    
  double eta(trackParticle->eta());
  double phi(trackParticle->phi());
  if(fabs(eta)>10.) {
    ATH_MSG_WARNING("fabs(trackParticle->eta()) = " << eta << " > 10.");
    return tlv;
  }
  double e(caloCluster->e());
  double et(e/cosh(eta));
    
  tlv.SetPtEtaPhiE(et, eta, phi, e);
  return tlv;
}

TLorentzVector DerivationFramework::SkimmingToolHIGG2::muonFourMomentum(const xAOD::Muon *mu) 
{
  TLorentzVector tlv;
  tlv.SetPtEtaPhiM(mu->pt(), mu->eta(), mu->phi(), mu->m());
  return tlv;
}

TLorentzVector DerivationFramework::SkimmingToolHIGG2::jetFourMomentum(const xAOD::Jet *jet) const
{
  TLorentzVector tlv;
  if(m_DFCommonJetFourMomentum) {
    static const SG::ConstAccessor<float> DFCommonJets_Calib_ptAcc("DFCommonJets_Calib_pt");
    static const SG::ConstAccessor<float> DFCommonJets_Calib_etaAcc("DFCommonJets_Calib_eta");
    static const SG::ConstAccessor<float> DFCommonJets_Calib_phiAcc("DFCommonJets_Calib_phi");
    static const SG::ConstAccessor<float> DFCommonJets_Calib_mAcc("DFCommonJets_Calib_m");

    const float& pt =DFCommonJets_Calib_ptAcc(*jet);
    const float& eta=DFCommonJets_Calib_etaAcc(*jet);
    const float& phi=DFCommonJets_Calib_phiAcc(*jet);
    const float& m  =DFCommonJets_Calib_mAcc(*jet);
    tlv.SetPtEtaPhiM(pt, eta, phi, m);
  }
  else
    tlv.SetPtEtaPhiM(jet->pt(), jet->eta(), jet->phi(), jet->m());

  return tlv;
}

double DerivationFramework::SkimmingToolHIGG2::getDeltaR(const double eta1, const double phi1, const double eta2, const double phi2) 
{
  double dEta(eta1 - eta2);
  double dPhi(phi1 - phi2);
  if(dPhi>+M_PI) dPhi -= 2.*M_PI;
  if(dPhi<-M_PI) dPhi += 2.*M_PI;
  double dR(sqrt(dEta*dEta + dPhi*dPhi));
  return dR;
}

const double DerivationFramework::SkimmingToolHIGG2::s_MZ(ParticleConstants::ZMassInMeV); 
const double DerivationFramework::SkimmingToolHIGG2::s_MKplus(ParticleConstants::chargedKaonMassInMeV); 
