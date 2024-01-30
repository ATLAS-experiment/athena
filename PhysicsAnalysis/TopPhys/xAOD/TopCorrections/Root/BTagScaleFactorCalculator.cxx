/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

// $Id: BTagScaleFactorCalculator.cxx 809570 2017-08-18 13:15:17Z iconnell $
#include "TopCorrections/BTagScaleFactorCalculator.h"
#include "TopCorrections/TopCorrectionsTools.h"
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

#include "xAODJet/JetContainer.h"
#include <cmath>
#include <algorithm>
#include <functional>

// For debug function
#include "PATInterfaces/SystematicCode.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceROOT.h"
#include "PathResolver/PathResolver.h"

namespace top {
  BTagScaleFactorCalculator::BTagScaleFactorCalculator(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),
    m_trigDecisionTool("Trig::TrigDecisionTool"),
    m_nominal(CP::SystematicSet()) {
    declareProperty("config", m_config);
  }

  StatusCode BTagScaleFactorCalculator::initialize() {
    ATH_MSG_INFO(" top::BTagScaleFactorCalculator initialize");

    // for calo jets
    std::vector<std::string> availableWPs = m_config->bTagWP_available();
    for (auto& WP : availableWPs) {
      m_btagSelTools[WP] = "BTaggingSelectionTool_" + WP + "_" + m_config->sgKeyJets();
      top::check(m_btagSelTools[WP].retrieve(), "Failed to retrieve b-tagging Selection tool");
      if (std::find(m_config->bTagWP_calibrated().begin(),
                    m_config->bTagWP_calibrated().end(), WP) != m_config->bTagWP_calibrated().end()) {// need
                                                                                                      // scale-factors
                                                                                                      // only for
                                                                                                      // calibrated WPs
        m_btagEffTools[WP] = "BTaggingEfficiencyTool_" + WP + "_" + m_config->sgKeyJets();
        top::check(m_btagEffTools[WP].retrieve(), "Failed to retrieve b-tagging Efficiency tool");
        m_systs[WP] = m_btagEffTools[WP]->affectingSystematics();
        std::set<std::string> base_names = m_systs[WP].getBaseNames();
        m_config->setBTaggingSFSysts(WP, base_names);

	if(WP.find("Online") != std::string::npos) m_onlineJets[WP]=std::map<std::string,std::vector<TLorentzVector>>();
      }
    }
    if(m_onlineJets.size()>0)
      {   
	//check for b-jet triggers
	std::shared_ptr<std::vector<std::string> > selectors = m_config->allSelectionNames();
	for (std::string selPtr : *selectors) {
	  for (auto& trigger : m_config->allTriggers_Tight(selPtr)) {
	    std::string trigger_name=trigger.first;
	    if(trigger_name.find("bmv2c") != std::string::npos) {

	      std::string sub = "bmv2c";
	      uint pos=trigger_name.find(sub)+sub.length()+2;
	      std::string the_wp = ((std::string)"FixedCutBEff_")+trigger_name.substr(pos,2);
	      for(auto& a_wp : m_onlineJets) {
		if(a_wp.first.find(the_wp)!=std::string::npos) {
		  m_onlineJets[a_wp.first][trigger_name] = std::vector<TLorentzVector>(0);
		  break;
		}
	      }
	    }
	  }
	}

	for(auto& a_wp : m_onlineJets) {
	  if(a_wp.second.size()==0) ATH_MSG_WARNING("No b-jet triggers triggers match the Online WP" + a_wp.first);
	}
	//get the tool
	top::check(m_trigDecisionTool.retrieve(), "Failed to retrieve TrigDecisionTool");	
      }
      
    
    // for track jets
    availableWPs = m_config->bTagWP_available_trkJet();
    for (auto& WP : availableWPs) {
      m_trkjet_btagSelTools[WP] = "BTaggingSelectionTool_" + WP + "_" + m_config->sgKeyTrackJets();
      top::check(m_trkjet_btagSelTools[WP].retrieve(), "Failed to retrieve b-tagging Selection tool");
      if (std::find(m_config->bTagWP_calibrated_trkJet().begin(),
                    m_config->bTagWP_calibrated_trkJet().end(), WP) != m_config->bTagWP_calibrated_trkJet().end()) {// need
                                                                                                                    // scale-factors
                                                                                                                    // only
                                                                                                                    // for
                                                                                                                    // calibrated
                                                                                                                    // WPs
        m_trkjet_btagEffTools[WP] = "BTaggingEfficiencyTool_" + WP + "_" + m_config->sgKeyTrackJets();
        top::check(m_trkjet_btagEffTools[WP].retrieve(), "Failed to retrieve b-tagging Efficiency tool");
        m_trkjet_systs[WP] = m_trkjet_btagEffTools[WP]->affectingSystematics();
        std::set<std::string> base_names = m_trkjet_systs[WP].getBaseNames();
        m_config->setBTaggingSFSysts(WP, base_names, true);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode BTagScaleFactorCalculator::execute() {
    top::check(apply(m_config->systSgKeyMapJets(false)),
               "Failed to apply btagging SFs");
    if (m_config->useTrackJets()) top::check(apply(m_config->systSgKeyMapTrackJets(), true),
                                             "Failed to apply track jet btagging SFs");

    return StatusCode::SUCCESS;
  }

  StatusCode BTagScaleFactorCalculator::apply(const std::shared_ptr<std::unordered_map<std::size_t,
                                                                                       std::string> >& jet_syst_collections,
                                              bool use_trackjets) {
    ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!!");
    ///-- Loop over all jet collections --///
    ///-- Lets assume that we're not doing ElectronInJet subtraction --///
    for (auto currentSystematic : *jet_syst_collections) {
      const xAOD::JetContainer* jets(nullptr);
      top::check(evtStore()->retrieve(jets, currentSystematic.second), "failed to retrieve jets");

      ///-- Tell the SF tools to use the nominal systematic --///
      /// -- Loop over all jets in each collection --///

      std::map<std::string,std::map<std::string,std::vector<uint>>> matchedtrigjets; //unused if no online tagger is requested
    ///-- If needed, prepare the trigger information --//
      if(m_onlineJets.size()!=0) {
	top::check(retrieveTriggerJets(),"Failed to retrieve trigger jets");
	for(auto& a_wp : m_onlineJets) {
	  matchedtrigjets[a_wp.first]=std::map<std::string,std::vector<uint>>();
	  for(auto& trig : a_wp.second) matchedtrigjets[a_wp.first][trig.first]=std::vector<uint>(0);
	}
      }

      for (auto jetPtr : *jets) {
        bool passSelection(false);
        if (jetPtr->isAvailable<char>("passPreORSelection")) {
          if (jetPtr->auxdataConst<char>("passPreORSelection") == 1) {
            passSelection = true;
          }
        }
        if (jetPtr->isAvailable<char>("passPreORSelectionLoose")) {
          if (jetPtr->auxdataConst<char>("passPreORSelectionLoose") == 1) {
            passSelection = true;
          }
        }

        if (passSelection) {
          // now loop over all available WPs

          for (auto& tagWP : (use_trackjets ? m_config->bTagWP_available_trkJet() : m_config->bTagWP_available())) {

	    ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 1 "+(TString)tagWP);
            // skip uncalibrated though available WPs
            if (use_trackjets &&
                std::find(m_config->bTagWP_calibrated_trkJet().begin(),
                          m_config->bTagWP_calibrated_trkJet().end(), tagWP)
                == m_config->bTagWP_calibrated_trkJet().end()) continue;
            else if (!use_trackjets &&
                     std::find(m_config->bTagWP_calibrated().begin(),
                               m_config->bTagWP_calibrated().end(), tagWP)
                     == m_config->bTagWP_calibrated().end()) continue;
            ToolHandle<IBTaggingEfficiencyTool>& btageff =
              use_trackjets ? m_trkjet_btagEffTools[tagWP] : m_btagEffTools[tagWP];
            ToolHandle<IBTaggingSelectionTool>& btagsel =
              use_trackjets ? m_trkjet_btagSelTools[tagWP] : m_btagSelTools[tagWP];
            CP::SystematicSet& sysSet = use_trackjets ? m_trkjet_systs[tagWP] : m_systs[tagWP];

            // need now the DSID to find out which shower was used in the sample
            unsigned int MapIndex = m_config->getMapIndex();

            btageff->setMapIndex("B", MapIndex);
	    btageff->setMapIndex("Light", MapIndex);
	    btageff->setMapIndex("C", MapIndex);
	    btageff->setMapIndex("T", MapIndex);
	    

            // Check if this jet collection systematic matches with one removed from the EV decomposition
            // (TopCorrectionsTools)
            std::string bTagSystName = top::bTagNamedSystCheck(m_config, currentSystematic.second, tagWP, use_trackjets, false);
            // If this string is not empty, we need to search and find the appropriate systematic set to apply
            if (bTagSystName != "") {
              CP::SystematicSet bTagSyst;
              bTagSyst.insert(sysSet.getSystematicByBaseName(bTagSystName));
              top::check(btageff->applySystematicVariation(bTagSyst),
                         "Failed to set new b-tagging SF to a shifted systematic set : " + bTagSystName);
            } else {
              top::check(btageff->applySystematicVariation(m_nominal),
                         "Failed to set new b-tagging SF to nominal");
            }

            float btag_SF(1.0);
            float btag_MCeff(-1.0);
            bool isTagged = false;//unused in case of Continuous
	    int flavour_label=-1;
	    bool jetMatchesTrigger=false;
	    float online_pt(-10), online_eta(-10), online_phi(-10), online_E(-10);
	    std::map<std::string,float> onlinemv2;
	    Analysis::CalibrationDataVariables vars_jet;
	      
	    if(tagWP.find("Online") == std::string::npos) {

	      if (std::fabs(jetPtr->eta()) <= 2.5) {

		top::check(btageff->getMCEfficiency(*jetPtr, btag_MCeff),
			   "Failed to get nominal b-tagging MC efficiency");
	
		if (tagWP.find("Continuous") == std::string::npos) {
		  isTagged = btagsel->accept(*jetPtr);
		  if (isTagged) top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
					   "Failed to get nominal b-tagging SF");
		  else top::check(btageff->getInefficiencyScaleFactor(*jetPtr, btag_SF),
				  "Failed to get nominal b-tagging SF");
		} else {
		  top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
			     "Failed to get nominal Continuous b-tagging SF");
		}
	      }
	      jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_nom") = btag_SF;
	      jetPtr->auxdecor<float>("btag_MCeff_" + tagWP + "_nom") = btag_MCeff;
	      
	    } else if(!use_trackjets) { //it is an online tagger (not available for track jets)
	      ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 2 "+(TString)tagWP);
		jetPtr->getAttribute("HadronConeExclTruthLabelID",flavour_label); // get the jet truth flavour
		vars_jet.jetPt = jetPtr->pt();
		vars_jet.jetEta = jetPtr->eta();

		onlinemv2.clear();

		for(auto& trig_item : m_onlineJets[tagWP.c_str()]) {
		  std::string TriggerChain = trig_item.first;

		  btag_SF = 1.0;
		  btag_MCeff = -1.0;
		  online_pt=-10;
		  online_eta=-10; 
		  online_phi=-10; 
		  online_E=-10;
		  ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 2.4 "+(TString)tagWP);
		  onlinemv2[TriggerChain.c_str()] = getOnlineWeight(jetPtr,tagWP,TriggerChain,matchedtrigjets[tagWP][TriggerChain],online_pt,online_eta,online_phi,online_E);
		  ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 2.4.5 "+(TString)tagWP);
		  jetMatchesTrigger = (onlinemv2[TriggerChain.c_str()]>-1.5);
		  isTagged = btagsel->accept(jetPtr->pt(),jetPtr->eta(),onlinemv2[TriggerChain.c_str()]);
		  ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 2.5 "+(TString)tagWP);
		  vars_jet.jetTagWeight = onlinemv2[TriggerChain.c_str()];
		  
		  if(std::fabs(jetPtr->eta()) <= 2.5 && abs(flavour_label)==5 && jetPtr->pt()>35000 && jetMatchesTrigger) { 
		    top::check(btageff->getScaleFactor(flavour_label, vars_jet, btag_SF), //is flavour_label the correct thing?
			       "Failed to get nominal ONLINE b-tagging SF for trigger " + TriggerChain);
		    top::check(btageff->getMCEfficiency(flavour_label, vars_jet, btag_MCeff), //is flavour_label the correct thing? 
			       "Failed to get nominal ONLINE b-tagging MC efficiency for trigger " + TriggerChain);
		  }
		  		  
		  jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_" + TriggerChain + "_nom") = btag_SF;
		  jetPtr->auxdecor<float>("btag_MCeff_" + tagWP + "_" + TriggerChain + "_nom") = btag_MCeff;
		  
		  jetPtr->auxdecor<char>("trigMatch_isbtagged_" + tagWP + "_nom") = isTagged;
		  jetPtr->auxdecor<float>("trigMatch_taggerWeight_" + tagWP + "_nom") = onlinemv2[TriggerChain.c_str()];
		  jetPtr->auxdecor<float>("trigMatch_pt_" + tagWP + "_nom") =  online_pt;
		  jetPtr->auxdecor<float>("trigMatch_eta_" + tagWP + "_nom") =  online_eta;
		  jetPtr->auxdecor<float>("trigMatch_phi_" + tagWP + "_nom") =  online_phi;
		  jetPtr->auxdecor<float>("trigMatch_e_" + tagWP + "_nom") =  online_E;

		}

	      ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY!! 3 "+(TString)tagWP);
	    }


            ///-- For nominal calibration, vary the SF systematics --///
            if (currentSystematic.first == m_config->nominalHashValue()) {
              for (const auto& variation : sysSet) {
                btag_SF = 1.;
		btag_MCeff = -1.;
                CP::SystematicSet syst_set;
                syst_set.insert(variation);
                top::check(btageff->applySystematicVariation(syst_set),
                           "Failed to set new b-tagging systematic variation " + syst_set.name());

		if(tagWP.find("Online") == std::string::npos) {
		  if (std::fabs(jetPtr->eta()) <= 2.5) {

		    top::check(btageff->getMCEfficiency(*jetPtr, btag_MCeff),
			       "Failed to get b-tagging MC efficiency for variation " + syst_set.name());
		    
		    if (tagWP.find("Continuous") == std::string::npos) {
		      if (isTagged) top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
					       "Failed to get b-tagging SF for variation " + syst_set.name());
		      else top::check(btageff->getInefficiencyScaleFactor(*jetPtr, btag_SF),
				      "Failed to get b-tagging SF for variation " + syst_set.name());
		    } else {
		      top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
				 "Failed to get Continuous b-tagging SF for variation " + syst_set.name());
		    }
		  }
		  jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_" + variation.name()) = btag_SF;
		  jetPtr->auxdecor<float>("btag_MCeff_" + tagWP + "_" + variation.name()) = btag_MCeff;

		} else if(!use_trackjets) { //it is an online tagger (not available for track jets)         
		  for(auto& trig_item : m_onlineJets[tagWP.c_str()]) {
		    std::string TriggerChain = trig_item.first;

		    btag_SF = 1.;
		    btag_MCeff = -1.;
		    jetMatchesTrigger = (onlinemv2[TriggerChain.c_str()]>-1.5);
		    
		    if(std::fabs(jetPtr->eta()) <= 2.5 && abs(flavour_label)==5 && jetPtr->pt()>35000 && jetMatchesTrigger) { 
		      vars_jet.jetTagWeight = onlinemv2[TriggerChain.c_str()];
		      top::check(btageff->getScaleFactor(flavour_label, vars_jet, btag_SF),//is flavour_label the correct thing? 
				 "Failed to get ONLINE b-tagging SF for variation " + syst_set.name() + " and trigger " + TriggerChain);
		      top::check(btageff->getMCEfficiency(flavour_label, vars_jet, btag_MCeff),//is flavour_label the correct thing? 
				 "Failed to get ONLINE b-tagging MC efficiency for variation " + syst_set.name() + " and trigger " + TriggerChain);
		    }
		    jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_" + TriggerChain + "_" + variation.name()) = btag_SF;
		    jetPtr->auxdecor<float>("btag_MCeff_" + tagWP + "_" + TriggerChain + "_" + variation.name()) = btag_MCeff;
		  }
		}	
	
	      } // loop through b-tagging systematic variations
	    } // Calibration systematic is nominal, so calculate SF systematics
          }
        }
      }
    }
       
