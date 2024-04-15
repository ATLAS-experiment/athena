
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
*/


// ********************************************************************
// NAME:     LArNoiseCorrelationMonAlg.cxx
// PACKAGE:  LArMonTools
//
// AUTHOR:   Margherita Spalla (margherita.spalla@cern.ch)
//           Based on structure of LArDigitMon by Helary Louis (helary@lapp.in2p3.fr)
// 
// Computes and plots the correlation between single channels to monitor coherent noise. 
// The correlation is computed as: 
//    corr(i,j) = Cov(i,j)/sqrt(Var(i)*Var(j)) where 'Cov(i,j)' is the sample covariance of channels  i and j and 'Var(i)' is the sample variance of channel i.
//    Variance and covariance are computed summing over all samples and all events for each channel: Cov(i,j)=[sum(x_i*x_j)-N*mean_i*mean_j]/(N-1) , where x_i is the single sample minus the pedestal.
//
// Correlation histograms are computed per FEB. The FEBs to be monitored are set in the JO.
//
// Available parameters in the jo file:
//   1) List of FEBs to be monitored: should be passed as a list of strings of the form 'BarrelCFT00Slot02'. If the list is empty, all FEBs are monitored.
//   2) control PublishAllFebsOnline: if it is set to true, switched off the monitoring in case the FEB list (1) is empty and the algorithm is running online.
//   3) list of triggers to be used ('TriggerChain'): to be passed as a single string "trigger_chain_1, trigger_chain_2". The default is "HLT_noalg_zb_L1ZB, HLT_noalg_cosmiccalo_L1RD1_EMPTY", for the latter, only events in the abort gap are used, selection done by hand.
//   4) control IsCalibrationRun: to be set to true when running on calibration, it switches off the trigger selection.
//   5) control PublishPartialSums: tells the algorithm to also publish the partial sum histograms. default is false.

//
// ********************************************************************

//LAr infos:
#include "Identifier/HWIdentifier.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"

//Helper:
#include "LArMonitoring/LArStrHelper.h"

//Header:
#include "LArNoiseCorrelationMonAlg.h"


/*---------------------------------------------------------*/
LArNoiseCorrelationMonAlg::~LArNoiseCorrelationMonAlg()
{ }


/*---------------------------------------------------------*/
StatusCode 
LArNoiseCorrelationMonAlg::initialize()
{
  
  ATH_MSG_INFO( "Initialize LArNoiseCorrelationMonAlg" );  
  
  ATH_CHECK(detStore()->retrieve( m_LArOnlineIDHelper, "LArOnlineID" ));

  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_keyPedestal.initialize());
  ATH_CHECK(m_LArDigitContainerKey.initialize());
  
  /** Get bad-channel mask (only if jO IgnoreBadChannels is true)*/
  ATH_CHECK(m_bcContKey.initialize(m_ignoreKnownBadChannels));
  ATH_CHECK(m_bcMask.buildBitMask(m_problemsToMask,msg()));

  // initialize superclass
  ATH_CHECK( AthMonitorAlgorithm::initialize() ); 

  if(m_plotsOFF) return StatusCode::SUCCESS; // will not plot anything

  /*now the group*/
  m_noiseCorrGroups=Monitored::buildToolMap<int>(m_tools,m_noiseCorrGroupName,m_FEBlist);
  
  
  const std::set<std::string> febSet(m_FEBlist.begin(),m_FEBlist.end());

  //Check for abort-gap if the trigger m_abortGapTrig is in the list of triggers
  m_checkAbortGap=std::find(m_vTrigChainNames.begin(),m_vTrigChainNames.end(),m_abortGapTrig)!=m_vTrigChainNames.end();


  /** helper for feb names*/
  LArStrHelper larStrHelp;

  //Pre-fill the 'model' of the internal structure (to be copied for each event) 
  for (HWIdentifier fid : m_LArOnlineIDHelper->feb_range()) {
      const std::string fName=larStrHelp.febNameString(m_LArOnlineIDHelper->isEMBchannel(fid),m_LArOnlineIDHelper->pos_neg(fid),m_LArOnlineIDHelper->feedthrough(fid),m_LArOnlineIDHelper->slot(fid));
      if (febSet.find(fName)!=febSet.end()) {
        m_febMapModel.emplace(std::make_pair(fid,perFeb_t(fName)));
      }
  }

  return StatusCode::SUCCESS;
}


