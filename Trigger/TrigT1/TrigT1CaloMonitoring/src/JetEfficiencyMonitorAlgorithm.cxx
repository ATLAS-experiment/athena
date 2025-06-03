/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetEfficiencyMonitorAlgorithm.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "FourMomUtils/P4Helpers.h"

#include <cstdlib>

using Athena::Units::GeV;

static const std::map<std::string, int> l1_trigger_flatline_vals = {
  {"L1_gLJ80p0ETA25", 175*GeV}, {"L1_gLJ100p0ETA25", 200*GeV},
  {"L1_gLJ140p0ETA25", 270*GeV}, {"L1_gLJ160p0ETA25", 270*GeV}, 
  {"L1_SC175-SCjJ10", 270*GeV}, {"L1_gJ20p0ETA25", 40*GeV},
  {"L1_gJ50p0ETA25", 80*GeV}, {"L1_gJ100p0ETA25", 200*GeV},
  {"L1_gJ400p0ETA25", 800*GeV}, {"L1_jJ30", 50*GeV}, 
  {"L1_jJ40", 60*GeV}, {"L1_jJ50", 70*GeV}, 
  {"L1_jJ60", 80*GeV}, {"L1_jJ80", 100*GeV}, 
  {"L1_jJ90", 110*GeV}, {"L1_jJ125", 135*GeV}, 
  {"L1_jJ140", 160*GeV}, {"L1_jJ160", 180*GeV},
  {"L1_jJ180", 200*GeV}
};

// Define gFEX_trigger_thresholds as a static const map
// can add triggers and their corresponding threshold value as: {"L1_gLJ80p0ETA25", 100 * GeV}
// if the value is easily extracted from the trigger name, then just use the trigger name
// use trigger name from the function extractgFEXThresholdValue
static const std::map<std::string, int> gFEX_trigger_thresholds = {};
int JetEfficiencyMonitorAlgorithm::extractgFEXThresholdValue(const std::string& key) const {
  std::regex pattern(R"(L1_g(?:LJ|J)(\d+)(?:p0ETA25)?)");
  std::smatch match;
  if (std::regex_search(key, match, pattern)) {
      int threshold = std::stoi(match[1]) * GeV;
      if (key.find("gLJ") != std::string::npos) { //the gLJ threshold is actually 30 GeV less than the threshold in the name
          threshold -= 30 * GeV;
      }
      if (threshold < 0) {
        ATH_MSG_WARNING("Extracted threshold for " << key << " is negative!");
      }
      ATH_MSG_DEBUG("Trigger " << key << ": extracted threshold " << threshold );
      return threshold;
  } else {
      ATH_MSG_WARNING("Trigger " << key << " threshold not extracted!");
      return -1; // Error indicator
  }
}



JetEfficiencyMonitorAlgorithm::JetEfficiencyMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JetEfficiencyMonitorAlgorithm::initialize() {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);

  // we initialise all the containers that we need
  ATH_CHECK(m_jetKey.initialize() ); //initialize offline SR jets
  ATH_CHECK(m_LRjetKey.initialize() ); //initialize offline LR jets
  ATH_CHECK(m_gFexLRJetContainerKey.initialize(SG::AllowEmpty)); //initizlize gfex lr jets
  ATH_CHECK(m_gFexSRJetContainerKey.initialize(SG::AllowEmpty)); //initizlize gfex sr jets

  return AthMonitorAlgorithm::initialize();
}