    ATH_MSG_INFO("BTagScaleFactorCalculator::TMP APPLY END");
    return StatusCode::SUCCESS;
  }

  //helper function for trigger navigation, from https://twiki.cern.ch/twiki/bin/view/Atlas/TrigBjetCalibration2016Rel21#Retrieving_online_btagging
  template<class Object, class Collection>
  const Object* getTrigObject(Trig::Feature<Collection>& feature){

    const Collection* trigCol = feature.cptr();
    if ( !trigCol ) {
      std::cout << "ERROR: No Trig Collection pointer" << std::endl;
      return 0;
    }
    if(trigCol->size() != 1){
      std::cout << "ERROR Trig Collection size " << trigCol->size() << std::endl;
      return 0;
    }
    return trigCol->at(0);
  }


  StatusCode BTagScaleFactorCalculator::retrieveTriggerJets() {

    for(auto& a_wp : m_onlineJets) {
      m_onlineBtagging[a_wp.first.c_str()]=std::map<std::string,std::vector<float>>();

      for(auto& trig_item : a_wp.second) {
	std::string TriggerChain = trig_item.first;
	// retrive online jets and btagging (from https://twiki.cern.ch/twiki/bin/view/Atlas/TrigBjetCalibration2016Rel21#Retrieving_online_btagging )

	m_onlineBtagging[a_wp.first][TriggerChain]=std::vector<float>(0);
	Trig::FeatureContainer fc = m_trigDecisionTool->features(TriggerChain);
	Trig::FeatureContainer::combination_const_iterator comb   (fc.getCombinations().begin());
	Trig::FeatureContainer::combination_const_iterator combEnd(fc.getCombinations().end());
	for( ; comb!=combEnd ; ++comb) {
	  std::vector< Trig::Feature<xAOD::JetContainer> >  jetCollections  = comb->containerFeature<xAOD::JetContainer>("SplitJet");
	  std::vector< Trig::Feature<xAOD::BTaggingContainer> > bjetCollections = comb->containerFeature<xAOD::BTaggingContainer>("HLTBjetFex");
	  for ( unsigned ifeat=0 ; ifeat<jetCollections.size() ; ifeat++ ) {
	    const xAOD::Jet* hlt_jet = getTrigObject<xAOD::Jet, xAOD::JetContainer>(jetCollections.at(ifeat));
	    // online jet already in?       
	    bool notsaved = true;
	    TLorentzVector vtemp;
	    vtemp.SetPtEtaPhiM(hlt_jet->pt(), hlt_jet->eta(), hlt_jet->phi(), hlt_jet->m());
	    for (auto vsaved : m_onlineJets[a_wp.first][TriggerChain]) if (vsaved.DeltaR(vtemp) == 0) notsaved = false;
	    // if not, add it
	    if (notsaved) {
	      m_onlineJets[a_wp.first][TriggerChain].push_back(vtemp);
	      const xAOD::BTagging* hlt_btag = getTrigObject<xAOD::BTagging, xAOD::BTaggingContainer>(bjetCollections.at(ifeat));
	      double MV2_mvx;
	      std::string onlineTagger = (TriggerChain.find("bmv2c20") != std::string::npos) ? "MV2c20" : "MV2c10";
	      hlt_btag->MVx_discriminant(onlineTagger ,MV2_mvx);    // MV2c20 used for 2016 
	      m_onlineBtagging[a_wp.first][TriggerChain].push_back(MV2_mvx);
	    }
	  }
	}
      }
    }
    if(m_onlineJets.size()==0) return StatusCode::FAILURE;
    return StatusCode::SUCCESS;
  }

  float BTagScaleFactorCalculator::getOnlineWeight(const xAOD::Jet *jet,std::string wp,std::string chain, std::vector<uint> & matchedtrigjets,float & online_pt,float & online_eta,float & online_phi,float & online_E) const
  {
    ATH_MSG_INFO(Form("BTagScaleFactorCalculator::TMP getOnlineWeight - btagsize=%d, jetsize=%d",(int)(m_onlineBtagging.at(wp).at(chain).size()),(int)(m_onlineJets.at(wp).at(chain).size())));

    float jet_onlinemv2=-2;
    // geometrical matching between online and offline jets (from https://twiki.cern.ch/twiki/bin/view/Atlas/TrigBjetCalibration2016Rel21#Retrieving_online_btagging )

    TLorentzVector vjet;
    vjet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());

    int found_trigjet=-1;
    for (uint itrigjet = 0; itrigjet<m_onlineJets.at(wp).at(chain).size(); itrigjet++) {

      bool isalreadymatched = false;
      for (uint imtj : matchedtrigjets) {
	if (itrigjet == imtj) {
	  isalreadymatched = true;
	  break;
	}
      }
      ATH_MSG_INFO(Form("BTagScaleFactorCalculator::TMP getOnlineWeight 1-%d",(int)itrigjet));
      if (isalreadymatched) continue;
      if (vjet.DeltaR(m_onlineJets.at(wp).at(chain).at(itrigjet)) < 0.2) {
	ATH_MSG_INFO(Form("BTagScaleFactorCalculator::TMP getOnlineWeight 1.5-%d, %d, %d",(int)itrigjet,m_onlineBtagging.at(wp).size(),m_onlineBtagging.at(wp).at(chain).size()));
	jet_onlinemv2=m_onlineBtagging.at(wp).at(chain).at(itrigjet);
	  matchedtrigjets.push_back(itrigjet);
	  found_trigjet=itrigjet;
	  break;
      }
    }
    ATH_MSG_INFO("BTagScaleFactorCalculator::TMP getOnlineWeight 2");
    if(found_trigjet>=0) {
      TLorentzVector vtmp = m_onlineJets.at(wp).at(chain).at(found_trigjet);
      online_pt=vtmp.Pt();
      online_eta=vtmp.Eta();
      online_phi=vtmp.Phi();
      online_E=vtmp.E();
    }
    ATH_MSG_INFO("BTagScaleFactorCalculator::TMP getOnlineWeight END");
    return jet_onlinemv2;
  }

  StatusCode BTagScaleFactorCalculator::debug() {
    ATH_MSG_INFO("BTagScaleFactorCalculator::debug function");
    // Use after package is initialised otherwise vectors will be empty
    for (auto& tagWP :  m_config->bTagWP_available()) {
      ATH_MSG_INFO("Tagger working point : " << tagWP);
      ToolHandle<IBTaggingEfficiencyTool>& btageff = m_btagEffTools[tagWP];
      // Retrieve tool
      top::check(btageff.retrieve(), "Failed to retrieve tool");
      // Retrieve list of systematics included
      CP::SystematicSet systs = btageff->affectingSystematics();
      CP::SystematicSet recsysts = btageff->recommendedSystematics();

      ATH_MSG_INFO("-----------------------------------------------------------------------");

      const std::map<CP::SystematicVariation,
                     std::vector<std::string> > allowed_variations = btageff->listSystematics();

      ATH_MSG_INFO("Allowed systematics variations for tool " << btageff->name());
      for (auto var : allowed_variations) {
        std::string flvs = "";
        for (auto flv : var.second) flvs += flv;
        ATH_MSG_INFO(" (" << flvs << ") - " << var.first);
      }

      // Now use the new functions added into xAODBTaggingEfficiency-00-00-39
      // std::map<std::string, std::vector<std::string> >
      ATH_MSG_INFO("-----------------------------------------------------------------------");
      ATH_MSG_INFO("List of b-tagging scale factor systematics");
      std::map<std::string,
               std::vector<std::string> >  listOfScaleFactorSystematics = btageff->listScaleFactorSystematics(false);
      for (auto var : listOfScaleFactorSystematics) {
        ATH_MSG_INFO("Jet flavour : " << var.first);
        std::vector<std::string> systs = var.second;
        std::sort(systs.begin(), systs.end());
        for (auto sys : systs) {
          ATH_MSG_INFO(" (" << var.first << ") - " << sys);
        }
        ATH_MSG_INFO(" ");
      }

      ATH_MSG_INFO("List of (named) b-tagging scale factor systematics");
      listOfScaleFactorSystematics = btageff->listScaleFactorSystematics(true);
      for (auto var : listOfScaleFactorSystematics) {
        ATH_MSG_INFO("Jet flavour : " << var.first);
        std::vector<std::string> systs = var.second;
        std::sort(systs.begin(), systs.end());
        for (auto sys : systs) {
          ATH_MSG_INFO(" (" << var.first << ") - " << sys);
        }
        ATH_MSG_INFO(" ");
      }
      ATH_MSG_INFO("-----------------------------------------------------------------------");
    }
    return StatusCode::SUCCESS;
  }
}