/*---------------------------------------------------------*/
StatusCode 
LArNoiseCorrelationMonAlg::fillHistograms(const EventContext& ctx) const
{
  if(m_plotsOFF) {
    ATH_MSG_DEBUG("Plotting switched off, either we are online and custom FEB list is empty, OR something went wrong with the custom list of FEBs passed");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("in fillHists()" ); 


  /** check for abort gap*/
  if (m_checkAbortGap && !m_trigDecTool.empty()) {
    // BCIDs of the abort gap
    constexpr unsigned int ABORT_GAP_START = 3446;
    constexpr unsigned int ABORT_GAP_END   = 3563;
    const unsigned int bcid=ctx.eventID().bunch_crossing_id();
    if (m_trigDecTool->isPassed(m_abortGapTrig) && bcid<ABORT_GAP_START && bcid>ABORT_GAP_END) {
      ATH_MSG_DEBUG("Passed trigger ["<<m_abortGapTrig <<"] and bcid "<< bcid <<" falls outside the abort gap. Ignoring this event");
      return StatusCode::SUCCESS;  
    }
  }
  
  /*retrieve cabling*/
  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey,ctx};
  const LArOnOffIdMapping* cabling=*cablingHdl;
  if(!cabling) {
     ATH_MSG_ERROR("Do not have cabling map with key: "<<m_cablingKey.key());
     return StatusCode::FAILURE;
  }

  /*retrieve pedestal*/
  SG::ReadCondHandle<ILArPedestal> pedestalHdl{m_keyPedestal,ctx};
  const ILArPedestal* pedestals=*pedestalHdl;

  const LArBadChannelCont* bcCont=nullptr;
  if (m_ignoreKnownBadChannels) {
    SG::ReadCondHandle<LArBadChannelCont> bcContHdl{m_bcContKey,ctx};
    bcCont=(*bcContHdl);
  }


  /** retrieve LArDigitContainer*/
  SG::ReadHandle<LArDigitContainer> pLArDigitContainer{m_LArDigitContainerKey,ctx};
  
  ATH_MSG_DEBUG ( " LArDigitContainer size "<<pLArDigitContainer->size()<<" for key "<<m_LArDigitContainerKey); 

  //copy model-map ...
  std::map<HWIdentifier,perFeb_t> febMap{m_febMapModel};
  //... and pre-fill with pointers to digits and pedestals
  for (const LArDigit* pDig : *pLArDigitContainer) {
    const HWIdentifier chid=pDig->channelID();
    const HWIdentifier fid=m_LArOnlineIDHelper->feb_Id(chid);
    auto febDat=febMap.find(fid);
    if (febDat!=febMap.end()) {
      CaloGain::CaloGain gain = pDig->gain();
	    double ped = pedestals->pedestal(chid,gain);
       if(isGoodChannel(chid,ped,cabling,bcCont)) {
        febDat->second.m_digitsAndPed.emplace_back(std::make_pair(pDig,ped));
       }
    }

  }

  //now fill the plots
  for (auto& [febid,febdat] : febMap) {
    febdat.sumSamples(m_LArOnlineIDHelper);
    auto chanMean = Monitored::Collection("chanMean",febdat.m_meanSum,[](const std::pair<int,double> ch){return ch.second;});
    auto chanMeanX = Monitored::Collection("chanMeanX",febdat.m_meanSum,[](const std::pair<int,double> ch){return ch.first;});
    auto chanPartSum = Monitored::Collection("chanPartSum",febdat.m_partSum,[](const std::pair<std::pair<int,int>,double> ch){return ch.second;});
    auto chanPartSumX = Monitored::Collection("chanPartSumX",febdat.m_partSum,[](const std::pair<std::pair<int,int>,double> ch){return std::min(ch.first.first,ch.first.second);});
    auto chanPartSumY = Monitored::Collection("chanPartSumY",febdat.m_partSum,[](const std::pair<std::pair<int,int>,double> ch){return std::max(ch.first.first,ch.first.second);}); //needs max and min to fill the correlation plot always on the same side of the diagonal, otherwise it would be mixed up
  
    //fill the correct FEB
    fill(m_tools[m_noiseCorrGroups.at(febdat.m_febName)],chanMean,chanMeanX,chanPartSum,chanPartSumX,chanPartSumY);

    /* actual correlations will be computed at post-processing stage */

  }
  return StatusCode::SUCCESS;
}



/*---------------------------------------------------------*/
/** check if channel is ok for monitoring */
bool LArNoiseCorrelationMonAlg::isGoodChannel(const HWIdentifier ID,const float ped, const LArOnOffIdMapping *cabling, const LArBadChannelCont* bcCont) const
 {
    /** Remove problematic channels*/
   if (m_ignoreKnownBadChannels && m_bcMask.cellShouldBeMasked(bcCont,ID))
     return false;

    /**skip cells with no pedestals reference in db.*/
    if(ped <= (1.0+LArElecCalib::ERRORCODE))
      return false;
    
    /**skip disconnected channels:*/
    if(!cabling->isOnlineConnected(ID))
      return false;

    return true;
 }


/*---------------------------------------------------------*/


void LArNoiseCorrelationMonAlg::perFeb_t::sumSamples(const LArOnlineID* lArOnlineIDHelper) {
  const size_t S=m_digitsAndPed.size();
  for (size_t i1=0;i1<S;++i1) {
    const auto& [pDig1,pedestal1]=m_digitsAndPed[i1];
    const int ch1 = lArOnlineIDHelper->channel(pDig1->channelID());
    //Sum mean:
    for (const short adc : pDig1->samples()) {
      m_meanSum.emplace_back(ch1,adc-pedestal1);
    }  
    //sum of squares
    for (size_t i2=i1;i2<S;++i2) {
      const auto& [pDig2,pedestal2]=m_digitsAndPed[i2];
      const int ch2 = lArOnlineIDHelper->channel(pDig2->channelID());
      double sumSquare=0;
      const unsigned nADC=std::min(pDig1->nsamples(),pDig2->nsamples());    
      for (unsigned i=0;i<nADC;++i) {
        sumSquare+=((pDig1->samples().at(i)-pedestal1)*(pDig2->samples().at(i)-pedestal2));
      }//end loop over samples   
     m_partSum.emplace_back(std::make_pair(std::make_pair(ch1,ch2),sumSquare));
    }//end loop pDig2
  }//end loop pDig1 
}
