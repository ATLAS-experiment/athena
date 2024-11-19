/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibDigitsAccumulator.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "xAODEventInfo/EventInfo.h"
#include <cmath>
#include <cstdint>

using CLHEP::ns;

LArCalibDigitsAccumulator::LArCalibDigitsAccumulator (const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator),
  m_sc2ccMappingTool("CaloSuperCellIDTool"), 
  m_onlineHelper(nullptr),
  m_sampleShift(0)
{
  declareProperty("LArAccuCalibDigitContainerName",m_calibAccuDigitContainerName, "LArAccumulatedCalibDigits");
  declareProperty("KeyList",m_keylist);
  declareProperty("StepOfTriggers",m_nStepTrigger=1);
  declareProperty("DelayScale",m_delayScale=1*ns);
  declareProperty("KeepOnlyPulsed",m_keepPulsed=false);
  declareProperty("KeepFullyPulsedSC",m_keepFullyPulsedSC=false);
  declareProperty("DropPercentTrig",m_DropPercentTrig=10);
  declareProperty("isSC",m_isSC=false);
  declareProperty("SampleShift",m_sampleShift=0);
  m_delay=-1;
  m_event_counter=0;
}


StatusCode LArCalibDigitsAccumulator::initialize(){
  
  m_eventNb=0;
  // retrieve online ID helper
  StatusCode sc;
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_calibMapKey.initialize() );

  if(m_isSC){
     const LArOnline_SuperCellID *scid;
     sc = detStore()->retrieve(scid, "LArOnline_SuperCellID");
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "Could not get LArOnline_SuperCellID helper !" );
        return sc;
     } else {
        m_onlineHelper = (const LArOnlineID_Base*)scid;
        ATH_MSG_DEBUG("Found the LArOnlineID helper");
     }
     ATH_CHECK( m_sc2ccMappingTool.retrieve() );
     ATH_CHECK( m_cablingKeySC.initialize() );
     ATH_CHECK( m_calibMapSCKey.initialize() );
  } else { 
     const LArOnlineID* ll;
     sc = detStore()->retrieve(ll, "LArOnlineID");
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "Could not get LArOnlineID helper !" );
        return sc;
     } else {
        m_onlineHelper = (const LArOnlineID_Base*)ll;
        ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
     }
  } //m_isSC

  return StatusCode::SUCCESS;
}



