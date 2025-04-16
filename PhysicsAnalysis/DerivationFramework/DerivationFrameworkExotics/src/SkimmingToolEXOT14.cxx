/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Based on DerivationFramework::SkimmingToolExample

#include "DerivationFrameworkExotics/SkimmingToolEXOT14.h"
#include <vector>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetAuxContainer.h"
#include "AthContainers/Decorator.h"
// #include "xAODTracking/TrackingPrimitives.h"

// #include "JetCalibTools/JetCalibrationTool.h"


// Constructor
DerivationFramework::SkimmingToolEXOT14::SkimmingToolEXOT14(const std::string& t,
							    const std::string& n,
							    const IInterface* p) : 
  base_class(t, n, p),
  m_trigDecisionTool("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_n_tot(0),
  m_n_passGRL(0),
  m_n_passLArError(0),
  m_n_passTrigger(0),
  m_n_passPreselect(0),
  m_n_passJetPts(0),
  m_n_passJetsDEta(0),
  m_n_passDiJetMass(0),
  m_n_passJetsDPhi(0),
  m_n_pass(0)
{


  declareProperty("JetContainer",          m_jetSGKey = "AntiKt4LCTopoJets");

  declareProperty("RequireGRL",            m_reqGRL          = true);
  declareProperty("ReqireLArError",        m_reqLArError     = true);
  declareProperty("RequireTrigger",        m_reqTrigger      = true);
  declareProperty("RequirePreselection",   m_reqPreselection = true);
  declareProperty("RequireJetPts",         m_reqJetPts       = true);
  declareProperty("RequireJetsDEta",       m_reqJetsDEta     = true);
  declareProperty("RequireDiJetMass",      m_reqDiJetMass    = true);
  declareProperty("RequireJetsDPhi",       m_reqJetsDPhi     = true);

  declareProperty("GoodRunList",           m_goodRunList = "");

  declareProperty("DefaultTrigger",        m_defaultTrigger = "HLT_xe100");
  declareProperty("Triggers",              m_triggers = std::vector<std::string>()); 

  declareProperty("MinimumJetPt",          m_minJetPt = 25*CLHEP::GeV);
  declareProperty("MaxEta",                m_maxEta = 4.8);

  declareProperty("LeadingJetPtCut",       m_leadingJetPt    = 75*CLHEP::GeV);
  declareProperty("SubleadingJetPtCut",    m_subleadingJetPt = 50*CLHEP::GeV);

  declareProperty("EtaSeparation",         m_etaSeparation = 4.8);

  declareProperty("DiJetsMass",            m_dijetMass = 1000*CLHEP::GeV);

  declareProperty("DiJetDPhi",             m_jetDPhi = 2.5);

}
  
// Destructor
DerivationFramework::SkimmingToolEXOT14::~SkimmingToolEXOT14() {
}  

// Athena initialize and finalize
StatusCode DerivationFramework::SkimmingToolEXOT14::initialize()
{
  ATH_MSG_VERBOSE("INITIALIZING VBFINV SELECTOR TOOL");

  ////////////////////////////
  // trigger decision tool
  if(m_trigDecisionTool.retrieve(DisableTool{!m_reqTrigger}).isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool: " << m_trigDecisionTool);
    return StatusCode::FAILURE;
  }
  if (!m_triggers.size()) m_triggers.push_back(m_defaultTrigger);
  ATH_MSG_INFO("Retrieved tool: " << m_trigDecisionTool);
  ////////////////////////////
  

  ////////////////////////////
  // jet energy calibration
  m_JESTool.setTypeAndName("JetCalibrationTool/m_JESTool");
  CHECK( m_JESTool.retrieve() ); //optional, just forces initializing the tool here instead of at first use
  ATH_MSG_INFO("Retrieved tool: " << m_JESTool);
  ////////////////////////////


  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::SkimmingToolEXOT14::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed " << m_n_tot << " events, " << m_n_pass << " events passed filter ");

  ATH_MSG_INFO("GRL       :: " << m_n_passGRL);
  ATH_MSG_INFO("LAr Error :: " << m_n_passLArError);
  ATH_MSG_INFO("Trigger   :: " << m_n_passTrigger);
  ATH_MSG_INFO("Preselect :: " << m_n_passPreselect);
  ATH_MSG_INFO("JetPts    :: " << m_n_passJetPts);
  ATH_MSG_INFO("JetsDEta  :: " << m_n_passJetsDEta);
  ATH_MSG_INFO("DijetMass :: " << m_n_passDiJetMass);
  ATH_MSG_INFO("JetsDPhi  :: " << m_n_passJetsDPhi);

  return StatusCode::SUCCESS;

}

// The filter itself
bool DerivationFramework::SkimmingToolEXOT14::eventPassesFilter() const
{

  m_n_tot++;

  bool writeEvent(false);

  // int *leading    = new int(0);
  // if (!evtStore()->contains<int>("leading"))    CHECK(evtStore()->record(leading,    "leading"));

  if (!SubcutGoodRunList() && m_reqGRL      ) return false;
  if (!SubcutLArError()    && m_reqLArError ) return false;
  if (!SubcutTrigger()     && m_reqTrigger  ) return false;

  const auto jets = SubcutPreselect();
  if (!m_reqPreselection) writeEvent = true;	    

  // There *must* be two jets for the remaining 
  // pieces, but you can still save the event...
  if (jets) {

    bool passDiJets(true);     
    if (!SubcutJetPts(jets.value())        && m_reqJetPts   ) passDiJets = false;
    if (!SubcutJetDEta(jets.value())       && m_reqJetsDEta ) passDiJets = false;
    if (!SubcutDijetMass(jets.value())     && m_reqDiJetMass) passDiJets = false;
    if (!SubcutJetDPhi(jets.value())       && m_reqJetsDPhi ) passDiJets = false;
    if (passDiJets) writeEvent = true; 
  }

  if (!writeEvent) return false;
  
  m_n_pass++;
  return true;

}

bool DerivationFramework::SkimmingToolEXOT14::SubcutGoodRunList() const {

  // Placeholder
  m_n_passGRL++;
  return true;

}
  
  
bool DerivationFramework::SkimmingToolEXOT14::SubcutLArError() const {

  // Retrieve EventInfo
  const xAOD::EventInfo *eventInfo(0);
  ATH_CHECK(evtStore()->retrieve(eventInfo), false);

  const bool passLArError = !(eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error);
  
  if (passLArError) m_n_passLArError++;
  return passLArError;

}


bool DerivationFramework::SkimmingToolEXOT14::SubcutTrigger() const {

  const xAOD::EventInfo *eventInfo(0);
  ATH_CHECK(evtStore()->retrieve(eventInfo), false);

  bool passTrigger = false;

  for (unsigned int i = 0; i < m_triggers.size(); i++) {
    bool thisTrig = m_trigDecisionTool->isPassed(m_triggers.at(i));
    SG::Decorator<bool> acc(TriggerVarName(m_triggers.at(i)));
    acc(*eventInfo) = thisTrig;
    // ATH_MSG_INFO("TRIGGER = " << m_triggers.at(i) <<  " -->> " << thisTrig);
    passTrigger |= thisTrig;
  }
  
  //  temporary pass-through of trigger cut for MC
  if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) passTrigger = true;

  if (passTrigger) m_n_passTrigger++;
  return passTrigger;

}

