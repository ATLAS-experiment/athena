/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************
 * AsgTool: TrigEgammaNavTPBaseTool
 * Authors:
 *      Joao Victor Pinto <jodafons@cern.ch>
 *      Edmar de Souza <edmar.egidio@cern.ch>
 * Description:
 *      Trigger e/gamma Zee Tag&Probe Base tool class. Inherits from TrigEgammaAnalysisBaseTool.
 *      Provides methods for selecting T&P pairs, 
 *      matching probes to objects in TE containers.
 *      Creates a vector of pairs with 
 *      offline electrons probes and the last TE with a match.
 *      Relies heavily on TrigNavigation, since we use the TriggerDecision.
 *      All derived classes work with list of probes for a given trigger.
 *      As input property, pass a list of triggers to study.
 **********************************************************************/

#include "TrigEgammaMonitorTagAndProbeAlgorithmZeeg.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "string"
#include <algorithm>
#include <typeinfo>
#include "TrigSteeringEvent/Chain.h"
#include "TrigDecisionTool/ChainGroup.h"
#include "TrigDecisionInterface/Conditions.h"

#include <type_traits>
#include <string>

#include "LArRecEvent/LArEventBitInfo.h"
#include "StoreGate/ReadHandle.h"

//**********************************************************************
using namespace Trig;
using namespace xAOD;
#include <vector>
#include <string>
#include <iostream>


TrigEgammaMonitorTagAndProbeAlgorithmZeeg::TrigEgammaMonitorTagAndProbeAlgorithmZeeg( const std::string& name, ISvcLocator* pSvcLocator ):
  TrigEgammaMonitorAnalysisAlgorithm( name, pSvcLocator )
{

}


TrigEgammaMonitorTagAndProbeAlgorithmZeeg::~TrigEgammaMonitorTagAndProbeAlgorithmZeeg()
{}




StatusCode TrigEgammaMonitorTagAndProbeAlgorithmZeeg::initialize() {
   
    ATH_MSG_INFO("TrigEgammaMonitorTagAndProbeAlgorithmZeeg::initialize()...");
    ATH_CHECK(TrigEgammaMonitorAnalysisAlgorithm::initialize());

    ATH_CHECK(m_offElectronKey.initialize());
    ATH_CHECK(m_offPhotonIsolationKeys.initialize());
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_eventInfoDecorKey.initialize() );
    ATH_CHECK(m_electronIsolationKeyPtCone20.initialize());
    ATH_CHECK(m_photonsKey.initialize());    
    ATH_CHECK( m_hltPhotonsKey.initialize() );


    ATH_MSG_INFO("Now configuring (probe) trgger chains for analysis: " << name() );
    for(auto& trigName : m_trigInputList)
    {
        ATH_MSG_INFO("Trigger booked: " << trigName);
      if(getTrigInfoMapR3().count(trigName) != 0){
        ATH_MSG_INFO("Trigger already booked, removing from trigger list " << trigName);
      }else {
        m_trigList.push_back(trigName);
        setTrigInfoR3(trigName);
      }
    }


    ATH_MSG_INFO("Now configuring (tag) trgger chains for analysis: " << name() );
    for(auto& tagName : m_tagTrigList)
    {
        ATH_MSG_INFO("Tag Trigger booked: " << tagName);
      if(getTrigInfoMapR3().count(tagName) != 0){
        ATH_MSG_INFO("TagTrigger already booked, removing from trigger list " << tagName);
      }else {
        m_tagList.push_back(tagName);
        setTrigInfo(tagName);
      }
    }


    return StatusCode::SUCCESS;


}