StatusCode LArCalibDigitsAccumulator::execute() 
{
  
  StatusCode sc;

  if ( m_event_counter < 100 || m_event_counter%100==0 )
    ATH_MSG_INFO( "Processing event " << m_event_counter );
  ++m_event_counter;
  
  const LArCalibLineMapping *clcabling = nullptr;
  if(m_isSC) {
     SG::ReadCondHandle<LArCalibLineMapping> clHdl{m_calibMapSCKey};
     clcabling =*clHdl;
  } else {
     SG::ReadCondHandle<LArCalibLineMapping> clHdl{m_calibMapKey};
     clcabling =*clHdl;
  }
  if(!clcabling) {
    ATH_MSG_WARNING( "Do not have calib line mapping from key " << m_calibMapKey.key() );
    return StatusCode::FAILURE;
  }
  
  // new here ====
  const LArOnOffIdMapping* cabling(nullptr);
  const LArOnOffIdMapping* cablingLeg(nullptr);
  if( m_isSC ){
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKeySC};
    cabling = {*cablingHdl};
    if(!cabling) {
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKeySC.key());
      return StatusCode::FAILURE;
    }
    
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdlLeg{m_cablingKey};
    cablingLeg = {*cablingHdlLeg};
    if(!cablingLeg) {
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKey.key());
      return StatusCode::FAILURE;
    }
  }

  // =============

   

  // pointer to input container
  const LArCalibDigitContainer* calibDigitContainer=nullptr;

  // Event info 
  const xAOD::EventInfo* thisEventInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve(thisEventInfo) );
  unsigned eventNb = thisEventInfo->eventNumber();
  // retrieve calibration settings
  const LArCalibParams* calibParams;
  sc=detStore()->retrieve(calibParams,"LArCalibParams");
  if (sc.isFailure())
    {ATH_MSG_ERROR( "Cannot load LArCalibParams from DetStore." );
      return StatusCode::FAILURE;
    }

  unsigned int sizeSteps = (m_nStepTrigger>1 ? (m_nStepTrigger+1):1);

  // retrieve input calibDigits
  
  //Loop over all containers that are to be processed (e.g. different gains)
  for (const std::string& key : m_keylist) {

    sc=evtStore()->retrieve(calibDigitContainer,key);
    if(sc.isFailure()) {
      ATH_MSG_ERROR( "Can't retrieve LArCalibDigitContainer with key " << key << "from StoreGate." );
      return StatusCode::SUCCESS;
    }else{
      ATH_MSG_DEBUG( "Retrieved LArCalibDigitContainer with key " << key << " from StoreGate." );
    }

    if(calibDigitContainer->empty()) {
      ATH_MSG_DEBUG( "LArCalibDigitContainer with key=" << key << " is empty " );
    }else{
      ATH_MSG_DEBUG( "LArCalibDigitContainer with key=" << key << " has size =  " << calibDigitContainer->size() );
    }

    // counter of triggers 
    std::vector<unsigned int> ntrigger, nTriggerPerStep, nStepTrigger, iStepTrigger;
    // effective number of triggers: per channel
    ntrigger.resize(calibDigitContainer->size(),0);
    // asked number of triggers (from calib settings): per FEB
    nTriggerPerStep.resize(m_onlineHelper->febHashMax(),0);
    nStepTrigger.resize(m_onlineHelper->febHashMax(),0);
    iStepTrigger.resize(m_onlineHelper->febHashMax(),0);

    // output container
    LArAccumulatedCalibDigitContainer* larAccuCalibDigitContainer = new LArAccumulatedCalibDigitContainer();
    //Loop over all cells
    for (const LArCalibDigit* digit : *calibDigitContainer) {
      if(m_keepPulsed && !digit->isPulsed()) continue;
      
      // identificators
      HWIdentifier chid=digit->hardwareID();      
      const HWIdentifier febid=m_onlineHelper->feb_Id(chid);
      const IdentifierHash febhash = m_onlineHelper->feb_Hash(febid);
      const IdentifierHash hashid = m_onlineHelper->channel_Hash(chid);


      
      //// ############################

      std::vector<Identifier> ccellIds(0);
      unsigned numPulsedLeg = 0;
      unsigned numCL = 0;
      if( m_isSC ){

	///first check if there is a -1 and continue
	bool hasInvalid=false;
	for(auto s : digit->samples()){
	  if(s<0){
	    hasInvalid=true;
	    break;
	  }
	}
	if(hasInvalid) continue;

	Identifier myofflineID = cabling->cnvToIdentifier(digit->hardwareID()) ;
	ccellIds = m_sc2ccMappingTool->superCellToOfflineID( myofflineID );
        for (Identifier id : ccellIds) {// loop cells in sc
	  HWIdentifier cellLegHWID  = cablingLeg->createSignalChannelID(id);
	  const std::vector<HWIdentifier>& calibLineLeg = clcabling->calibSlotLine(cellLegHWID);
	  numCL += calibLineLeg.size();
          for (HWIdentifier calibLineHWID : calibLineLeg) {// loop legacy calib lines
	    if ( calibParams->isPulsed(eventNb,calibLineHWID) ){
	      numPulsedLeg += 1;
	    }else{
	      if ( digit->isPulsed() ) ATH_MSG_WARNING("SC "<< chid << " constituent cell "<< cellLegHWID << " calib line "<< calibLineHWID<< " not pulsed");}
	  }//end calib line Leg loop
	}//end legacy cell loop
	if ( digit->isPulsed() && numPulsedLeg != numCL ){ //(int)(ccellIds.size()) ){
	  ATH_MSG_WARNING("Number of pulsed legacy cells does not equal number of calibration lines "<<chid<<"!! LArParams counter = " << numPulsedLeg << ", SC2CCMappingTool = " << ccellIds.size() << ", num CLs = "<< numCL);

	  if(m_keepFullyPulsedSC){ // && numPulsedLeg < (int)(ccellIds.size())){
	    ATH_MSG_WARNING("Discarding this SC ("<<chid<<") as it is not fully pulsed");
	    continue;
	  }
	}
	ATH_MSG_DEBUG("SC "<<chid<<" pulsed cells "<< numPulsedLeg <<" or "<< ccellIds.size()<<", "<<numCL<<" calibration lines");
      } // end m_isSC

      //// ############################

      // BELOW: DIRTY HACK BECAUSE THERE SEEMS TO BE A BUG IN THE CABLINGSVC CONCERNING THE CALIBLINES.
      // get calibration settings
      const std::vector<HWIdentifier>& calibLineID=clcabling->calibSlotLine(chid);
      HWIdentifier calibModuleID;
      if(!calibLineID.empty()){
	calibModuleID=m_onlineHelper->calib_module_Id(calibLineID[0]);
	nTriggerPerStep[febhash] = calibParams->NTrigger(calibModuleID);
	ATH_MSG_DEBUG( "Ntrigger per step = " << nTriggerPerStep[febhash] );
	if(nTriggerPerStep[febhash] > 1000) nTriggerPerStep[febhash]=100; // very dirty !!! 
      }else{

	nTriggerPerStep[febhash] = 100; // very dirty !! 
      }

      // cell is pulsed ? 
      bool isPulsed = digit->isPulsed();
      
      //First cell to be processed, set delay
      if (m_delay==-1) { /// why this is not reset to -1 each event, does not make sense
	m_delay=digit->delay();
      }
      else{
	// next cells: should be the same delay
	if (m_delay!=digit->delay()) {
	  ATH_MSG_DEBUG( "Delay is changing to " << digit->delay() << " from " << m_delay << ": book a new LArAccumulatedCalibDigitContainer" );
	  m_delay=digit->delay();
	}
      }
      
      std::string patternName = getPatternName(key, isPulsed, m_delay, digit->DAC());

      std::map<std::string, std::vector<LArAccumulated> >::iterator accIter=m_Accumulated_map.find(patternName);
      if(accIter==m_Accumulated_map.end()){
	accIter=m_Accumulated_map.insert(std::make_pair(patternName,std::vector<LArAccumulated>(m_onlineHelper->channelHashMax()))).first;
      }

      CaloGain::CaloGain gain=digit->gain();
      if (gain<0 || gain>CaloGain::LARNGAIN)
	{ATH_MSG_ERROR( "Found not-matching gain number ("<< (int)gain <<")" );
          delete larAccuCalibDigitContainer;
	  return StatusCode::FAILURE;
	}
      
      // object to be filled for each cell
      LArAccumulated& cellAccumulated = accIter->second[hashid];
      cellAccumulated.m_onlineId=chid.get_identifier32().get_compact();

      // trigger counter for each cell
      cellAccumulated.m_ntrigger++;
      ATH_MSG_DEBUG( "chid = " << chid << ", trigger = " << cellAccumulated.m_ntrigger << ", DAC = " << digit->DAC()<<", Delay = "<<digit->delay()<<", isPulsed? "<<digit->isPulsed() );
            
      // at first trigger, initialize vectors
      unsigned int sizeSamples = digit->samples().size();
      ATH_MSG_DEBUG( "sizeSteps = " << sizeSteps << ", # of samples = " << sizeSamples );

      if(cellAccumulated.m_ntrigger==1){
	cellAccumulated.m_sum.clear();
	cellAccumulated.m_sum2.clear();
	cellAccumulated.m_sum.resize(sizeSamples,0);
	cellAccumulated.m_sum2.resize(sizeSamples,0);
      }    

      /// assuming sizeSamples>m_sampleShift, is there a protection for this somehwere?
      for(unsigned int j=0;j<sizeSamples;j++){
	int digis;
	if(j-m_sampleShift >= sizeSamples){
	  digis=digit->samples()[sizeSamples-1];
	  cellAccumulated.m_sum[j] += digis;
	  cellAccumulated.m_sum2[j] += digis*digis;
	}
	else if((int)j-m_sampleShift<0){
	  digis=digit->samples()[0];
	  cellAccumulated.m_sum[j] += digis;
	  cellAccumulated.m_sum2[j] += digis*digis;
	} else{
	  digis=digit->samples()[j-m_sampleShift];
	  cellAccumulated.m_sum[j] += digis;
	  cellAccumulated.m_sum2[j] += digis*digis;
	}
      }
     
      // when reached total number of triggers for this step, fill LArAccumulatedCalibDigit and reset number of triggers
      unsigned int ntrigUsed = nTriggerPerStep[febhash];
      if ( m_isSC && m_DropPercentTrig != 0 ){
	ntrigUsed -= ntrigUsed*(m_DropPercentTrig/100);
      }

      if(cellAccumulated.m_ntrigger==ntrigUsed){ 
	ATH_MSG_DEBUG( "filling LArAccumulatedCalibDigit " );
	ATH_MSG_DEBUG( "chid = " << chid << ", gain = " << gain << ", DAC = " << digit->DAC() << ", isPulsed = " << isPulsed << ", delay = " << m_delay << ", trigPerStep = " << nTriggerPerStep[febhash] << ", istep = " << iStepTrigger[febhash] );


	LArAccumulatedCalibDigit* accuCalibDigit;
	if ( m_isSC ){	  
      	  ATH_MSG_DEBUG("Channel "<<chid<<" DAC "<< digit->DAC()<< " will multiply by "<<numPulsedLeg<< " = "<<digit->DAC()*numPulsedLeg<<" is pulsed??? "<<isPulsed);
      	  accuCalibDigit = new LArAccumulatedCalibDigit(chid,gain,sizeSamples,(int32_t)(digit->DAC()*numPulsedLeg),(uint16_t)m_delay,isPulsed,0,0);
      	}else{
      	  accuCalibDigit = new LArAccumulatedCalibDigit(chid,gain,sizeSamples,(uint16_t)digit->DAC(),(uint16_t)m_delay,isPulsed,0,0);
      	}


	accuCalibDigit->setAddSubStep(cellAccumulated.m_sum,cellAccumulated.m_sum2,ntrigUsed);
	iStepTrigger[febhash]++;
	
	std::vector<float> mean =  accuCalibDigit->mean();
	std::vector<float> RMS =  accuCalibDigit->RMS();

	for(unsigned int i=0;i<mean.size();i++){
	  ATH_MSG_DEBUG( "mean["<<i<<"] = " << mean[i] );
	  ATH_MSG_DEBUG( "RMS["<<i<<"] = " << RMS[i] );
	}

	larAccuCalibDigitContainer->push_back(accuCalibDigit);

	cellAccumulated.m_nused = cellAccumulated.m_ntrigger;
	cellAccumulated.m_ntrigger = 0;
      }
      
    }// loop over cells in container


    larAccuCalibDigitContainer->setDelayScale(m_delayScale);
    sc = evtStore()->record(larAccuCalibDigitContainer,key);
    if (sc!=StatusCode::SUCCESS)
      {ATH_MSG_WARNING( "Unable to record LArAccumulatedCalibDigitContainer with key " << key << " from DetectorStore. " );
      } 
    else
      ATH_MSG_DEBUG( "Recorded succesfully LArAccumulatedCalibDigitContainer with key " << key  << " with size " << larAccuCalibDigitContainer->size());
    
    sc = evtStore()->setConst(larAccuCalibDigitContainer);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( " Cannot lock LArAccumulatedCalibDigitContainer " );
      return(StatusCode::FAILURE);
    }


  } // loop over key container
  return StatusCode::SUCCESS;
}
  
std::string LArCalibDigitsAccumulator::getPatternName(const std::string& gain, bool isPulsed, int delay, int dac){
  std::ostringstream ss;
  ss<<gain<<"_"<<isPulsed<<"_"<<delay<<"_"<<dac;
  return ss.str();
}


StatusCode LArCalibDigitsAccumulator::finalize(){

  if ( !m_isSC ) return StatusCode::SUCCESS;
  int ntrigUsed = 100 - 100*(m_DropPercentTrig/100); /// assuming 100 per step
  for(auto &acc : m_Accumulated_map){
    std::string pattern=acc.first;
    for(auto &sc : acc.second){
      if(sc.m_onlineId && sc.m_nused != ntrigUsed){
	ATH_MSG_WARNING("Not enough triggers for pattern " << pattern << " channel OnlineID " << sc.m_onlineId << " ntriggers " << sc.m_ntrigger );
      }
    }
  }

  return StatusCode::SUCCESS;

}