std::optional<DerivationFramework::SkimmingToolEXOT14::LeadingJets_t>
DerivationFramework::SkimmingToolEXOT14::SubcutPreselect() const {

  // xAOD::TStore store;
  const xAOD::JetContainer *jets(0); 
  ATH_CHECK(evtStore()->retrieve(jets, m_jetSGKey), {});
  xAOD::JetContainer::const_iterator jet_itr(jets->begin());
  xAOD::JetContainer::const_iterator jet_end(jets->end());

  xAOD::JetContainer calibJets;
  calibJets.setStore(new xAOD::JetAuxContainer());

  TLorentzVector j1TLV, j2TLV;

  // Copy jets into the container to be calibrated
  while(jet_itr != jet_end) {
    xAOD::Jet* jetC = new xAOD::Jet();
    jetC->setJetP4(xAOD::JetFourMom_t((*jet_itr)->pt(), (*jet_itr)->eta(), (*jet_itr)->phi(), (*jet_itr)->m()));
    calibJets.push_back(jetC);
    ++jet_itr;
  }

  // Calibrate the jets
  if(m_JESTool->applyCalibration(calibJets).isFailure())
    ATH_MSG_WARNING("Jet calibration returned FAILURE!");

  jet_itr = calibJets.begin();
  jet_end = calibJets.end();
  while(jet_itr != jet_end) {

    if (abs((*jet_itr)->eta()) > m_maxEta) continue;

    if ((*jet_itr)->pt() > j1TLV.Pt()) {

      j2TLV = j1TLV;
      j1TLV.SetPtEtaPhiE((*jet_itr)->pt(), (*jet_itr)->eta(), (*jet_itr)->phi(), (*jet_itr)->e());

    } else if ((*jet_itr)->pt() > j2TLV.Pt()) {

      j2TLV.SetPtEtaPhiE((*jet_itr)->pt(), (*jet_itr)->eta(), (*jet_itr)->phi(), (*jet_itr)->e());
    }
    
    ++jet_itr;
  }

  // save this for this code.
  if (j2TLV.Pt() > m_minJetPt) {
    m_n_passPreselect++;
    return LeadingJets_t{j1TLV, j2TLV};
  }

  return {};

}


bool DerivationFramework::SkimmingToolEXOT14::SubcutJetPts(const LeadingJets_t& jets) const {

  bool passJetPts =  (!m_leadingJetPt    || jets[0].Pt() > m_leadingJetPt);
  passJetPts &= (!m_subleadingJetPt || jets[1].Pt() > m_subleadingJetPt);

  if (passJetPts) m_n_passJetPts++;
  return passJetPts;

}

bool DerivationFramework::SkimmingToolEXOT14::SubcutJetDEta(const LeadingJets_t& jets) const {

  const double JetsDEta = fabs(jets[0].Eta() - jets[1].Eta());
  const bool passJetsDEta = JetsDEta > m_etaSeparation;

  if (passJetsDEta) m_n_passJetsDEta++;
  return passJetsDEta;

}


bool DerivationFramework::SkimmingToolEXOT14::SubcutDijetMass(const LeadingJets_t& jets) const {

  const double DiJetMass = (jets[0] + jets[1]).M();
  const bool passDiJetMass = DiJetMass > m_dijetMass;

  if (passDiJetMass) m_n_passDiJetMass++;
  return passDiJetMass;

}

bool DerivationFramework::SkimmingToolEXOT14::SubcutJetDPhi(const LeadingJets_t& jets) const {

  const double JetsDPhi = fabs(jets[0].DeltaPhi(jets[1]));
  const bool passJetsDPhi = JetsDPhi < m_jetDPhi;

  if (passJetsDPhi) m_n_passJetsDPhi++;
  return passJetsDPhi;

}

std::string DerivationFramework::SkimmingToolEXOT14::TriggerVarName(std::string s) const {
  std::replace(s.begin(), s.end(), '-', '_'); return s;
}



