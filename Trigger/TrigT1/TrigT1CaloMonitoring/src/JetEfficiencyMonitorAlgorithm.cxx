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

JetEfficiencyMonitorAlgorithm::JetEfficiencyMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JetEfficiencyMonitorAlgorithm::initialize() {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);

  // we initialise all the containers that we need
  ATH_CHECK(m_jetKey.initialize()); //initialize offline SR jets
  ATH_CHECK(m_LRjetKey.initialize()); //initialize offline LR jets
  ATH_CHECK(m_gFexLRJetContainerKey.initialize()); //initizlize gfex lr jets
  ATH_CHECK(m_gFexSRJetContainerKey.initialize()); //initizlize gfex sr jets

  // TOBs may come from trigger bytestream - renounce from scheduler
  renounce(m_LRjetKey);
  renounce(m_gFexLRJetContainerKey);
  renounce(m_gFexSRJetContainerKey);

  return AthMonitorAlgorithm::initialize();
}

StatusCode JetEfficiencyMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  ATH_MSG_DEBUG("JetEfficiencyMonitorAlgorithm::fillHistograms");
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;
  
  //  Retrieve Offline Jets from SG
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
  if(!jets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline Small Radius Jet Container");
    return StatusCode::SUCCESS;
  }
  //  Retrieve Offline LR Jets from SG
  SG::ReadHandle<xAOD::JetContainer> LRjets(m_LRjetKey,ctx);
  if(!LRjets.isValid()){
    ATH_MSG_WARNING("Failed to retrieve Offline Large Radius Jet Container");
    return StatusCode::SUCCESS;
  }
  //  Retrieve gfex SR Jets from SG
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexSRJetContainer{m_gFexSRJetContainerKey, ctx};
  if(!gFexSRJetContainer.isValid()){
    ATH_MSG_WARNING("No gFex Small Radius Jet container found in storegate  "<< m_gFexSRJetContainerKey);
    return StatusCode::SUCCESS;
  }
  //  Retrieve gfex LR Jets from SG
  SG::ReadHandle<xAOD::gFexJetRoIContainer> gFexLRJetContainer{m_gFexLRJetContainerKey, ctx};
  if(!gFexLRJetContainer.isValid()){
    ATH_MSG_WARNING("No gFex Large Radius Jet container found in storegate  "<< m_gFexLRJetContainerKey);
    return StatusCode::SUCCESS;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //DEFINITIONS and extracting variables from the python config file!
  bool use_emulated_gfex_trig =  m_emulated;
  bool use_passed_before_prescale =  m_passedb4Prescale;
  const std::string& HLTbootstrap_trigger = m_HLTbootstrap_reference_trigger.value();
  const std::string& bootstrap_trigger = m_bootstrap_reference_trigger.value();
  const std::vector<std::string>& muon_triggers = m_muon_reference_triggers.value();
   const std::vector<std::string>& HLTrandom_triggers = m_HLTrandom_reference_triggers.value();
  std::vector<std::string> gFex_types {"leadingGfex_SmallRadiusTOB", "leadingGfex_LargeRadiusTOB"};
  
  
  //Define the various reference vector things!
  std::vector<std::string> reference_trigger_options {"Bootstrap", "BootstrapHLT", "RandomHLT", "No", "Muon"};
  
  bool bootstrap_ref_decision = false; //bootstrap trigger decision 
  bool bootstrapHLT_ref_decision = false; //HLT bootstrap trigger chain decision 
  bool random_ref_decision = false; //random reference triggers decision
  bool muon_ref_decision = false; //muon reference triggers decision
  


  //if using pass before prescale to increase statstics, we determine reference trigger decision differently
  if (use_passed_before_prescale) {
    const unsigned int bs_bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(bootstrap_trigger);
    bootstrap_ref_decision = bs_bits & TrigDefs::L1_isPassedBeforePrescale;
    for (auto & u : muon_triggers) {
      const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(u);
      bool pass = bits & TrigDefs::L1_isPassedBeforePrescale;
      if (pass) {muon_ref_decision = true;}
    } // iterating through the list of unbaised triggers
  } //close if used pass before prescale
  // if not using pass before prescale, then is a more direct process to see if trigger passed
  else {
    bootstrap_ref_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(bootstrap_trigger);
    for (auto & u : muon_triggers) {
      if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {muon_ref_decision = true;}
    } //close iterating through the muon triggers
  } //close else

  //then for the HLT decision chains, we always want to use the traditional way of getting our trigger deicsion
  bootstrapHLT_ref_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(HLTbootstrap_trigger);
  for (auto & u : HLTrandom_triggers) {
    if (AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(u)) {random_ref_decision = true;}
  } //close iterating through the HLT random chains

  
  std::map<std::string, bool> reference_trigger_decision {
    {"Bootstrap", bootstrap_ref_decision },
    {"BootstrapHLT", bootstrapHLT_ref_decision },
    {"RandomHLT", random_ref_decision},
    {"No", true},
    {"Muon", muon_ref_decision}
  };
  
  
  //fill a simple histogram of just the run number
  auto run = Monitored::Scalar<int>("run",GetEventInfo(ctx)->runNumber());
  fill(m_packageName, run); //fill the run number histogram that was default
  
  
  //definition of variables for the offlineSRJet_maxEta_minPt_requirement and offlineLRJet_maxEta_minPt_requirement
  //these just force us to have a minimum pt, and limited eta region for our efficiency checks 
  constexpr int minPt = 10*GeV;
  constexpr float maxEta = 2.5;
  
  std::map<std::string, SG::ReadHandle<xAOD::gFexJetRoIContainer>> gFEX_Container {
    //this naming is a bit misleading but it allows us to easily check the LR and SR gfex TOB contianers 
    //to find the leading LR and SR gfex TOBS (becuase they are not PT ordered by default)
    //the string descripters are this way to stay conistent with the naming used later when we want to write 
    //and access the values of LR and SR gfex TOB properties, like pt, eta and phi
    {"leadingGfex_SmallRadiusTOB",gFexSRJetContainer}, {"leadingGfex_LargeRadiusTOB", gFexLRJetContainer}
  };
  
  //fill maps that allow us to keep track of variables according to the different containers
  float offline_SR_pt = 0, offline_LR_pt = 0, gfex_SR_pt = 0, gfex_LR_pt=0;
  std::map<std::string, float> jet_pt {
    {"leadingOffline_SmallRadiusJet", offline_SR_pt}, {"leadingOffline_LargeRadiusJet", offline_LR_pt}, 
    {"leadingGfex_SmallRadiusTOB", gfex_SR_pt}, {"leadingGfex_LargeRadiusTOB", gfex_LR_pt}
  };
  
  float offline_SR_eta = 0, offline_LR_eta = 0, gfex_SR_eta = 0, gfex_LR_eta=0;
  std::map<std::string, float> jet_eta {
    {"leadingOffline_SmallRadiusJet", offline_SR_eta}, {"leadingOffline_LargeRadiusJet", offline_LR_eta},
    {"leadingGfex_SmallRadiusTOB", gfex_SR_eta}, {"leadingGfex_LargeRadiusTOB", gfex_LR_eta}
  };
  
  float offline_SR_phi = 0, offline_LR_phi = 0, gfex_SR_phi = 0, gfex_LR_phi=0;
  std::map<std::string, float> jet_phi {
    {"leadingOffline_SmallRadiusJet", offline_SR_phi}, {"leadingOffline_LargeRadiusJet", offline_LR_phi}, 
    {"leadingGfex_SmallRadiusTOB", gfex_SR_phi}, {"leadingGfex_LargeRadiusTOB", gfex_LR_phi}
  };
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Fill pt, eta and phi vals for all the containers!
  //offline jet containers
  if (!jets->empty()) { //check that there are jets before accessing the container
    xAOD::JetContainer::const_iterator leading_offline_SR_jet = jets->begin(); //the first jet in the contianer is the leading jet
    jet_pt["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->pt();
    jet_eta["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->eta();
    jet_phi["leadingOffline_SmallRadiusJet"] = (*leading_offline_SR_jet)->phi();
  } //(close IF) jets size > 0 loop
  //LR jet containers
  if (!LRjets->empty()) { //check that there are jets before accessing the container
    xAOD::JetContainer::const_iterator leading_offline_LR_jet = LRjets->begin(); //the first jet in the contianer is the leading jet
    jet_pt["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->pt();
    jet_eta["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->eta();
    jet_phi["leadingOffline_LargeRadiusJet"] = (*leading_offline_LR_jet)->phi();
  } //(close IF) LRjets size > 0 loop
  
  // gFex SR and LR TOB containers
  // when we emulate the gfex trigger decision, we just want to find the gfex tob that 
  // is the closest in physical space to the leading offline pt jet
  float maximum_value_for_delta_r = 0.4;
  for (auto & g : gFex_types){ //iterate through SR and LR gfex jets.
    if (!gFEX_Container[g]->empty()) { //check that there are jets before accessing the container
      const xAOD::gFexJetRoI* matching_gfex_jet = nullptr; //inital pointer of null
      float min_delta_r = 10000;
      //iterate through all the gfex tobs 
      for (const auto* gfex_jet : *gFEX_Container[g]) {
        std::string comparison_offline_jet = "";
        if (g == "gfex_SmallRadiusTOB") {comparison_offline_jet = "offline_SmallRadiusJet"; }
        else if (g == "gfex_LargeRadiusTOB") {comparison_offline_jet = "offline_LargeRadiusJet"; }
        float gfex_eta = gfex_jet->eta();
        float gfex_phi = gfex_jet->phi();
        float delta_eta = std::abs(gfex_eta-jet_eta[comparison_offline_jet]),
        delta_phi = P4Helpers::deltaPhi(gfex_phi,jet_phi[comparison_offline_jet]),
        delta_r = std::sqrt(std::pow(delta_eta,2)+ std::pow(delta_phi,2));
        if (maximum_value_for_delta_r > delta_r && delta_r < min_delta_r && std::abs(gfex_eta) < maxEta){
          min_delta_r = delta_r;
          matching_gfex_jet = gfex_jet;
        } //(close IF) loop if gfex jet satisfies pt and eta conditions
      } //(close FOR) loop that iterates through gfex tobs for the given event

      //if we successfully found a leading gfex tob, then we can save its physics properties 
      if (matching_gfex_jet != nullptr) {
        jet_eta[g] = matching_gfex_jet->eta(), jet_phi[g] = matching_gfex_jet->phi(), jet_pt[g] = matching_gfex_jet->et();
      } // (close IF) loop that checks if there is a leading gfex jet
    } // (close IF) loop that checks that there are gfex tobs
  }// (close FOR) that loops through sr and lr gfex tobs

  
  
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Physical cuts applied to all events on the offline jets
  //requring a minimum pt threshold (Defined as minPt @ line 120)
  //and maximum eta threshold (Defined as maxEta @ line 121)

  //offline SR Jet requriment 
  bool  offlineSRJet_maxEta_minPt_requirement = false;
  if(std::abs(jet_eta["leadingOffline_SmallRadiusJet"])<=maxEta && (jet_pt["leadingOffline_SmallRadiusJet"] >= minPt)) {  offlineSRJet_maxEta_minPt_requirement = true; }
  
  // offline LR Jet requriment 
  bool  offlineLRJet_maxEta_minPt_requirement = false;
  if(std::abs(jet_eta["leadingOffline_LargeRadiusJet"])<=maxEta && (jet_pt["leadingOffline_LargeRadiusJet"] >= minPt)) {  offlineLRJet_maxEta_minPt_requirement = true; }
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //PREP LISTS OF TRIGGERS
  
  //Create and clean up the list of L1Triggers
  std::vector<std::string> multiJet_LegacySmallRadiusTriggers = m_multiJet_LegacySmallRadiusTriggers;
  std::vector<std::string> SmallRadiusJetTriggers_phase1_and_legacy = m_SmallRadiusJetTriggers_phase1_and_legacy;
  std::vector<std::string> LargeRadiusJetTriggers_phase1_and_legacy = m_LargeRadiusJetTriggers_phase1_and_legacy;
  
  std::map<std::string, int> l1_trigger_flatline_vals {
    //this is around where the l1trigger pt effiencies flatten out
    //in order to make eta effiency curves, its useful to isolate the pt behavior,
    // so we want 100% pt effiency, or after 'flattening out'
    {"L1_J15", 35*GeV}, {"L1_J20", 40*GeV},  {"L1_J30", 60*GeV},
    {"L1_J40", 80*GeV}, {"L1_J50", 90*GeV}, {"L1_J75", 120*GeV},
    {"L1_J100", 140*GeV}
  };
  
  /////////////////////////////////////////////////////////////
  std::map<std::string, float> gFexTriggers_val {
    //trigger val corresponding to the gfex triggers
    {"L1_gJ20", 20*GeV}, {"L1_gJ30", 30*GeV},  {"L1_gJ40", 40*GeV},
    {"L1_gJ50", 50*GeV}, {"L1_gJ60", 60*GeV}, {"L1_gJ100", 100*GeV},
    {"L1_gJ160", 160*GeV}, {"L1_gLJ80", 80*GeV}, {"L1_gLJ100", 100*GeV},
    {"L1_gLJ140", 140*GeV},  {"L1_gLJ160", 160*GeV}
  };
  
  std::map<std::string, std::vector<float>> multijet_triggers_val {
    {"L1_3J50", {50*GeV, 50*GeV, 50*GeV}}, {"L1_4J15", {15*GeV, 15*GeV, 15*GeV, 15*GeV}},
    {"L1_4J20", {20*GeV, 20*GeV, 20*GeV, 20*GeV}}, {"L1_J85_3J30", {85*GeV, 30*GeV, 30*GeV, 30*GeV}},
    {"L1_2J15_XE55", {15*GeV, 15*GeV}}, {"L1_2J50_XE40", {50*GeV, 50*GeV}}
  };
  
  //define emulatedString variable so that we can have emulated in the title
  //(or not) according to the status of the gfex triggers
  std::string emulatedString = " ";
  if (use_emulated_gfex_trig) {emulatedString = " Emulated Decision "; }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Fill sample histograms of the pt of leading jet
  auto raw_pt  = Monitored::Scalar<float>("raw_pt", 0.);
  raw_pt = jet_pt["leadingOffline_SmallRadiusJet"];
  fill(m_packageName, raw_pt);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // FILL EFFIENCY HISTOGRAMS INVOLVING SMALL RADIUS OFFLINE JETS
  
  for (auto & r : reference_trigger_options){ //iterate through the refernce triggers
    if (offlineSRJet_maxEta_minPt_requirement && reference_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      for(unsigned int t=0; t< SmallRadiusJetTriggers_phase1_and_legacy.size(); ++t) {//iterate through all of the useful triggers list (that we make effiency curves for)
        const std::string& trigger_name = SmallRadiusJetTriggers_phase1_and_legacy[t]; // define the trigger name and get its decision
        
        //default definition of the trigger of interest decison to be false,
        //we will then check if the trigger actually passed 
        bool trig_of_interest_decision = false;
        
        //check if we are looking at a gfex trigger and if we want to emulate the gfex trigger
        if ((gFexTriggers_val.find(trigger_name) != gFexTriggers_val.end()) && use_emulated_gfex_trig) {
          // check if the emuaulated trigger passed
          // we have threshold values of gfex trigger values saved in the map 
          // gFexTriggers_val that connects the trigger name to the value
          // here we check if the gfex SR TOB pt is large than the threshold, which should mean the trigger passes
          if (jet_pt["leadingGfex_SmallRadiusTOB"] > gFexTriggers_val[trigger_name]) { 
            trig_of_interest_decision = true;
          }
        } else if (use_passed_before_prescale) {
          //We can choose if we want to use pass before prescale, or not when defining our trigger efficiency
          //this boolean is defiend in the jeteffmonalg.py file
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        //get values and fill the histogram of offline jet pt and boolean of trigger passing
        auto pt_ref  = Monitored::Scalar<float>("pt_"+ r, jet_pt["leadingOffline_SmallRadiusJet"]);
        auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt);
        
        //if loop to see if trigger of interest passed. If yes we want to fill a histogram with the pt value
        if (trig_of_interest_decision) {
          auto passed_pt_val = Monitored::Scalar<float>("pt:" + r +  "_" + trigger_name, jet_pt["leadingOffline_SmallRadiusJet"]);
          fill(m_packageName, passed_pt_val);
        } //(close IF) trig_of_interest_decision loop
        
        //filling histograms that are effiency curves as a funciton of eta
        //in order to ensure that we are isolating only the eta behavior, we have a
        // flatline value where the pt effiencies aproximtley flatten out to 1
        //these are hard coded, and saved for only a few of the triggers!
        if (l1_trigger_flatline_vals.find(trigger_name) != l1_trigger_flatline_vals.end()) {
          if(jet_pt["leadingOffline_SmallRadiusJet"]>l1_trigger_flatline_vals[trigger_name]) { //is jet pt greater than the flatline value?
            //get value of eta, and histogram passing boolean and fill
            auto eta_ref  = Monitored::Scalar<float>("eta_" + r, jet_eta["leadingOffline_SmallRadiusJet"]);
            auto passed_eta = Monitored::Scalar<bool>("eta_" + r + "_" + trigger_name, trig_of_interest_decision);
            fill(m_packageName, eta_ref, passed_eta);
            //if the trigger passes, we can also add the eta value to a stand alone histogram
            if (trig_of_interest_decision) {
              auto passed_eta_val  = Monitored::Scalar<float>("eta:" + r +"_" + trigger_name, jet_eta["leadingOffline_SmallRadiusJet"]);
              fill(m_packageName, passed_eta_val);
            } //(close IF) passed eta if loop
          } //(close IF) jet pt is greater than pt flatline vlaue loop
        } //(close IF) loop that checks if the trigger of interest is int eh
      } //(close FOR) loop that iterates through all of L1 single jet triggers we make effiency curves for
      
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      //now lets iterate through the multi jet triggers to make efficiency curves!
      for(unsigned int t=0; t< multiJet_LegacySmallRadiusTriggers.size(); ++t) {
        const std::string& trigger_name = multiJet_LegacySmallRadiusTriggers[t];
        
        //determine if the trigger passed
        bool trig_of_interest_decision = false;
        if (use_passed_before_prescale) {
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        // for these multijet triggers, we only want to fill the "last" jet 
        // of the trigger defintion 
        // example:
        ///// a 4J100 jet trigger?
        ///// we want to only plot the pt of the jet with the 4th highest pT 
        int multijet_num = multijet_triggers_val[trigger_name].size(); // number of jets we expect from trigger definition 
        int jets_num = jets->size(); //total number of jets in the event
        if (jets_num >= multijet_num) {
          //iterate through each jet required by trigger and check that the value is satisfied
          int jet_count = 0;
          for (const auto* j : *jets) {
            jet_count += 1;
            if(jet_count == multijet_num) { //only want to fill histogram on the last jet of the multijet
              const float jet_pt_loop = j->pt();
              auto pt_ref  = Monitored::Scalar<float>("pt_"+ r, jet_pt_loop);
              auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
              fill(m_packageName, pt_ref, passed_pt);
              if (trig_of_interest_decision) {
                auto passed_pt_val = Monitored::Scalar<float>("pt:" + r +  "_" + trigger_name, jet_pt_loop);
                fill(m_packageName, passed_pt_val);
              }  //(close IF) passed_pt if loop
            } //(close IF) loop that checks we are filling only the last jet of the multijet
          } //(close FOR) loop that iterates trhrough each jet required by the multijet trigger
        } //(close IF) loop that checks if the number of jets preseent agrees with the multijet trigger
      } //(close FOR) loop that iterates through the multijet triggers
      
    } //(close IF) loop that checks if the reference trigger and physical property pass is passed
  } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //FILL EFFIENCY HISTOGRAMS INVOLVING LARGE RADIUS OFFLINE JETS
  
  for (auto & r : reference_trigger_options){ //iterate through the reference triggers
    if ( offlineLRJet_maxEta_minPt_requirement && reference_trigger_decision[r]) { //check that the physical cuts and reference trigger is passed
      for(unsigned int t=0; t< LargeRadiusJetTriggers_phase1_and_legacy.size(); ++t){ //iterate through all of the useful LR triggers list (that we make effiency curves for)
        const std::string& trigger_name = LargeRadiusJetTriggers_phase1_and_legacy[t];
        
        bool trig_of_interest_decision = false;
        
        if ((gFexTriggers_val.find(trigger_name) != gFexTriggers_val.end()) && use_emulated_gfex_trig) {
          if (jet_pt["leadingGfex_LargeRadiusTOB"] > gFexTriggers_val[trigger_name]) { //the qualifier for emulating trigger passing
            trig_of_interest_decision = true;
          }
        } else if (use_passed_before_prescale) {
          const unsigned int bits = AthMonitorAlgorithm::getTrigDecisionTool()->isPassedBits(trigger_name);
          trig_of_interest_decision = bits & TrigDefs::L1_isPassedBeforePrescale;
        } else { trig_of_interest_decision = AthMonitorAlgorithm::getTrigDecisionTool()->isPassed(trigger_name); }
        
        auto pt_ref  = Monitored::Scalar<float>("pt_"+ r, jet_pt["leadingOffline_LargeRadiusJet"]);
        auto passed_pt  = Monitored::Scalar<bool>("pt_" + r + "_" + trigger_name, trig_of_interest_decision);
        fill(m_packageName, pt_ref, passed_pt);
        
        if (trig_of_interest_decision) { //if loop to see if trigger of interest passed. If yes we want to fill a histogram with the value
          auto passed_pt_val = Monitored::Scalar<float>("pt:" + r +  "_" + trigger_name, jet_pt["leadingOffline_LargeRadiusJet"]);
          fill(m_packageName, passed_pt_val);
        } //(close IF) trig_of_interest_decision loop
        
      } //(close FOR) loop that iterates through all of the triggers we make effiency curves for
    } //(close FOR) the iteration that fills effiency histogram for 4 different kinds of refernce triggers
  } //(close IF) loop that checks if the physical properties were passed for the jet
  
  variables.clear();
  return StatusCode::SUCCESS;
}
