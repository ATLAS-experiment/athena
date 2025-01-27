/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigEgammaMonitorPhotonAlgorithm.h"
#include "LArRecEvent/LArEventBitInfo.h"
#include "StoreGate/ReadDecorHandle.h"


using namespace Trig;


TrigEgammaMonitorPhotonAlgorithm::TrigEgammaMonitorPhotonAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ):
  TrigEgammaMonitorAnalysisAlgorithm( name, pSvcLocator )
{
}

TrigEgammaMonitorPhotonAlgorithm::~TrigEgammaMonitorPhotonAlgorithm()
{}



StatusCode TrigEgammaMonitorPhotonAlgorithm::initialize() 
{
  
  ATH_CHECK(TrigEgammaMonitorAnalysisAlgorithm::initialize());

  ATH_CHECK(m_offPhotonKey.initialize());
  ATH_CHECK(m_offPhotonIsolationKeys.initialize());
  ATH_CHECK( m_eventInfoDecorKey.initialize() );
  
  for(auto& trigName : m_trigInputList)
  {
    if(getTrigInfoMap().count(trigName) != 0){
      ATH_MSG_DEBUG("Trigger already booked, removing from trigger list " << trigName);
    }else {
      m_trigList.push_back(trigName);
      setTrigInfo(trigName);
    }
  }
 
  return StatusCode::SUCCESS;
}


StatusCode TrigEgammaMonitorPhotonAlgorithm::fillHistograms( const EventContext& ctx ) const  
{
    ATH_MSG_DEBUG("Executing TrigEgammaMonitorPhotonAlgorithm");


    if(isHLTTruncated()){
        ATH_MSG_DEBUG("HLTResult truncated, skip trigger analysis");
        return StatusCode::SUCCESS; 
    }
    
    // Noise burst protection 
    SG::ReadHandle<xAOD::EventInfo> thisEvent(GetEventInfo(ctx));
    ATH_CHECK(thisEvent.isValid());
    if ( thisEvent->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO)) {
        ATH_MSG_DEBUG("LAr Noise Burst Veto, skip trigger analysis");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Chains for Analysis " << m_trigList);

    for(const auto& trigger : m_trigList){

        const TrigInfo info = getTrigInfo(trigger);
		ATH_MSG_DEBUG("Start Chain Analysis ============================= " << trigger << " " << info.trigger);

		// Check if this trigger is in the bootstrap map
		auto it = m_BSTrigMap.find(trigger);

		if  ( it != m_BSTrigMap.end() ) {

			ATH_MSG_DEBUG( trigger << " is a bootstrapped trigger"); 

			std::string bootstrap = it->second;
			ATH_MSG_DEBUG( "Bootstrapping " << trigger << " from " << bootstrap ); 


			if (!tdt()->isPassed(bootstrap)){
				ATH_MSG_DEBUG("Not passed BS trigger. Skipping! ========================== " << trigger);
				continue;
			} else {
				ATH_MSG_DEBUG("BS trigger passed!");
			}
		
    }
          
        std::vector< std::pair<std::shared_ptr<const xAOD::Egamma>, const TrigCompositeUtils::Decision * >> pairObjs;
    
        if ( executeNavigation( ctx,info,pairObjs).isFailure() ) 
        {
            ATH_MSG_DEBUG("executeNavigation Fails");
            return StatusCode::SUCCESS;
        }

        std::vector< std::pair<const xAOD::Egamma*, const TrigCompositeUtils::Decision*>> pairObjsRaw;
        pairObjsRaw.reserve(pairObjs.size());
        for (const auto& itr : pairObjs) {
          pairObjsRaw.emplace_back(itr.first.get(), itr.second);
        }
        
        fillDistributions( pairObjsRaw, info );
        fillEfficiencies( pairObjsRaw, info, m_onlyHLT );
        fillResolutions( pairObjsRaw, info );


        ATH_MSG_DEBUG("End Chain Analysis ============================= " << trigger);
    } // End loop over trigger list
    
    
    return StatusCode::SUCCESS;
}





StatusCode TrigEgammaMonitorPhotonAlgorithm::executeNavigation( const EventContext& ctx, const TrigInfo& info,
                                                       std::vector<std::pair<std::shared_ptr<const xAOD::Egamma>, const TrigCompositeUtils::Decision * >> &pairObjs) 
  const
{
  ATH_MSG_DEBUG("Apply navigation selection for Photons");

  SG::ReadHandle<xAOD::PhotonContainer> offPhotons(m_offPhotonKey, ctx);

  if(!offPhotons.isValid())
  {
    ATH_MSG_DEBUG("Failed to retrieve offline photons ");
	  return StatusCode::FAILURE;
  }
  const std::string trigItem = info.trigger;
  const float etthr = info.etthr;
  const std::string pidName = info.pidname;
  const std::string decor="is"+pidName;

  for(const auto *const eg : *offPhotons ){
      const TrigCompositeUtils::Decision *dec=nullptr; 
      if(!eg->caloCluster()){
          ATH_MSG_DEBUG("No caloCluster");
          continue;
      } 
      if( !(getCluster_et(eg) > (etthr-5.)*Gaudi::Units::GeV)) continue; //Take 5 GeV below threshold
      if(!info.etcut){
        if(!eg->passSelection(m_photonPid)) {
          ATH_MSG_DEBUG("Fails PhotonID: " << m_photonPid);
          continue; // reject offline photons reproved by tight requiriment
        }
      }
      if(m_forcePidSelection){///default is true
        if(!ApplyPhotonPid(eg,pidName)){
	        ATH_MSG_DEBUG("Fails PhotonID: "<< pidName << " Trigger: " << trigItem);
	        continue;
	      }
	      ATH_MSG_DEBUG("Passes PhotonID "<< pidName);
      }
      // default is false: if true, skip converted photons 
      if(m_doUnconverted){
          if (eg->vertex()){
              ATH_MSG_DEBUG("Removing converted photons, continuing...");
              continue;
          }
      }
      const auto ph = std::make_shared<const xAOD::Photon>(*eg);
      ph->auxdecor<bool>(decor)=static_cast<bool>(true);
      match()->match(ph.get(), trigItem, dec, TrigDefs::includeFailedDecisions);
      //match()->match(ph, trigItem, dec);
      pairObjs.emplace_back(ph, dec);
      // }

  }

  ATH_MSG_DEBUG("BaseToolMT::Photon TEs " << pairObjs.size() << " found.");
  return StatusCode::SUCCESS;
}
