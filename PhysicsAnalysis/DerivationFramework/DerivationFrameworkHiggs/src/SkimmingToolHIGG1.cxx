/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// SkimmingToolHIGG1.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Based on DerivationFramework::SkimmingToolExample

#include "DerivationFrameworkHiggs/SkimmingToolHIGG1.h"
#include <vector>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODTracking/TrackingPrimitives.h"
#include "PhotonVertexSelection/IPhotonVertexSelectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"
#include "AthContainers/ConstAccessor.h"

// Constructor
DerivationFramework::SkimmingToolHIGG1::SkimmingToolHIGG1(const std::string& t,
							    const std::string& n,
							    const IInterface* p) : 
  base_class(t, n, p),
  m_trigDecisionTool("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_mergedCutTools("")
{


  declareProperty("RequireGRL",            m_reqGRL           = true);
  declareProperty("ReqireLArError",        m_reqLArError      = true);
  declareProperty("RequireTrigger",        m_reqTrigger       = true);
  declareProperty("RequirePreselection",   m_reqPreselection  = true);
  declareProperty("IncludeSingleMergedElectronPreselection", m_incMergedElectron = false);
  declareProperty("IncludeSingleElectronPreselection",   m_incSingleElectron  = true);
  declareProperty("IncludeDoubleElectronPreselection",   m_incDoubleElectron  = false);
  declareProperty("IncludeSingleMuonPreselection",       m_incSingleMuon  = true);
  declareProperty("IncludeDoubleMuonPreselection",       m_incDoubleMuon  = false);
  declareProperty("IncludePhotonDoubleElectronPreselection", m_incDoubleElectronPhoton = false);
  declareProperty("IncludePhotonMergedElectronPreselection", m_incMergedElectronPhoton = false);
  declareProperty("IncludeHighPtPhotonElectronPreselection", m_incHighPtElectronPhoton = false);
  declareProperty("IncludeDoublePhotonPreselection",     m_incTwoPhotons  = true);

  declareProperty("RequireKinematic",      m_reqKinematic     = true);
  declareProperty("RequireQuality",        m_reqQuality       = true);
  declareProperty("RequireIsolation",      m_reqIsolation     = true);
  declareProperty("RequireInvariantMass",  m_reqInvariantMass = true);

  declareProperty("GoodRunList",           m_goodRunList = "");

  declareProperty("DefaultTrigger",        m_defaultTrigger = "EF_g35_loose_g25_loose");
  declareProperty("Triggers",              m_triggers = std::vector<std::string>()); 
  declareProperty("MergedElectronTriggers",m_mergedtriggers = std::vector<std::string>() );

  declareProperty("MinimumPhotonPt",       m_minPhotonPt = 20*CLHEP::GeV);
  declareProperty("MinimumElectronPt",     m_minElectronPt = 20*CLHEP::GeV);
  declareProperty("MinimumMergedElectronPt",  m_minMergedElectronPt = 18*CLHEP::GeV);

  declareProperty("MinimumMuonPt",         m_minMuonPt = 20*CLHEP::GeV);
  declareProperty("MaxMuonEta",            m_maxMuonEta = 2.7);
  declareProperty("RemoveCrack",           m_removeCrack = true);
  declareProperty("MaxEta",                m_maxEta = 2.47);

  declareProperty("RelativePtCuts",        m_relativePtCuts     = true);
  declareProperty("LeadingPhotonPtCut",    m_leadingPhotonPt    = 0.35);
  declareProperty("SubleadingPhotonPtCut", m_subleadingPhotonPt = 0.25);

  declareProperty("MinInvariantMass",      m_minInvariantMass = 105*CLHEP::GeV);
  declareProperty("MaxInvariantMass",      m_maxInvariantMass = 160*CLHEP::GeV);

  declareProperty("MergedElectronCutTool",  m_mergedCutTools);

}
  
// Destructor
DerivationFramework::SkimmingToolHIGG1::~SkimmingToolHIGG1() {
}  

// Athena initialize and finalize
StatusCode DerivationFramework::SkimmingToolHIGG1::initialize()
{
  ATH_MSG_VERBOSE("INITIALIZING HSG1 SELECTOR TOOL");

  ////////////////////////////
  // trigger decision tool
  if(m_trigDecisionTool.retrieve(DisableTool{!m_reqTrigger}).isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool: " << m_trigDecisionTool);
    return StatusCode::FAILURE;
  }
  if (m_triggers.empty()) m_triggers.push_back(m_defaultTrigger);
  ATH_MSG_INFO("Retrieved tool: " << m_trigDecisionTool);
  ////////////////////////////
  //
  if(m_incMergedElectronPhoton){
    if( m_mergedCutTools.retrieve().isFailure() )
    {
      ATH_MSG_FATAL("Failed to retrieve tool: ElectronPhotonSelectorTools");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK( m_eventInfoKey.initialize() );

  ATH_CHECK( m_photonKey.initialize() );

  ATH_CHECK( m_electronKey.initialize() );

  ATH_CHECK( m_muonKey.initialize() );
  
  ////////////////////////////
  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::SkimmingToolHIGG1::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed " << m_n_tot << " events, " << m_n_pass << " events passed filter ");

  
  ATH_MSG_INFO("GRL       :: " << m_n_passGRL);
  ATH_MSG_INFO("lar       :: " << m_n_passLArError);
  ATH_MSG_INFO("trig      :: " << m_n_passTrigger);
  ATH_MSG_INFO("----------------------------");
  if(m_incDoubleElectron)
    ATH_MSG_INFO("2e        :: " << m_n_passDoubleElectronPreselect);
  if(m_incSingleElectron)
    ATH_MSG_INFO("1y1e      :: " << m_n_passSingleElectronPreselect);
  if(m_incSingleMuon)
    ATH_MSG_INFO("1y1mu     :: " << m_n_passSingleMuonPreselect);
  if(m_incDoubleMuon)
    ATH_MSG_INFO("1y2mu     :: " << m_n_passSinglePhotonDoubleMuonPreselect);
  if(m_incDoubleElectronPhoton)
    ATH_MSG_INFO("1y2e      :: " << m_n_passSinglePhotonDoubleElectronPreselect);
  if(m_incMergedElectronPhoton)
    ATH_MSG_INFO("1y1eMerge :: " << m_n_passSinglePhotonMergedElectronPreselect);
  if(m_incHighPtElectronPhoton)
    ATH_MSG_INFO("1y1e HiPt :: " << m_n_passHighPtPhotonMergedElectronPreselect);
  if(m_incMergedElectron)
    ATH_MSG_INFO("1eMerge   :: " << m_n_passSingleMergedElectronPreselect);

  if(m_incTwoPhotons){
    ATH_MSG_INFO("2y        :: " << m_n_passPreselect);
    ATH_MSG_INFO("----------------------------");
    ATH_MSG_INFO("2y - kin       :: " << m_n_passKinematic);
    ATH_MSG_INFO("2y - qual      :: " << m_n_passQuality);
    ATH_MSG_INFO("2y - iso       :: " << m_n_passIsolation);
    ATH_MSG_INFO("2y - inv       :: " << m_n_passInvariantMass);
  }
  ATH_MSG_INFO("----------------------------");
  ATH_MSG_INFO("passed     :: " << m_n_pass);
  
  return StatusCode::SUCCESS;
}

// The filter itself
bool DerivationFramework::SkimmingToolHIGG1::eventPassesFilter() const
{

  m_n_tot++;

  bool writeEvent(false);
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);

  if (m_reqGRL      && !SubcutGoodRunList() ) return false;
  if (m_reqLArError && !SubcutLArError(*eventInfo)    ) return false;
  if (m_reqTrigger  && !SubcutTrigger()     ) return false;

  const auto leadingPhotons = SubcutPreselect();
  if (m_incTwoPhotons && !m_reqPreselection) writeEvent = true;	    

  // ey, ee, muy events
  if (m_incSingleElectron && SubcutOnePhotonOneElectron() ) writeEvent = true;
  if (m_incDoubleElectron && SubcutTwoElectrons()         ) writeEvent = true;
  if (m_incSingleMuon     && SubcutOnePhotonOneMuon()     ) writeEvent = true;
  
  // eey, mumuy events
  if (m_incMergedElectronPhoton && SubcutOnePhotonMergedElectrons(*eventInfo)) writeEvent = true;
  if (m_incDoubleMuon           && SubcutOnePhotonTwoMuons()       ) writeEvent = true;
  if (m_incDoubleElectronPhoton && SubcutOnePhotonTwoElectrons()   ) writeEvent = true;
  if (m_incHighPtElectronPhoton && SubcutHighPtOnePhotonOneElectron() ) writeEvent = true;

  if (m_incMergedElectron && SubcutOneMergedElectron() ) writeEvent = true;
  // There *must* be two photons for the remaining 
  // pieces, but you can still save the event...
  if (m_incTwoPhotons && leadingPhotons) {
    GetDiphotonVertex(); 
    const double mass = CalculateInvariantMass(leadingPhotons.value());

    bool passTwoPhotonCuts(true);     
    if (m_reqQuality         && !SubcutQuality(leadingPhotons.value())) passTwoPhotonCuts = false;
    if (m_reqKinematic       && !SubcutKinematic(leadingPhotons.value(), mass)) passTwoPhotonCuts = false;
    if (m_reqIsolation       && !SubcutIsolation()) passTwoPhotonCuts = false;
    if (m_reqInvariantMass   && !SubcutInvariantMass(mass)) passTwoPhotonCuts = false;
    // yy events
    if (passTwoPhotonCuts) writeEvent = true; 
    
  }
   

  if (!writeEvent) return false;
  
  m_n_pass++;
  return true;
}

bool DerivationFramework::SkimmingToolHIGG1::SubcutGoodRunList() const {

  // Placeholder
  m_n_passGRL++;
  return true;
}
  
  
bool DerivationFramework::SkimmingToolHIGG1::SubcutLArError(const xAOD::EventInfo& eventInfo) const {

  if (eventInfo.errorState(xAOD::EventInfo::LAr) != xAOD::EventInfo::Error) {
    m_n_passLArError++;
    return true;
  }
  else return false;
}


bool DerivationFramework::SkimmingToolHIGG1::SubcutTrigger() const {

  //just for counting purposes
  bool passTrigger = !m_reqTrigger;
  
  if(m_triggers.empty()) passTrigger = true;

  for (unsigned int i = 0; i < m_triggers.size(); i++) {
    ATH_MSG_DEBUG("TRIGGER = " << m_triggers.at(i));
    if(m_trigDecisionTool->isPassed(m_triggers.at(i)))
      passTrigger = true;
  }
  
  if (passTrigger) m_n_passTrigger++;
  return passTrigger;

}


std::optional<DerivationFramework::SkimmingToolHIGG1::LeadingPhotons_t>
DerivationFramework::SkimmingToolHIGG1::SubcutPreselect() const {

  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  int ph_pos_lead = -1;
  int ph_pos_subl = -1;
  int ph_pt_lead = 0;
  int ph_pt_subl = 0;

  for(int i = 0; ph_itr != ph_end; ++ph_itr, ++i) {

    if (PhotonPreselect(*ph_itr)) {

      if ((*ph_itr)->pt() > ph_pt_lead) {

        ph_pos_subl = ph_pos_lead; ph_pos_lead = i;
        ph_pt_subl = ph_pt_lead;
        ph_pt_lead = (*ph_itr)->pt();

      } else if ((*ph_itr)->pt() > ph_pt_subl) {
        ph_pos_subl = i;
        ph_pt_subl = (*ph_itr)->pt();
      }
    }
  }

  // save this for the derivation.
  //std::vector<int> *leadingV = new std::vector<int>();
  //leadingV->push_back(m_ph_pos_lead);
  //leadingV->push_back(m_ph_pos_subl);
  //if (!evtStore()->contains<std::vector<int> >("leadingV")) CHECK(evtStore()->record(leadingV, "leadingV"));

  // save this for this code.
  if (ph_pos_subl != -1) {
    const xAOD::Photon* ph_lead = *(photons->begin() + ph_pos_lead);
    const xAOD::Photon* ph_subl = *(photons->begin() + ph_pos_subl);
    m_n_passPreselect++;
    
    return LeadingPhotons_t{ph_lead,ph_subl};
  }

  return {};
}


bool DerivationFramework::SkimmingToolHIGG1::PhotonPreselect(const xAOD::Photon *ph) const {

  if (!ph) return false;

  if (!ph->isGoodOQ(34214)) return false;

  bool val(false);
  bool defined(false);

  static const SG::ConstAccessor<char> DFCommonPhotonsIsEMLooseAcc("DFCommonPhotonsIsEMLoose");
  if(DFCommonPhotonsIsEMLooseAcc.isAvailable(*ph)){
    defined = true;
    val = static_cast<bool>(DFCommonPhotonsIsEMLooseAcc(*ph));
  }
  else{
    defined = ph->passSelection(val, "Loose");
  }
  
  if(!defined || !val) return false;
  

  // veto topo-seeded clusters 
  // uint16_t author = 0;
  // author = ph->author();  
  // if (author & xAOD::EgammaParameters::AuthorCaloTopo35) return false;

  // Check which variable versions are best...
  const xAOD::CaloCluster *caloCluster(ph->caloCluster());
  double eta = std::abs(caloCluster->etaBE(2));

  if (eta > m_maxEta)             return false;
  if (m_removeCrack && 
      1.37 <= eta && eta <= 1.52) return false;
  if (caloCluster->e()/cosh(eta) < m_minPhotonPt) return false;

  return true;

}

bool DerivationFramework::SkimmingToolHIGG1::SubcutKinematic(const LeadingPhotons_t& leadingPhotons, double invariantMass) const {

  bool passKinematic;
  if (m_relativePtCuts) {
    passKinematic =  (leadingPhotons[0]->pt() > invariantMass * m_leadingPhotonPt);
    passKinematic &= (leadingPhotons[1]->pt() > invariantMass * m_subleadingPhotonPt);
  } else {
    passKinematic =  (leadingPhotons[0]->pt() > m_leadingPhotonPt);
    passKinematic &= (leadingPhotons[1]->pt() > m_subleadingPhotonPt);
  }

  if (passKinematic) m_n_passKinematic++;
  return passKinematic;

}

bool DerivationFramework::SkimmingToolHIGG1::SubcutQuality(const LeadingPhotons_t& leadingPhotons) const {

  bool val(0);
  bool passQuality = false;
  leadingPhotons[0]->passSelection(val, "Tight");
  const int ph_tight_lead = val;

  leadingPhotons[1]->passSelection(val, "Tight");
  const int ph_tight_subl = val;

  passQuality = (ph_tight_lead && ph_tight_subl);

  if (passQuality) m_n_passQuality++;
  return passQuality;

}

bool DerivationFramework::SkimmingToolHIGG1::SubcutIsolation() const {

  // PLACEHOLDER!!!
  m_n_passIsolation++;
  return true;
}


bool DerivationFramework::SkimmingToolHIGG1::SubcutInvariantMass(double invariantMass) const {

  bool passInvariantMass =  (!m_minInvariantMass ||
                             m_minInvariantMass < invariantMass);

  passInvariantMass &= (!m_maxInvariantMass ||
                        invariantMass < m_maxInvariantMass);

  if (passInvariantMass) m_n_passInvariantMass++;
  return passInvariantMass;

}

double DerivationFramework::SkimmingToolHIGG1::CalculateInvariantMass(const LeadingPhotons_t& leadingPhotons) const {

  /// CAUTION - PLACEHOLDERS
  const double ph_e_lead   = CorrectedEnergy(leadingPhotons[0]);
  const double ph_e_subl   = CorrectedEnergy(leadingPhotons[1]);

  /// CAUTION - CONSTANTS SHOULD BE UPDATED.
  const double ph_eta_lead = CorrectedEta(leadingPhotons[0]);
  const double ph_eta_subl = CorrectedEta(leadingPhotons[1]);

  const double ph_phi_lead = leadingPhotons[0]->phi();
  const double ph_phi_subl = leadingPhotons[1]->phi();

  const double ph_pt_lead  = ph_e_lead / cosh(ph_eta_lead);
  const double ph_pt_subl  = ph_e_subl / cosh(ph_eta_subl);

  TLorentzVector leadPhotonLV;
  TLorentzVector sublPhotonLV;
  leadPhotonLV.SetPtEtaPhiM(ph_pt_lead, ph_eta_lead, ph_phi_lead, 0.);
  sublPhotonLV.SetPtEtaPhiM(ph_pt_subl, ph_eta_subl, ph_phi_subl, 0.);

  return (leadPhotonLV + sublPhotonLV).M();

}



double DerivationFramework::SkimmingToolHIGG1::GetDiphotonVertex() const {

  return 0;

}

//// THIS IS A PLACEHOLDER!!
double DerivationFramework::SkimmingToolHIGG1::CorrectedEnergy(const xAOD::Photon *ph) {

  return ph->e();

}


//////////  THE FOLLOWING TWO FUNCTIONS ARE ADAPTED FROM 
//////////  RUN I HSG1 CUT FLOWS: USE WITH CARE AND CHECK!!!
double DerivationFramework::SkimmingToolHIGG1::CorrectedEta(const xAOD::Photon *ph) const {

  double eta1 = ph->caloCluster()->etaBE(1); 

  double R_photom_n_front, Z_photom_n_front;
  if (std::abs(eta1) < 1.5) { // barrel
    R_photom_n_front = ReturnRZ_1stSampling_cscopt2(eta1);
    Z_photom_n_front = R_photom_n_front*sinh(eta1);
  } else { // endcap
    Z_photom_n_front = ReturnRZ_1stSampling_cscopt2(eta1);
    R_photom_n_front = Z_photom_n_front/sinh(eta1);
  }

  return asinh((Z_photom_n_front - GetDiphotonVertex())/R_photom_n_front);

}


double DerivationFramework::SkimmingToolHIGG1::ReturnRZ_1stSampling_cscopt2(double eta1) {

  float abs_eta1 = std::abs(eta1);

  double radius = -99999;
  if (abs_eta1 < 0.8) {
    radius = 1558.859292 - 4.990838  * abs_eta1 - 21.144279 * abs_eta1 * abs_eta1;
  } else if (abs_eta1 < 1.5) {
    radius = 1522.775373 + 27.970192 * abs_eta1 - 21.104108 * abs_eta1 * abs_eta1;
  } else { //endcap
    radius = 3790.671754;
    if (eta1 < 0.) radius = -radius;
  }

  return radius;

}

bool DerivationFramework::SkimmingToolHIGG1::SubcutOnePhotonOneElectron() const {

  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  xAOD::ElectronContainer::const_iterator el_itr(electrons->begin());
  xAOD::ElectronContainer::const_iterator el_end(electrons->end());

  bool passSingleElectronPreselect = false;

  for( ; ph_itr != ph_end; ++ph_itr){
    if(PhotonPreselect(*ph_itr)){
      for( ; el_itr != el_end; ++el_itr){
        if(ElectronPreselect(*el_itr)){
          passSingleElectronPreselect = true;
        }
      }
    }
  }


  if(passSingleElectronPreselect) m_n_passSingleElectronPreselect++;
  return passSingleElectronPreselect;
}


bool DerivationFramework::SkimmingToolHIGG1::SubcutOneMergedElectron() const {
  
  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  int nEle(0);
  for(const auto *const el: *electrons){
    if( el->pt() < m_minElectronPt)
      continue;
    //Count the number of Si tracks matching the electron
    int nSiTrack(0);
    int z0_1 = 1;
    for( unsigned int trk_i(0); trk_i < el->nTrackParticles(); ++trk_i){
      const auto *ele_tp =  el->trackParticle(trk_i);
      if(!ele_tp){
        continue;
      }
      uint8_t nPixHits(0), nPixDead(0), nSCTHits(0), nSCTDead(0);
      bool allFound = true;
      allFound = allFound && ele_tp->summaryValue(nPixHits, xAOD::numberOfPixelHits);
      allFound = allFound && ele_tp->summaryValue(nPixDead, xAOD::numberOfPixelDeadSensors);
      allFound = allFound && ele_tp->summaryValue(nSCTHits, xAOD::numberOfSCTHits);
      allFound = allFound && ele_tp->summaryValue(nSCTDead, xAOD::numberOfSCTDeadSensors);

      // Require that the track be a reasonble silicon track
      int nSiHitsPlusDeadSensors = nPixHits + nPixDead + nSCTHits + nSCTDead;
      if(nSiHitsPlusDeadSensors >= 7)
      {
        //Ensure that the tracks come from roughly the same region of the detector
        if(nSiTrack == 0)
          z0_1 = ele_tp->z0();
        else if( std::abs(z0_1 - ele_tp->z0()) > 10 )
          continue;
        ++nSiTrack;
      }
    }
    //If 2 or more the electron is selected
    if(nSiTrack>1)
      ++nEle;
  }
  if(nEle>0){
    ++m_n_passSingleMergedElectronPreselect;
    return true;
  }
  return false;
}

bool DerivationFramework::SkimmingToolHIGG1::SubcutTwoElectrons() const {

  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  xAOD::ElectronContainer::const_iterator el_itr(electrons->begin());
  xAOD::ElectronContainer::const_iterator el_end(electrons->end());

  int nEle(0);
  bool passDoubleElectronPreselect = false;
  
  for( ; el_itr != el_end; ++el_itr){
    if(ElectronPreselect(*el_itr))
      nEle++;
  }
  
  if(nEle >=2) passDoubleElectronPreselect = true;
  
  if(passDoubleElectronPreselect) m_n_passDoubleElectronPreselect++;
  return passDoubleElectronPreselect;
}


bool DerivationFramework::SkimmingToolHIGG1::SubcutOnePhotonOneMuon() const {

  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  SG::ReadHandle<xAOD::MuonContainer> muons (m_muonKey);

  xAOD::MuonContainer::const_iterator mu_itr(muons->begin());
  xAOD::MuonContainer::const_iterator mu_end(muons->end());

  bool passSingleMuonPreselect = false;

  for( ; ph_itr != ph_end; ++ph_itr){
    if(PhotonPreselect(*ph_itr)){
      for( ; mu_itr != mu_end; ++mu_itr){
        if(MuonPreselect(*mu_itr)){
          passSingleMuonPreselect = true;
        }
      }
    }
  }


  if(passSingleMuonPreselect) m_n_passSingleMuonPreselect++;
  return passSingleMuonPreselect;
}

bool DerivationFramework::SkimmingToolHIGG1::SubcutOnePhotonTwoMuons() const
{
  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  SG::ReadHandle<xAOD::MuonContainer> muons (m_muonKey);

  xAOD::MuonContainer::const_iterator mu_itr(muons->begin());
  xAOD::MuonContainer::const_iterator mu_end(muons->end());

  int nPhoton = 0;
  int nMuon   = 0;

  for( ; ph_itr != ph_end; ++ph_itr){
    if(PhotonPreselect(*ph_itr)){
      ++nPhoton;
    }
  }

  for( ; mu_itr != mu_end; ++mu_itr){
    if(MuonPreselect(*mu_itr)){
      ++nMuon;
    }
  }


  if(nPhoton >= 1 &&  nMuon >= 2){
    ATH_MSG_DEBUG("Event selected with " << nPhoton << " photons and " << nMuon << " muons");
    m_n_passSinglePhotonDoubleMuonPreselect++;
    return true;
  } else {
    return false;
  }
}


bool DerivationFramework::SkimmingToolHIGG1::SubcutOnePhotonTwoElectrons() const
{
  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  xAOD::ElectronContainer::const_iterator el_itr(electrons->begin());
  xAOD::ElectronContainer::const_iterator el_end(electrons->end());

  int nPhoton    = 0;
  int nElectron  = 0;

  for( ; ph_itr != ph_end; ++ph_itr){
    if(PhotonPreselect(*ph_itr)){
      ++nPhoton;
    }
  }

  for( ; el_itr != el_end; ++el_itr){
    if(ElectronPreselect(*el_itr)){
      ++nElectron; 
    }
  }

  if(nPhoton >= 1 &&  nElectron >= 2){
    ATH_MSG_DEBUG("Event selected with " << nPhoton << " photons and " << nElectron << " electrons");
    ++m_n_passSinglePhotonDoubleElectronPreselect;
    return true;
  } else {
    return false;
  }

}

bool DerivationFramework::SkimmingToolHIGG1::SubcutOnePhotonMergedElectrons(const xAOD::EventInfo& eventInfo) const
{


  bool passTrigger=false;
  if(!m_mergedtriggers.empty()) {
    for (unsigned int i = 0; i < m_mergedtriggers.size(); i++) {
      ATH_MSG_DEBUG("TRIGGER = " << m_mergedtriggers.at(i));
      if(m_trigDecisionTool->isPassed(m_mergedtriggers.at(i)))
        passTrigger = true;
    }
  } else {
    if(!eventInfo.eventType(xAOD::EventInfo::IS_SIMULATION))
      ATH_MSG_WARNING("Selecting Merged electrons but no Merged Triggers Selected ! -- was that intentional?");
    passTrigger =  true;
  }
  if(!passTrigger)
    return false;
   

  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  bool passSelection = false;

  for(const auto *el : *electrons){
    if(MergedElectronPreselect(el)){
      for(const auto *ph: *photons){
        if(PhotonPreselect(ph)){
          passSelection = true;
          auto eph = ph->p4() + el->p4();
          if(eph.M() >  90 * CLHEP::GeV)  
          {  
            break;
          }
        }
      }
    }
    if(passSelection)
    { 
      break;
    }
  }

  if(passSelection){
    ATH_MSG_DEBUG("Event selected with a photons and a merged electron");
    ++m_n_passSinglePhotonMergedElectronPreselect;
    return true;
  } else {
    return false;
  }

}


bool DerivationFramework::SkimmingToolHIGG1::SubcutHighPtOnePhotonOneElectron() const
{


  SG::ReadHandle<xAOD::PhotonContainer> photons (m_photonKey);

  xAOD::PhotonContainer::const_iterator ph_itr(photons->begin());
  xAOD::PhotonContainer::const_iterator ph_end(photons->end());

  SG::ReadHandle<xAOD::ElectronContainer> electrons (m_electronKey);

  xAOD::ElectronContainer::const_iterator el_itr(electrons->begin());
  xAOD::ElectronContainer::const_iterator el_end(electrons->end());

  int nPhoton    = 0;
  int nElectron  = 0;

  for( ; ph_itr != ph_end; ++ph_itr){
    if(PhotonPreselect(*ph_itr) && (*ph_itr)->pt() > 500*CLHEP::GeV){
      ++nPhoton;
    }
  }

  for( ; el_itr != el_end; ++el_itr){
    if( std::abs((*el_itr)->eta()) <= m_maxEta  && (*el_itr)->pt() > m_minElectronPt){
      ++nElectron; 
    }
  }

  if(nPhoton >= 1 &&  nElectron >= 1 ){
    ATH_MSG_DEBUG("Event selected with " << nPhoton << " high pt photons and " << nElectron << " merged electron");
    ++m_n_passHighPtPhotonMergedElectronPreselect;
    return true;
  } else {
    return false;
  }

}



bool DerivationFramework::SkimmingToolHIGG1::ElectronPreselect(const xAOD::Electron *el) const {

  if (!el) return false;

  bool val(false);
  bool defined(false);

  static const SG::ConstAccessor<char> DFCommonElectronsLooseAcc("DFCommonElectronsLoose");
  if(DFCommonElectronsLooseAcc.isAvailable(*el)){
    defined = true;
    val = val || static_cast<bool>(DFCommonElectronsLooseAcc(*el));
  }else{
    defined = el->passSelection(val, "Loose");
  } 

  static const SG::ConstAccessor<char> DFCommonElectronsLHLooseAcc("DFCommonElectronsLHLoose");
  if(DFCommonElectronsLHLooseAcc.isAvailable(*el)){
    defined = true;
    val = val || static_cast<bool>(DFCommonElectronsLHLooseAcc(*el));
  }

  if(!defined || !val) return false;

  double eta = std::abs(el->eta());
  double pt = el->pt();

  if (eta > m_maxEta) return false;
  if (m_removeCrack && 1.37 <= eta && eta <= 1.52) return false;
  if (pt <= m_minElectronPt) return false;

  return true;

}

bool DerivationFramework::SkimmingToolHIGG1::MergedElectronPreselect(const xAOD::Electron *el) const {

  if (!el) return false;

  double eta = std::abs(el->eta());
  double pt = el->pt();

  if (eta > m_maxEta) return false;
  if (m_removeCrack && 1.37 <= eta && eta <= 1.52) return false;
  if (pt <= m_minMergedElectronPt) return false;

  return m_mergedCutTools->accept(el) || ElectronPreselect(el);

}

bool DerivationFramework::SkimmingToolHIGG1::MuonPreselect(const xAOD::Muon *mu) const {

  if (!mu) return false;

  static const SG::ConstAccessor<char> DFCommonGoodMuonAcc("DFCommonGoodMuon");
  if(DFCommonGoodMuonAcc.isAvailable(*mu))
    if( !static_cast<bool>(DFCommonGoodMuonAcc(*mu)) )
      return false;
  
  static const SG::ConstAccessor<char> DFCommonMuonsPreselectionAcc("DFCommonMuonsPreselection");
  if(DFCommonMuonsPreselectionAcc.isAvailable(*mu))
    if( !static_cast<bool>(DFCommonMuonsPreselectionAcc(*mu)) )
      return false;

  double eta = std::abs(mu->eta());
  double pt = mu->pt();

  if (eta > m_maxMuonEta) return false;
  if (pt <= m_minMuonPt) return false;

  return true;

}

const double DerivationFramework::SkimmingToolHIGG1::s_MZ = 91187.6*CLHEP::MeV; 