StatusCode TrigEgammaMonitorTagAndProbeAlgorithmZeeg::fillHistograms( const EventContext& ctx ) const {
  

    //Read in the photon container
    SG::ReadHandle<xAOD::PhotonContainer> probes2(m_photonsKey, ctx); // photon container 

    //Read in HLT_Photon container
    SG::ReadHandle<xAOD::PhotonContainer> hltPhotons(m_hltPhotonsKey, ctx);

    if (!hltPhotons.isValid()) {
        ATH_MSG_DEBUG("HLT_egamma_Photons container not found");
        return StatusCode::SUCCESS;
    }
    
    if (hltPhotons->empty()) {
        ATH_MSG_DEBUG("HLT_egamma_Photons is empty");
        return StatusCode::SUCCESS;
    }
    

    //Matching function requires a shared vector
    //Therefore we need to push the photons inside a vector
    std::vector<std::shared_ptr<const xAOD::Photon>> probesTest;


    //Reserve space for the number of photons
    probesTest.reserve(probes2->size());
    
    //Push the photons inside the vector
    //for (const xAOD::Photon* photon2 : *probes2) {
    //    probesTest.emplace_back(photon2, [](const xAOD::Photon*) {});
    //}

    //The execute function seems to want this empty vector
    //This is empty initial because we write the code to select the probes
    //Then we will this, if it is empty then we did not find any good
    //photon probes
    std::vector<std::shared_ptr<const xAOD::Photon>> probes;
    //probes.reserve(probes2->size());
    std::vector<std::pair<TLorentzVector, TLorentzVector>> tagPairs;





    // Select TP Pairs
    ATH_MSG_INFO("Execute TP Zeeg selection");
    if( !executeTandP(ctx, probes, tagPairs) ){
        ATH_MSG_INFO("Tag and Probe Zeeg event failed.");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("SIZE OF THE NOW FILLED PROBES");
    ATH_MSG_DEBUG("SIZE: " << probes.size());

    // Check HLTResult
    if(isHLTTruncated()){
        ATH_MSG_INFO("HLTResult truncated, skip trigger analysis");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Made it pass HLT");

    // Noise burst protection 
    SG::ReadHandle<xAOD::EventInfo> thisEvent(GetEventInfo(ctx));
    ATH_CHECK(thisEvent.isValid());
    if ( thisEvent->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO)) {
        ATH_MSG_INFO("LAr Noise Burst Veto, skip trigger analysis");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Made it pass LAR");
     //To keep track of the number of events that pass the trigger (includes both tag and probe)
     //We needed to add in the monGroup
     auto monGroup = getGroup( m_anatype );

     //Looping over the tag triggers
     for(const std::string& tagTrigger : m_tagTrigList){
	// Check that we have probe photons
    	if (probes.size() == 0) continue;

	if (tdt()->isPassed(tagTrigger)){
		ATH_MSG_DEBUG("Starting loop:Passed Tag Trigger: " << tagTrigger);
    		ATH_MSG_DEBUG("HLT_egamma_Photons size = " << hltPhotons->size());
		
		//Find the passed trigger in the map located in python/TrigEgammaMonitCategory.py
		auto it = m_TPMatchingMap.find(tagTrigger);

		//Check if the tag trigger could be found
		if (it == m_TPMatchingMap.end()) {
		    ATH_MSG_DEBUG("No TP mapping found for tag: " << tagTrigger);
		    ATH_MSG_DEBUG("Please include the tag trigger in the  ");
		    ATH_MSG_DEBUG("correct dictionary in python/TrigEgammaMonitCategory.py  ");
		    ATH_MSG_DEBUG("THIS IS A VERY BAD AND FATAL ERROR");
		}

		//Get the probe triggers associated with the correct tag
		const std::vector<std::string>& probeTriggers = it->second;

		//Loop over the probe triggers
     		for (const std::string& probeTrigger : probeTriggers) {
     		   ATH_MSG_DEBUG("Probe Trigger: " <<probeTrigger);

		   //Quickly checking if the probe trigger passes the trigger decision
		   //This is soley used for the egamma monitoring system
		   //The true trigger matching will be done in the matchObjects function
     		   if (tdt()->isPassed(probeTrigger)){
     		        ATH_MSG_DEBUG("Passed full trigger  == " << probeTrigger);
     		 	fillLabel(monGroup, "CutCounter", "PassFullTrigger");
		    }

     		   	/** Pair objects used in executeTool **/
     		   	std::vector<std::pair<const xAOD::Egamma*, const TrigCompositeUtils::Decision*>> pairObjs;

			//Getting information about the trigger
     		   	const TrigInfo info = getTrigInfoR3(probeTrigger);

			//Redfining the probeTrigger to trigName
			//just to keep consistency of other code 
     		   	const std::string& trigName=probeTrigger;

     		   	ATH_MSG_DEBUG("Start Chain Analysis ============================= Passed Full probe Trigger: " << trigName);
     		   	
     		   	ATH_MSG_DEBUG("Trigger " << trigName << " pidword " << info.pidname << " threshold " << info.etthr);

     		   	matchObjects(trigName,probes , pairObjs);
			//Matching the offline photon to the online
			//This is not a true deltaR matching 
			//We are doing the deltaR then asking which object is the closet
			//This causes double events to appear in the monitoring code but does
			//not impact the efficiency or scale factors
			//

			int nMatched = 0;
			for (const auto& p : pairObjs) {
			    if (p.second != nullptr) nMatched++;
			}


     		   	// Just for counting
     		   	ATH_MSG_DEBUG("Number of photons probes: " << probes.size());
			ATH_MSG_DEBUG("Number of trigger matched photons: " << nMatched);

     		   	// Include fill here
     		   	fillDistributions(ctx, pairObjs, info );
     		   	fillEfficiencies(ctx, pairObjs, info, false);
     		   	fillResolutions(ctx, pairObjs,  info );

     		  // }// if pass probe trigger

     		} // End loop over probe list
     	} // if passed trigger list
     } //End loop over tag list

    return StatusCode::SUCCESS;
}




bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::executeTandP( const EventContext& ctx, std::vector<std::shared_ptr<const xAOD::Photon>> &probePhotons, std::vector<std::pair<TLorentzVector, TLorentzVector>> &tagPairs) const
{

    //Get the monitoring group for each trigger (m_anatype)
    auto monGroup = getGroup( m_anatype );
    
    //Starting number of events for the trigger
    fillLabel(monGroup, "CutCounter", "Events");

    SG::ReadHandle<xAOD::EventInfo> eventInfo = GetEventInfo (ctx);
    if( !eventInfo.isValid() ){
      ATH_MSG_INFO("Failed to retrieve EventInfo");
      return false;
    }


    //Check if trigger passes LAR requirement
    if (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error) {
        ATH_MSG_INFO("Event not passing LAr");
        return false;
    }

    fillLabel(monGroup, "CutCounter", "LAr");
    SG::ReadHandle<xAOD::ElectronContainer> offElectrons(m_offElectronKey, ctx);

    //Check if the offline container is valid
    if(!offElectrons.isValid())
    {
      ATH_MSG_INFO("Failed to retrieve offline Electrons ");
	    return false;
    }

    fillLabel(monGroup, "CutCounter", "RetrieveElectrons");

    ATH_MSG_INFO( "Electron size is " << offElectrons->size() );

    // Check Size of Electron Container
    // Ensure there are at least two electron for tag and probe
    if ( offElectrons->size() < 2 ) { // Not enough events for T&P
	    ATH_MSG_INFO("Not enough Electrons for T&P");
	    return false;
    }
    
    fillLabel(monGroup, "CutCounter", "TwoElectrons");

    //Get the photon container
    SG::ReadHandle<xAOD::PhotonContainer> photons(m_photonsKey, ctx);
    if(!photons.isValid())
    {
        ATH_MSG_INFO("Failed to retrieve Photons ");
        return false;
    }
    fillLabel(monGroup, "CutCounter", "RetrievePhotons");

    ATH_MSG_INFO( "Photon size is " << photons->size() );

    // Check size of Photon container
    // Make sure there is at least one photon
    if ( photons->size() < 1 ) { // Not enough events for T&P
           ATH_MSG_INFO("Not enough Photons for T&P");
           return false;
    }


    //Get the jet container
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
    if(!jets.isValid() && m_applyJetNearProbeSelection){
      ATH_MSG_INFO("Failed to retrieve JetContainer");
      return false;
    }

    ATH_MSG_INFO( "Jet size is " << jets->size());
    ATH_MSG_INFO( "Minimal trig list size is " << m_tagTrigList.size());

    //Check if the tag trigger passes
    //We know by this point that at least to electrons must be present
    if(!m_tagTrigList.empty()){
      if(m_applyMinimalTrigger){ 
        if ( !minimalTriggerRequirement() ){
    	  ATH_MSG_INFO( "Test Failed the minimal Trigger Requirement");
          return false;
	}
      }
      fillLabel(monGroup, "CutCounter", "PassMinimalTrigger");
 
    }else{
      ATH_MSG_DEBUG("Disable trigger tags because trigger tags list is empty.");
    }


    //We are now entering the function where we select our tag electrons
    ATH_MSG_INFO("Execute TandP BaseTool " << offElectrons->size());
    for(const auto *const elTag1 : *offElectrons)
    {
        if( !isGoodElectron( monGroup, elTag1) ) continue;
        ATH_MSG_INFO("Electron 1 is good"); 

        for(const auto *const elTag2 : *offElectrons)
        {  // Dress the probes with updated Pid decision

            if(elTag2==elTag1) continue;
            ATH_MSG_INFO("Electron 2 is not equal to electron 1"); 

            // Tag selection is limited to one tag per object type but i dont know what that means really
            if(!isGoodElectron(monGroup, elTag2)) continue;
       	    ATH_MSG_INFO("Electron 2 is good"); 

            fillLabel(monGroup, "TagCutCounter", "Electrons");
            // Check opposite charge
            if(m_oppositeCharge && (elTag2->charge() == elTag1->charge()) ) continue;
            ATH_MSG_INFO("Electron 2 charge == Electron 1 charge"); 

            fillLabel(monGroup, "TagCutCounter", "OS");
            if(!m_oppositeCharge && (elTag2->charge() != elTag1->charge()) ) continue;
            ATH_MSG_INFO("Electron 2 charge != Electron 1 charge"); 

            fillLabel(monGroup, "TagCutCounter", "SS");

	    //Di-electron trigger matching
	    if (!matchDiElectronTrigger(elTag1, elTag2)) continue;

	    //Building the mass of the electrons
            TLorentzVector el1;
            TLorentzVector el2;
            el1.SetPtEtaPhiE(elTag1->pt(), elTag1->trackParticle()->eta(), elTag1->trackParticle()->phi(), elTag1->e());
            el2.SetPtEtaPhiE(elTag2->pt(), elTag2->trackParticle()->eta(), elTag2->trackParticle()->phi(), elTag2->e());


	    // Quick check for electron within range
            float mee = (el1 + el2).M();
            if (mee < 50e3 || mee > 120e3) continue;

	    //Save all potential tag electron canidates
	    tagPairs.emplace_back(el1, el2);

    	} // End of for loop for second electron
     } //End of for loop for first electron

    //Now we have a vector of tag combinations 
     //selecting the photon and tag and probe
     ATH_MSG_INFO("Entering photonZeeg loop"); 
     for (const auto *const photProbe : *photons) {
         
	 if(!isGoodProbePhoton(monGroup, photProbe, jets.cptr())) continue;
         // Probe available. Good Probe?
         ATH_MSG_DEBUG("is good probe Photon");
         fillLabel( monGroup, "ProbeCutCounter", "GoodProbe");

         //Must be an easy way with IParticle
         TLorentzVector ph;
         ph.SetPtEtaPhiE(photProbe->pt(), photProbe->eta(), photProbe->phi(), photProbe->e());


	 for (const auto& pair : tagPairs) {

         const TLorentzVector& v1 = pair.first;
         const TLorentzVector& v2 = pair.second;

	 float tpPairMass = (v1 + v2).M();
         float eegMass    = (v1 + v2 + ph).M();
         ATH_MSG_DEBUG("tpPairMass" << tpPairMass); 
     	 ATH_MSG_DEBUG("eegMass" << eegMass); 

         // we want eeg mass to be on z and the ee mass to be < some number
         if( !((eegMass > 80.0*1.e3) && (eegMass < 100.0*1.e3))){
             ATH_MSG_DEBUG("eeg not in Z mass window [80 - 100]");
             continue;
         }
	 else {
             fillLabel(monGroup, "ProbeCutCounter", "ZMass");

             auto selProbe = std::make_shared<const xAOD::Photon>(*photProbe);
             probePhotons.emplace_back(std::move(selProbe));

             auto mon_count_probe= Monitored::Scalar<std::string>("ProbeCutCounter","GoodProbe");
             auto mon_meeg = Monitored::Scalar<float>("Meeg" , eegMass/1.e3 );
             fill( monGroup , mon_count_probe, mon_meeg );
             ATH_MSG_INFO("Fill TP Meeg and count");
         }

     } // end of tagpairs for loop
   } // End of photon probe loop
    ATH_MSG_DEBUG( "Number of probes found is " << probePhotons.size() );
    return true;
} // End of executeTandP


bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::minimalTriggerRequirement() const {
    
    	ATH_MSG_DEBUG("Apply Minimal trigger requirements (New method)");
        for (const std::string& s : m_tagTrigList) {

        if (tdt()->isPassed(s)){
		ATH_MSG_DEBUG("Passed minimal trigger requirement for trigger: "<< s);
		return true;
    }

}

    ATH_MSG_DEBUG("Minimal trigger not passed");
    return false;

}



void TrigEgammaMonitorTagAndProbeAlgorithmZeeg::matchObjects(const std::string& trigger_chain,
                                           std::vector<std::shared_ptr<const xAOD::Photon>>& probePhotons,
                                           std::vector<std::pair<const xAOD::Egamma*, const TrigCompositeUtils::Decision *>> &pairObj) const
{

  //Looping over the offline photons
  for (const auto& offline_photon : probePhotons)
  {
    const TrigCompositeUtils::Decision *dec = nullptr;

    ATH_MSG_DEBUG("Applying matching function");
    bool matched = match()->match(offline_photon.get(), trigger_chain, dec, TrigDefs::includeFailedDecisions);
    if (matched) {
        pairObj.emplace_back(offline_photon.get(), dec);
    } else {
        pairObj.emplace_back(offline_photon.get(), nullptr);
    }
  } 
   
}// end of match objects function

//For the Zeeg tag and probe, you cannot do trigger matching
//Separately for each electron. It has to be done as a pair
//The next two function checks if the electron is good
//Then checks if both passes the trigger
bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::isGoodElectron(const ToolHandle<GenericMonitoringTool>& monGroup,const xAOD::Electron *el) const
{
    fillLabel(monGroup, "TagCutCounter", "Electrons");

    ATH_MSG_INFO("Selecting good electron");

    const xAOD::TrackParticle *trk = el->trackParticle();
    if (!trk) {
        ATH_MSG_INFO("No track particle");
        return false;
    }
    fillLabel(monGroup, "TagCutCounter", "HasTrack");

    const xAOD::CaloCluster *clus = el->caloCluster();
    if (!clus) {
        ATH_MSG_INFO("No calo cluster");
        return false;
    }
    fillLabel(monGroup, "TagCutCounter", "HasCluster");

    ATH_MSG_INFO("Electron pt: " << el->pt());

    //    Gaudi::Property<float> m_tagMinEt{ this, "OfflineTagMinEt", 25};

    if (el->pt() < m_tagMinEt * Gaudi::Units::GeV) {
        ATH_MSG_INFO("Failed pt cut");
        return false;
    }
    fillLabel(monGroup, "TagCutCounter", "Et");

    // Checking Eta acceptance
    float absEta = std::fabs(clus->etaBE(2));
    if ((absEta > 1.37 && absEta < 1.52) || absEta > 2.47) {
        ATH_MSG_INFO("Failed eta acceptance");
        return false;
    }
    fillLabel(monGroup, "TagCutCounter", "Eta");

    // Checking Object quality
    if (!el->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON)) {
        ATH_MSG_INFO("Failed OQ");
        return false;
    }
    fillLabel(monGroup, "TagCutCounter", "IsGoodOQ");

    // Passed Good Electron selection
    ATH_MSG_INFO("Passing electron selection");

    return true;
}

bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::matchDiElectronTrigger(const xAOD::Electron* e1, const xAOD::Electron* e2) const
{
    // Loop over di-electron triggers (probe not included)
    for (const std::string& trig : m_tagTrigList) {

        if (!tdt()->isPassed(trig)) continue;

        auto features = tdt()->features<xAOD::ElectronContainer>(trig, TrigDefs::Physics);

	// Do we have two electrons that pass
	// Start with false
     	bool e1Match = false;
        bool e2Match = false;


	// Match Between offline and online electrons
        for (const auto& feat : features) {
            if (!feat.isValid()) continue;

            const xAOD::Electron* hlt_el = *feat.link;
            if (!hlt_el) continue;

            if (hlt_el->p4().DeltaR(e1->p4()) < 0.1)  e1Match = true;

            if (hlt_el->p4().DeltaR(e2->p4()) < 0.1)  e2Match = true;

	    //If we find a match exit
	    if (e1Match && e2Match) return true;

        } // End of offline online matching

        } //End of tag trigger

    //Did not find tag electrons
    return false;
} // End of matchDiElectronTrigger




bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::isTagElectron(const EventContext& ctx,
                                                              const ToolHandle<GenericMonitoringTool>& monGroup,
                                                              const xAOD::Electron *el) const
{
    fillLabel(monGroup, "TagCutCounter", "Electrons");

    // Tag the event
    ATH_MSG_INFO("Selecting Tag Electron");

    //Check constituents
    const xAOD::TrackParticle *trk = el->trackParticle();
    if(!el->trackParticle()){
        ATH_MSG_INFO("No track Particle");
        return false;
    }

    fillLabel(monGroup, "TagCutCounter", "HasTrack");

    ATH_MSG_INFO("Track pt " << trk->pt());
    const xAOD::CaloCluster *clus = el->caloCluster();
    if(!el->caloCluster()){
        ATH_MSG_INFO("No caloCluster");
        return false;
    }

    fillLabel(monGroup, "TagCutCounter", "HasCluster");

    ATH_MSG_INFO("Cluster E "<<clus->e());
    ATH_MSG_INFO("Selecting Tag Electron PID");
    fillLabel(monGroup, "TagCutCounter", "GoodPid");

    ATH_MSG_INFO("Selecting Tag Electron Et");
    //Require Et > 25 GeV
    if( !(el->e()/cosh(el->trackParticle()->eta())  > m_tagMinEt*Gaudi::Units::GeV) ){
        return false;
    }
    
    fillLabel(monGroup, "TagCutCounter", "Et");

    ATH_MSG_INFO("Selecting Tag Electron Eta");
    //fiducial detector acceptance region
    float absEta = fabs(el->caloCluster()->etaBE(2));
    if ((absEta > 1.37 && absEta < 1.52) || absEta > 2.47) {
        return false;
    }

    fillLabel(monGroup, "TagCutCounter", "Eta");

    ATH_MSG_INFO("Checking electron object quality");
    if (!el->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON)) return false;
    
    fillLabel(monGroup, "TagCutCounter", "IsGoodOQ");

    if(m_tagTrigList.empty())
    {
      ATH_MSG_INFO("Found a tag electron"); 
      return true;
    }

    ATH_MSG_INFO("Selecting Tag Electron Decision");
    // Check matching to a given trigger
    // The statement below is more general
    bool tagPassed=false;
//    for( auto& tag : m_tagList){
//      if(tdt()->isPassed(tag)){ 
//        tagPassed=true;
//        break;
//      }
//    }
        for (const std::string& tag : m_tagTrigList) {

        if (tdt()->isPassed(tag)){
        	auto hlt_features = tdt()->features<xAOD::ElectronContainer>(tag, TrigDefs::Physics);
        	for (auto const& feat : hlt_features) {
        	    if (!feat.isValid()) continue;

        	    const xAOD::Electron* hlt_el = *feat.link; 
        	    if (!hlt_el) continue;

        	    if (hlt_el->p4().DeltaR(el->p4()) < 0.1) {
        	        tagPassed=true;
        	        break; 
			}
		}
	}


    }

    if(!tagPassed) {
        ATH_MSG_INFO("Failed tag trigger "); 
        return false;
    }
    
    fillLabel(monGroup, "TagCutCounter", "PassTrigger");

    ATH_MSG_INFO("Matching Tag Electron FC");
    bool tagMatched=false;
    for (const std::string& tag : m_tagTrigList) {
        if (match()->isPassed(ctx, el,tag)){
            	tagMatched=true;
        }

    }

    if(!tagMatched){
        ATH_MSG_INFO("Failed a match ");
        return false; // otherwise, someone matched!
    }
    
    fillLabel(monGroup, "TagCutCounter", "MatchTrigger");

    ATH_MSG_INFO("Found a tag electron");
    return true;

}