StatusCode JetEfficiencyMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorithm::fillHistograms");
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

  //  Retrieve Offline Jets from SG
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
  if(!jets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline Small Radius Jet Container " << m_jetKey<< ". Will be skipped!");
  }
  //  Retrieve Offline LR Jets from SG
  SG::ReadHandle<xAOD::JetContainer> LRjets(m_LRjetKey,ctx);
  if(!LRjets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline Large Radius Jet Container "<<m_LRjetKey<< ". Will be skipped!");
  }

  // reterive the gFEX Jets from SG, only if we plan on accessing them! 
 // Declare the map of gFEX contianers
  std::map<std::string, SG::ReadHandle<xAOD::gFexJetRoIContainer>> gFEX_Container;

  // Retrieve gFex SR Jets from SG
  if (!m_gFexSRJetContainerKey.empty()) {
    SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexSRJetContainer{m_gFexSRJetContainerKey, ctx};
    if (!gFexSRJetContainer.isValid()) {
        ATH_MSG_WARNING("No gFex Small Radius Jet container found in storegate " << m_gFexSRJetContainerKey);
    } else {
        gFEX_Container.emplace("leadingGfex_SmallRadiusTOB", gFexSRJetContainer);
    }
  } 

  // Retrieve gFex LR Jets from SG
  if (!m_gFexLRJetContainerKey.empty()) {
    SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexLRJetContainer{m_gFexLRJetContainerKey, ctx};
    if (!gFexLRJetContainer.isValid()) {
        ATH_MSG_WARNING("No gFex Large Radius Jet container found in storegate " << m_gFexLRJetContainerKey);
    } else {
        gFEX_Container.emplace("leadingGfex_LargeRadiusTOB", gFexLRJetContainer);
    }
  }

  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //DEFINITIONS and extracting variables from the python config file!
  bool use_passed_before_prescale =  m_passedb4Prescale;
  const std::string& bootstrap_trigger = m_bootstrap_reference_trigger.value();
  const std::vector<std::string>& muon_triggers = m_muon_reference_triggers.value();
  const std::vector<std::string>& HLTrandom_triggers = m_HLTrandom_reference_triggers.value();
  std::vector<std::string> gFex_types {"leadingGfex_SmallRadiusTOB", "leadingGfex_LargeRadiusTOB"};
  
  
  //Define the various reference vector things!
  std::vector<std::string> reference_trigger_options {"Bootstrap",  "RandomHLT", "No", "Muon"};
  
  bool bootstrap_ref_decision = false; //bootstrap trigger decision 
  bool random_ref_decision = false; //random reference triggers decision
  bool muon_ref_decision = false; //muon reference triggers decision
  


  bootstrap_ref_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(bootstrap_trigger);
  for (auto & u : muon_triggers) {
    if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {muon_ref_decision = true;}
  } //close iterating through the muon triggers

  //then for the HLT decision chains, we always want to use the traditional way of getting our trigger deicsion
  for (auto & u : HLTrandom_triggers) {
    if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {random_ref_decision = true;}
  } //close iterating through the HLT random chains

  
  std::map<std::string, bool> reference_trigger_decision {
    {"Bootstrap", bootstrap_ref_decision },
    {"RandomHLT", random_ref_decision},
    {"No", true},
    {"Muon", muon_ref_decision}
  };
  
  
  //definition of variables for the offlineSRJet_maxEta_minPt_requirement and offlineLRJet_maxEta_minPt_requirement
  //these just force us to have a minimum pt, and limited eta region for our efficiency checks 
  constexpr int minPt = 10*GeV;
  constexpr float maxEta = 2.5;
  
  //fill maps that allow us to keep track of variables according to the different containers
  float offline_SR_pt = 0, offline_LR_pt = 0, gfex_SR_pt = 0, gfex_LR_pt=0;
  std::map<std::string, float> jet_pt {
    {"leadingOffline_SmallRadiusJet", offline_SR_pt}, 
    {"leadingOffline_LargeRadiusJet", offline_LR_pt},
    {"leadingGfex_SmallRadiusTOB", gfex_SR_pt}, 
    {"leadingGfex_LargeRadiusTOB", gfex_LR_pt}
  };
  
  float offline_SR_eta = 0, offline_LR_eta = 0, gfex_SR_eta = 0, gfex_LR_eta=0;
  std::map<std::string, float> jet_eta {
    {"leadingOffline_SmallRadiusJet", offline_SR_eta}, 
    {"leadingOffline_LargeRadiusJet", offline_LR_eta},
    {"leadingGfex_SmallRadiusTOB", gfex_SR_eta}, 
    {"leadingGfex_LargeRadiusTOB", gfex_LR_eta}
  };
  
  float offline_SR_phi = 0, offline_LR_phi = 0, gfex_SR_phi = 0, gfex_LR_phi=0;
  std::map<std::string, float> jet_phi {
    {"leadingOffline_SmallRadiusJet", offline_SR_phi}, 
    {"leadingOffline_LargeRadiusJet", offline_LR_phi},
    {"leadingGfex_SmallRadiusTOB", gfex_SR_phi}, 
    {"leadingGfex_LargeRadiusTOB", gfex_LR_phi}
  };
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Fill pt, eta and phi vals for all the containers!
  //offline jet containers
  if (jets.isValid()){
    if (!jets->empty()) { //check that there are jets before accessing the container
      xAOD::JetContainer::const_iterator leading_offline_SR_jet = jets->begin(); //the first jet in the contianer is the leading jet
      jet_pt["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->pt();
      jet_eta["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->eta();
      jet_phi["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->phi();
    }
  } //(close IF) jets size > 0 loop
  //LR jet containers
  if (LRjets.isValid()){
    if (!LRjets->empty()) { //check that there are jets before accessing the container
      xAOD::JetContainer::const_iterator leading_offline_LR_jet = LRjets->begin(); //the first jet in the contianer is the leading jet
      jet_pt["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->pt();
      jet_eta["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->eta();
      jet_phi["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->phi();
    } //(close IF) LRjets size > 0 loop
  }
  // gFex SR and LR TOB containers
  // when we emulate the gfex trigger decision, we just want to find the leading gFEX TOB
  // its not gaurenteed that these TOBs are pt ordered, so well iterate through them 
  float max_gfex_pt = -1.0; // Track max pt
  const xAOD::gFexJetRoI* most_energetic_gfex_jet = nullptr;

  for (auto & g : gFex_types) { // Iterate through SR and LR gFex jets
    if(auto itr = gFEX_Container.find(g); itr != gFEX_Container.end() ) { //check that we were able to access the contianer
        for (const auto* gfex_jet : *(itr->second)) {// Iterate through gFex jets
            float gfex_pt = gfex_jet->et();
            if (gfex_pt > max_gfex_pt && std::abs(gfex_jet->eta()) < maxEta) { // Track highest pt jet
                max_gfex_pt = gfex_pt;
                most_energetic_gfex_jet = gfex_jet;
            } 
        }
        if(most_energetic_gfex_jet) {
          jet_eta[g] = most_energetic_gfex_jet->eta();
          jet_phi[g] = most_energetic_gfex_jet->phi();
          jet_pt[g] = most_energetic_gfex_jet->et();
        }
      }
  }
    
  // #####################
  // #####################
  //Physical cuts applied to all events on the offline jets
  //requring a minimum pt threshold 
  //and maximum eta threshold 

  //offline SR Jet requriment 
  bool  offlineSRJet_maxEta_minPt_requirement = false;
  if(std::abs(jet_eta["leadingOffline_SmallRadiusJet"])<=maxEta && (jet_pt["leadingOffline_SmallRadiusJet"] >= minPt)) { 
     offlineSRJet_maxEta_minPt_requirement = true; 
  }
  
  // offline LR Jet requriment 
  bool  offlineLRJet_maxEta_minPt_requirement = false;
  if(std::abs(jet_eta["leadingOffline_LargeRadiusJet"])<=maxEta && (jet_pt["leadingOffline_LargeRadiusJet"] >= minPt)) {  
    offlineLRJet_maxEta_minPt_requirement = true; 
  }

  // #####################
  // #####################
  // Fill sample histograms of the pt and eta of leading SR jet
  if (offlineSRJet_maxEta_minPt_requirement ) {
    auto raw_pt  = Monitored::Scalar<float>("raw_pt", jet_pt["leadingOffline_SmallRadiusJet"]);
    auto raw_eta  = Monitored::Scalar<float>("raw_eta", jet_eta["leadingOffline_SmallRadiusJet"]);
    fill(m_packageName, raw_pt, raw_eta);
  }
 // #####################
 // #####################


  // FILL EFFIENCY HISTOGRAMS INVOLVING SMALL RADIUS OFFLINE JETS
  for (auto & r : reference_trigger_options){ //iterate through the refernce triggers
    if (offlineSRJet_maxEta_minPt_requirement && reference_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      //get the pt of leading jet 
      auto pt_ref  = Monitored::Scalar<float>("val_SRpt", jet_pt["leadingOffline_SmallRadiusJet"]);
      
      for (const auto& trigger_name : m_SmallRadiusJetTriggers_phase1){
        bool trig_of_interest_decision = false; //default definition of the trigger of interest decison to be false,
        if (use_passed_before_prescale) {
          //We can choose if we want to use pass before prescale, or not when defining our trigger efficiency
          //this boolean is defiend in the jeteffmonalg.py file
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
         //get values and fill the histogram of offline jet pt and boolean of trigger passing
        auto passed_pt_bool  = Monitored::Scalar<bool>("bool_"+r+"_"+trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt_bool);
        
         //filling histograms that are effiency curves as a funciton of eta
         //in order to ensure that we are isolating only the eta behavior, we have a
         // flatline value where the pt effiencies aproximtley flatten out to 1
         //these are hard coded, and saved for only a few of the triggers!
        auto l1_trigger_flat_val = l1_trigger_flatline_vals.find(trigger_name);
        if (l1_trigger_flat_val != l1_trigger_flatline_vals.end()) {
          if(jet_pt["leadingOffline_SmallRadiusJet"]>l1_trigger_flat_val->second) { //is jet pt greater than the flatline value?
             //get value of eta, and histogram passing boolean and fill
            auto eta_ref  = Monitored::Scalar<float>("val_SReta", jet_eta["leadingOffline_SmallRadiusJet"]);
            auto passed_eta = Monitored::Scalar<bool>("bool_" + r + "_" + trigger_name, trig_of_interest_decision);
            fill(m_packageName, eta_ref, passed_eta);
              //if the trigger passes, we can also add the eta value to a stand alone histogram
          } //(close IF) jet pt is greater than pt flatline vlaue loop
        } //(close IF) loop that checks if the trigger of interest is in list of flatline trigger vals
      } //(close FOR) loop that iterates through all of L1 single jet triggers we make effiency curves for

      // if a gFEX trigger is not defined in the menu, then we populate the turn on curve using TOBs
      // check the map to see if there is a predefined 
      for (const auto& trigger_name : m_SmallRadiusJetTriggers_gFEX) {
        bool trig_of_interest_decision = false; // Default to false
        int gFEX_threshold = -1; // Default invalid threshold value
          // Check if the trigger name exists in the map first
        auto gFEX_threshold_it = gFEX_trigger_thresholds.find(trigger_name);
        if (gFEX_threshold_it != gFEX_trigger_thresholds.end()) {
            gFEX_threshold = gFEX_threshold_it->second;
            ATH_MSG_DEBUG("gfex threshold in map! "<< gFEX_threshold);
        } else {
            // If not found in the map, extract the threshold dynamically
            gFEX_threshold = extractgFEXThresholdValue(trigger_name);
        }
        ATH_MSG_DEBUG("gfex threshold used: "<< gFEX_threshold);
        // Proceed only if a valid threshold was obtained
        if (gFEX_threshold != -1) {
            if (jet_pt["leadingGfex_SmallRadiusTOB"] >= gFEX_threshold) {
                trig_of_interest_decision = true;
            }
        }
        // Get values and fill the histogram of offline jet pt and boolean of trigger passing
        auto passed_pt_bool_gFEX = Monitored::Scalar<bool>("bool_" + r + "_" + trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt_bool_gFEX);
      } 
      
    } //(close IF) loop that checks if the reference trigger and physical property pass is passed
  } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //FILL EFFIENCY HISTOGRAMS INVOLVING LARGE RADIUS OFFLINE JETS
  
  for (auto & r : reference_trigger_options){ //iterate through the reference triggers
    if ( offlineLRJet_maxEta_minPt_requirement && reference_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      auto pt_ref  = Monitored::Scalar<float>("val_LRpt", jet_pt["leadingOffline_LargeRadiusJet"]);

      for (const auto& trigger_name : m_LargeRadiusJetTriggers_phase1){
        bool trig_of_interest_decision = false;
        if (use_passed_before_prescale) {
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        
        auto passed_pt_bool  = Monitored::Scalar<bool>("bool_"+r+"_"+trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt_bool);


         //filling histograms that are effiency curves as a funciton of eta
        auto l1_trigger_flat_val = l1_trigger_flatline_vals.find(trigger_name);
        if (l1_trigger_flat_val != l1_trigger_flatline_vals.end()) {
          if(jet_pt["leadingOffline_LargelRadiusJet"]>l1_trigger_flat_val->second) { //is jet pt greater than the flatline value?
             //get value of eta, and histogram passing boolean and fill
            auto eta_ref  = Monitored::Scalar<float>("val_LReta", jet_eta["leadingOffline_LargeRadiusJet"]);
            auto passed_eta = Monitored::Scalar<bool>("bool_" + r + "_" + trigger_name, trig_of_interest_decision);
            fill(m_packageName, eta_ref, passed_eta);
              //if the trigger passes, we can also add the eta value to a stand alone histogram
          } //(close IF) jet pt is greater than pt flatline vlaue loop
        } //(close IF) loop that checks if the trigger of interest is in list of flatline trigger vals
        for (const auto& trigger_name : m_LargeRadiusJetTriggers_gFEX) {
          bool trig_of_interest_decision = false; // Default to false
          int gFEX_threshold = -1; // Default invalid threshold value
            // Check if the trigger name exists in the map first
          auto gFEX_threshold_it = gFEX_trigger_thresholds.find(trigger_name);
          if (gFEX_threshold_it != gFEX_trigger_thresholds.end()) {
              gFEX_threshold = gFEX_threshold_it->second;
          } else {
              // If not found in the map, extract the threshold dynamically
              gFEX_threshold = extractgFEXThresholdValue(trigger_name);
          }
          // Proceed only if a valid threshold was obtained
          if (gFEX_threshold != -1) {
              if (jet_pt["leadingGfex_LargeRadiusTOB"] >= gFEX_threshold) {
                  trig_of_interest_decision = true;
              }
          }
          // Get values and fill the histogram of offline jet pt and boolean of trigger passing
          auto passed_pt_bool_gFEX = Monitored::Scalar<bool>("bool_" + r + "_" + trigger_name, trig_of_interest_decision);
          fill(m_packageName, pt_ref, passed_pt_bool_gFEX);
        } 
      } //(close FOR) loop that iterates through all of the triggers we make effiency curves for
    } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  } //(close IF) loop that checks if the physical properties were passed for the jet
  
  variables.clear();
  return StatusCode::SUCCESS;
}