bool TrigEgammaMonitorTagAndProbeAlgorithmZeeg::isGoodProbePhoton(const ToolHandle<GenericMonitoringTool>& monGroup, const xAOD::Photon *phot, const xAOD::JetContainer *jets) const
{
    fillLabel(monGroup, "ProbeCutCounter", "Photons");

    const xAOD::CaloCluster *clus = phot->caloCluster();
    if(!clus){
        ATH_MSG_INFO("Photon has no caloCluster");
        return false;
    }
    fillLabel(monGroup, "ProbeCutCounter", "HasCluster");

    //      probeLabels=["Photons","HasCluster","EtCut","Eta","IsGoodOQ","NearbyJet","ZMass","GoodProbe","","PassTrigger","Et22","Et25","Et35","Et50"]

    const float pt = phot->pt() / Gaudi::Units::GeV;
    if (pt > 22.0){
    fillLabel(monGroup, "ProbeCutCounter", "Et22");
    }

    if (pt > 25.0){
    fillLabel(monGroup, "ProbeCutCounter", "Et25");
    }

    if (pt > 35.0){
    fillLabel(monGroup, "ProbeCutCounter", "Et35");
    }

    if (pt > 50.0){
    fillLabel(monGroup, "ProbeCutCounter", "Et50");
    }

    if (pt < 15.0) return false;
    fillLabel(monGroup, "ProbeCutCounter", "EtCut");



    if(m_rmCrack){
        float absEta = fabs(clus->etaBE(2));
        if ((absEta > 1.37 && absEta < 1.52) || absEta > 2.37)
            return false;
    }
    fillLabel(monGroup, "ProbeCutCounter", "Eta");

    // Object quality
    if (!phot->isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON))
        return false;
    fillLabel(monGroup, "ProbeCutCounter", "IsGoodOQ");

    // Optional jet veto
    if(m_applyJetNearProbeSelection && jets){
        TLorentzVector probe;
        probe.SetPtEtaPhiE(phot->pt(), phot->eta(), phot->phi(), phot->e());

        int jetsAroundPhoton = 0;
        for(const auto *jet : *jets){
            TLorentzVector j;
            j.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
            if(j.Et() > 20*Gaudi::Units::GeV && j.DeltaR(probe) < 0.4)
                jetsAroundPhoton++;
        }
        if(jetsAroundPhoton >= 2)
            return false;
    }
    fillLabel(monGroup, "ProbeCutCounter", "NearbyJet");

    return true;
}

