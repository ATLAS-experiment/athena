/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     LArRODMonAlg.cxx
// PACKAGE:  LArMonitoring
//
// ********************************************************************

#include "LArRODMonAlg.h"

#include "Identifier/IdentifierHash.h"
#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArRawEvent/LArDigit.h"

#include "LArRawEvent/LArRawChannel.h"
#include "LArRawEvent/LArFebHeaderContainer.h"

#include "LArRawEvent/LArFebHeader.h" 

#include "LArCabling/LArOnOffIdMapping.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include <cmath>
#include <algorithm>
#include <format>
#include <type_traits>

/*---------------------------------------------------------*/
LArRODMonAlg::~LArRODMonAlg() {}

/*---------------------------------------------------------*/
StatusCode 
LArRODMonAlg::initialize() {
  ATH_MSG_VERBOSE( "In LArRODMonAlg::initialize() ");

  ATH_CHECK(detStore()->retrieve(m_LArOnlineIDHelper, "LArOnlineID"));

  ATH_CHECK(m_channelKey_fromDigits.initialize());
  ATH_CHECK(m_channelKey_fromBytestream.initialize());
  ATH_CHECK(m_digitContainerKey.initialize());
  ATH_CHECK(m_headerContainerKey.initialize());

  ATH_CHECK(m_keyOFC.initialize());
  ATH_CHECK(m_keyShape.initialize());
  ATH_CHECK(m_keyHVScaleCorr.initialize());
  ATH_CHECK(m_keyPedestal.initialize());

  ATH_CHECK(m_adc2mevKey.initialize());

  ATH_CHECK(m_noiseCDOKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());

  ATH_CHECK(m_bcContKey.initialize());
  ATH_CHECK(m_bcMask.buildBitMask(m_problemsToMask,msg()));

  ATH_CHECK( m_eventInfoKey.initialize() );


  //Check properties ... 
  if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) { //MT  environment
    if (m_doDspTestDump) {
      ATH_MSG_ERROR("Property 'DoDspTestDump' must not be true if nThreads>1");
      return StatusCode::FAILURE;
    }
    if (m_doCellsDump) {
      ATH_MSG_ERROR("Property 'DoCellsDump' must not be true if nThreads>1");
      return StatusCode::FAILURE;
    }
  }


  auto pairCmp = [](const std::pair<int,int>& p1, const std::pair<int,int>& p2) {return (p1.first<p2.first);};
  if (!std::is_sorted(m_E_precision.begin(),m_E_precision.end(),pairCmp)) {
      ATH_MSG_ERROR("Configuration problem: Energy ranges not in ascending order!");
      return StatusCode::FAILURE;
  }

  if (!std::is_sorted(m_T_precision.begin(),m_T_precision.end(),pairCmp)) {
      ATH_MSG_ERROR("Configuration problem: Time ranges not in ascending order!");
      return StatusCode::FAILURE;
  }

  if (!std::is_sorted(m_Q_precision.begin(),m_Q_precision.end(),pairCmp)) {
      ATH_MSG_ERROR("Configuration problem: Quality ranges not in ascending order!");
      return StatusCode::FAILURE;
  }


  // Open output files for DspTest
  if (m_doDspTestDump) {
    m_fai.open(m_AiFileName,std::ios::trunc);
    m_fdig.open(m_DigitsFileName,std::ios::trunc);
    m_fen.open(m_EnergyFileName,std::ios::trunc);
  }

  // Output file
  if (m_doCellsDump) {
    m_fdump.open(m_DumpCellsFileName);
    m_fdump<<"febid        channel CellID       slot   FT     barrel_ec posneg partition E_off     E_on     T_off     T_on     Q_off     Q_on     event   "<<std::endl;
  }
  

  ATH_MSG_DEBUG("Setting an offset time of " << m_timeOffset << " BC, i.e. " << m_timeOffset*m_BC << " ps");

  m_histoGroups = Monitored::buildToolMap<int>(m_tools,m_MonGroupName, m_partitions);

  return AthMonitorAlgorithm::initialize();
}

StatusCode LArRODMonAlg::fillHistograms(const EventContext& ctx) const {
  ATH_MSG_VERBOSE( "In LArRODMonAlg::fillHistograms()");

 //monitored variables
  auto weight_e = Monitored::Scalar<float>("weight_e",1.);
  auto weight_q = Monitored::Scalar<float>("weight_q",1.);
  auto weight_t = Monitored::Scalar<float>("weight_t",1.);
  auto numE = Monitored::Scalar<int>("numE",1.);
  auto numQ = Monitored::Scalar<int>("numQ",1.);
  auto numT = Monitored::Scalar<int>("numT",1.);
  auto gain = Monitored::Scalar<int>("gain",-1);
  auto partition = Monitored::Scalar<int>("partition",-1);
  auto partitionI = Monitored::Scalar<int>("partitionI",-1);
  auto lb = Monitored::Scalar<int>("LBN",0);
  auto sweetc = Monitored::Scalar<float>("Sweetc",1.);

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo{GetEventInfo(ctx)};

  SG::ReadCondHandle<ILArPedestal>    pedestalHdl{m_keyPedestal, ctx};
  const ILArPedestal* pedestals=*pedestalHdl;

  //retrieve BadChannel info:
  SG::ReadCondHandle<LArBadChannelCont> bcContHdl{m_bcContKey,ctx};
  const LArBadChannelCont* bcCont{*bcContHdl};

  bool isEventFlaggedByLArNoisyROAlg = false; // keep default as false
  bool isEventFlaggedByLArNoisyROAlgInTimeW = false; // keep deault as false

  if ( thisEventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,0) ) {
    isEventFlaggedByLArNoisyROAlg = true;
    ATH_MSG_DEBUG( "Event flagged as Noisy" );
  }

  if ( thisEventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,3) ) {
    isEventFlaggedByLArNoisyROAlgInTimeW = true;
    ATH_MSG_DEBUG( " !!! Noisy event found by LArNoisyROAlg in Time window of 500ms!!!" );
  }
  
  // Noise bursts cleaning (LArNoisyRO_Std or TimeWindowVeto) added by B.Trocme - 19/7/12
  if (m_removeNoiseBursts && (isEventFlaggedByLArNoisyROAlg || isEventFlaggedByLArNoisyROAlgInTimeW)) return StatusCode::SUCCESS;
  
  // Retrieve stream info
  int nStreams = m_streams.size();
   //  if ((nStreams == 1 && m_streams[0] == "all") || nStreams <= 0) selectstreams = false; 
 
  lb = thisEventInfo->lumiBlock(); 
  
  //Fixme: Use LArTrigStreamMatching also here.
  const int streamsize = nStreams + 1;
  std::vector<int> hasStream(streamsize,0);
  //  for (int str = 0; str < nStreams + 1; str++) hasStream[str] = 0;

  bool hasstrlist = false;
  const std::vector< xAOD::EventInfo::StreamTag >& evtStreamTags=thisEventInfo->streamTags();
  for (const auto& evtStreamTag : evtStreamTags) {
    std::vector<std::string>::const_iterator evtStreamTagIt=std::find(m_streams.begin(),m_streams.end(),evtStreamTag.name());
    if (evtStreamTagIt!=m_streams.end()) {
      const unsigned str=evtStreamTagIt-m_streams.begin();
      ATH_MSG_VERBOSE( "Keeping Stream Tag: " << evtStreamTag.type() << "_" << evtStreamTag.name());
      hasStream[str] = 1;
      hasstrlist = true;
    }
  }
  if (! hasstrlist) hasStream[nStreams] = 1; 


  SG::ReadHandle<LArRawChannelContainer> rawColl_fromDigits(m_channelKey_fromDigits, ctx);

  SG::ReadHandle<LArRawChannelContainer> rawColl_fromBytestream(m_channelKey_fromBytestream, ctx);
  
  SG::ReadHandle<LArDigitContainer> pLArDigitContainer(m_digitContainerKey, ctx);

  std::set<HWIdentifier> ignoreFEBs;

  if (m_doCheckSum || m_doRodStatus) {
     const LArFebHeaderContainer* febCont = SG::get(m_headerContainerKey, ctx);
     if (!febCont) {
       ATH_MSG_WARNING( "No LArFEB container found in TDS" ); 
     } else {
       for (const auto* febH : *febCont) {
         if (((m_doCheckSum && febH->ChecksumVerification()==false)) || 
            (m_doRodStatus && febH->RodStatus()!=0)) 
           ignoreFEBs.insert(febH->FEBId());
       }
     }
     ATH_MSG_DEBUG("Found " << ignoreFEBs.size() << " FEBs with checksum errors or statatus errors. Will ignore these FEBs.");
  }
  
  std::vector<ERRCOUNTER> errcounters(N_PARTITIONS);
  ERRCOUNTER allEC;  

  std::vector<unsigned> errsPerFEB;
  errsPerFEB.resize(m_LArOnlineIDHelper->febHashMax(),0);

  const bool ignoreFebs=(ignoreFEBs.size()>0);
  std::set<HWIdentifier>::const_iterator ignoreFebsEnd=ignoreFEBs.end();
  
  unsigned count_gain[(int)CaloGain::LARNGAIN] = {0};

  //Build an association of channels in the two LArRawChannelContainers.
  //The LArRawChannelContainers are unordered collections of LArRawChannels. 
  //But we know that they have the same order because they were built from the same source (namely the LArDigits and RawChannels in the Bytestream)
  //and we know that the LArRawChannels built offline are a subset of the LArRawChannelContainers read from Bytestream.
  //Therfore we can search much more efficiently
  LArRawChannelContainer::const_iterator rcDigIt=rawColl_fromDigits->begin();
  LArRawChannelContainer::const_iterator rcDigIt_e=rawColl_fromDigits->end();
  LArRawChannelContainer::const_iterator rcBSIt=rawColl_fromBytestream->begin();
  LArRawChannelContainer::const_iterator rcBSIt_e=rawColl_fromBytestream->end();

  LArDigitContainer::const_iterator digIt=pLArDigitContainer->begin();
  LArDigitContainer::const_iterator digIt_e=pLArDigitContainer->end();


  //Loop over indices in LArRawChannelContainer built offline (the small one)
  ATH_MSG_DEBUG( "Entering the LArRawChannel loop." );

  for (;rcDigIt!=rcDigIt_e;++rcDigIt) {
    const HWIdentifier idDig=rcDigIt->hardwareID();
    const HWIdentifier febId=m_LArOnlineIDHelper->feb_Id(idDig);
    // Check if this FEB should be ignored
    if (ignoreFebs) { 
      if (ignoreFEBs.find(febId)!=ignoreFebsEnd) continue;
    }
    //Check if this is a bad channel
    if (m_skipKnownProblematicChannels && m_bcMask.cellShouldBeMasked(bcCont,idDig)) continue;
  
    const CaloGain::CaloGain gain = rcDigIt->gain();
    if ((gain >= ERRCOUNTER::N) or (gain < 0)){
      ATH_MSG_WARNING( std::format("gain {} is out of range for the ERRCOUNTER array size, {}",
        static_cast<std::underlying_type_t<CaloGain::CaloGain>>(gain), ERRCOUNTER::N));
      return StatusCode::FAILURE;
    }
    //Check pedestal if needed
    if (m_skipNullPed) {
      const float ped = pedestals->pedestal(idDig,gain);
      if(ped <= (1.0+LArElecCalib::ERRORCODE)) continue;
    }

    //Now look for corresponding channel in the LArRawChannelContainer read from Bytestream (the big one)
    LArRawChannelContainer::const_iterator currIt=rcBSIt; //Remember current position in container
    for (;rcBSIt!=rcBSIt_e && rcBSIt->hardwareID() != idDig; ++rcBSIt);
    if (rcBSIt==rcBSIt_e) {
      ATH_MSG_WARNING( "LArDigitContainer not in the expected order. Change of LArByteStream format?" );
      //Wrap-around
      for (rcBSIt=rawColl_fromBytestream->begin();rcBSIt!=currIt && rcBSIt->hardwareID() != idDig; ++rcBSIt);
      if (rcBSIt==currIt) {
	      ATH_MSG_ERROR( "Channel " << m_LArOnlineIDHelper->channel_name(idDig) << " not found." );
	      return StatusCode::FAILURE;
      }
    }

    //Now look for corresponding channel in the LArDigitContainer read from Bytestream
    //Should be in almost in sync with the RawChannelContainer we are iterating over, 
    //but contains disconnected channels that are not part of the LArRawChannelContainer
    LArDigitContainer::const_iterator currDigIt=digIt; //Remember current position in digit-container
    for (;digIt!=digIt_e && (*digIt)->hardwareID() != idDig; ++digIt);
    if (digIt==digIt_e) {
        ATH_MSG_WARNING( "LArRawChannelContainer not in the expected order. Change of LArRawChannelBuilder behavior?" );
      //Wrap-around
      for (digIt=pLArDigitContainer->begin();digIt!=currDigIt && (*digIt)->hardwareID() != idDig; ++digIt);
      if (digIt==currDigIt) {
	      ATH_MSG_ERROR( "Channel " << m_LArOnlineIDHelper->channel_name(idDig) << " not found in LArDigitContainer." );
	      return StatusCode::FAILURE;
      }
    }
    const LArDigit* dig=*digIt;
    const std::vector<short>& samples=dig->samples();
    const auto [minSamplesIt, maxSamplesIt] = std::minmax_element(samples.begin(),samples.end());
    if (m_adc_th<=0 || (*maxSamplesIt-*minSamplesIt)>m_adc_th) {
      const diff_t compRes=compareChannel(*rcDigIt,*rcBSIt);
      if (compRes.e_on!=compRes.e_off || compRes.t_on!=compRes.t_off || compRes.q_on!=compRes.q_off) {
        if (m_ndump<m_max_dump || m_printEnergyErrors || m_doCellsDump || m_doDspTestDump) {
          detailedOutput(compRes,*dig,ctx);
          ++m_ndump;
        }
        if (m_doCellsDump) {
          dumpCellInfo(idDig,gain,ctx,compRes);
        }
      }
        //Count errors:
      const PARTITION p=getPartition(idDig);
      if (compRes.e_on!=compRes.e_off) {
        ++(errcounters[p].errors_E[gain]);
        ++(allEC.errors_E[gain]);

        IdentifierHash febHash=m_LArOnlineIDHelper->feb_Hash(febId);
        ++(errsPerFEB[febHash]);
      }
      if (compRes.t_on!=compRes.t_off) {//Time-error 
        ++(errcounters[p].errors_T[gain]);
        ++(allEC.errors_T[gain]);
      }

      if (compRes.q_on!=compRes.q_off) {//Quality-error 
        ++(errcounters[p].errors_Q[gain]);
        ++(allEC.errors_Q[gain]);
      }
      
    }
    else {
      ATH_MSG_DEBUG( "Samples : "<< *maxSamplesIt << " " << *minSamplesIt );
    }      
  }//end loop over rawColl_fromDigits

  ATH_MSG_DEBUG( "End of rawChannels loop" );
  
  for (unsigned i=0;i<m_LArOnlineIDHelper->febHashMax();++i) {
    const HWIdentifier febid=m_LArOnlineIDHelper->feb_Id(i);
    const std::string pn=getPartitionName(febid);
    sweetc = errsPerFEB[i];
    fill(m_tools[m_histoGroups.at(pn)],sweetc);
  }


  ATH_MSG_VERBOSE( "*Number of errors in Energy Computation : " );
  ATH_MSG_VERBOSE( "*     Low Gain : " << allEC.errors_E[2] << " / " << count_gain[CaloGain::LARLOWGAIN] );
  ATH_MSG_VERBOSE( "*     Medium Gain : " << allEC.errors_E[1] << " / " << count_gain[CaloGain::LARMEDIUMGAIN] );
  ATH_MSG_VERBOSE( "*     High Gain : " <<  allEC.errors_E[0] << " / " << count_gain[CaloGain::LARHIGHGAIN] );
  ATH_MSG_VERBOSE( "*Number of errors in Time Computation : " );
  ATH_MSG_VERBOSE( "*     Low Gain : " <<  allEC.errors_T[2] << " / " << count_gain[CaloGain::LARLOWGAIN] );
  ATH_MSG_VERBOSE( "*     Medium Gain : " <<  allEC.errors_T[1] <<  " / " << count_gain[CaloGain::LARMEDIUMGAIN] );
  ATH_MSG_VERBOSE( "*     High Gain : " <<  allEC.errors_T[0] << " / " << count_gain[CaloGain::LARHIGHGAIN] );
  ATH_MSG_VERBOSE( "*Number of errors in Quality Computation : " );
  ATH_MSG_VERBOSE( "*     Low Gain : " <<  allEC.errors_Q[2] << " / " << count_gain[CaloGain::LARLOWGAIN] );
  ATH_MSG_VERBOSE( "*     Medium Gain : " <<  allEC.errors_Q[1] << " / " << count_gain[CaloGain::LARMEDIUMGAIN] );
  ATH_MSG_VERBOSE( "*     High Gain : " <<  allEC.errors_Q[0] << " / " << count_gain[CaloGain::LARHIGHGAIN] );

  for (unsigned p=0;p<N_PARTITIONS;++p) {
    unsigned allErrsPartE=0;
    unsigned allErrsPartT=0;
    unsigned allErrsPartQ=0;
    partition = p;
    for (unsigned g=0;g<3;++g) {
      gain = g;
      weight_e = (float)errcounters[p].errors_E[g];
      weight_q = (float)errcounters[p].errors_Q[g];
      weight_t = (float)errcounters[p].errors_T[g];
      fill(m_MonGroupName, partition, gain, weight_e, weight_q, weight_t);

      allErrsPartE+=errcounters[p].errors_E[g];
      allErrsPartT+=errcounters[p].errors_T[g];
      allErrsPartQ+=errcounters[p].errors_Q[g];
  } 
  partitionI = p;
  numE = (float)allErrsPartE;
  numT = (float)allErrsPartT;
  numQ = (float)allErrsPartQ;
  fill(m_MonGroupName, lb, partitionI, numE, numT, numQ); 
  }//end loop over partitions

  /*
  for(int str = 0; str < nStreams + 1; str++) {
    if (hasStream[str] == 1) {
      
      m_hEErrors_LB_stream->Fill((float)m_curr_lb,(float)str,(float)allErr_E);
      m_hTErrors_LB_stream->Fill((float)m_curr_lb,(float)str,(float)allErr_T);
      m_hQErrors_LB_stream->Fill((float)m_curr_lb,(float)str,(float)allErr_Q);
      FIXME 
    }
  }
  */

  return StatusCode::SUCCESS;
}



LArRODMonAlg::diff_t LArRODMonAlg::compareChannel(const LArRawChannel& rcDig, 
                                                  const LArRawChannel& rcBS) const {
  diff_t result;                                        
  const HWIdentifier chid=rcDig.channelID();
  const int slot_fD = m_LArOnlineIDHelper->slot(chid);
  const int feedthrough_fD = m_LArOnlineIDHelper->feedthrough(chid);
  const float timeOffline = rcDig.time() - m_timeOffset*m_BC;

  const float& en_fB=rcBS.energy();
  const float& en_fD=rcDig.energy();

  ATH_MSG_VERBOSE(chid.getString()<<" | "<<timeOffline<<" "<<rcBS.time()<<" | "<<en_fB<<" "<<en_fD);

  if (fabs(timeOffline) > m_peakTime_cut*1000.){
    ATH_MSG_DEBUG( " timeOffline too large " << timeOffline );
    return result;
  }

  const std::string & hg=getPartitionName(chid);  
  auto slot = Monitored::Scalar<int>("slot",slot_fD);
  auto ft = Monitored::Scalar<int>("FT",feedthrough_fD);  

  auto pairValueCmp = [](const int& a, const std::pair<int,int>& b){return a<b.first;};
  //Energy check:
  //Find expected precision given the cell-energy:
  auto e_Precision=std::upper_bound(m_E_precision.begin(),m_E_precision.end(),std::abs(en_fB),pairValueCmp);
  const size_t energyrange=e_Precision-m_E_precision.begin();
  auto erange = Monitored::Scalar<int>("Erange",energyrange);
  auto DiffE = Monitored::Scalar<float>("Ediff",en_fD - en_fB);
  fill(m_MonGroupName,DiffE,erange);

  auto Eon = Monitored::Scalar<float>("Eon",en_fB);
  auto Eoff = Monitored::Scalar<float>("Eoff",en_fD);
  fill(m_tools[m_histoGroups.at(hg)],DiffE,erange,Eon,Eoff);
 
  if (std::abs(en_fD-en_fB) > e_Precision->second) {
    //Fill results object for error counting, and dumping (if needed)compRes.e_on!=compRes.e_off)
    result.e_off=en_fD;
    result.e_on=en_fB;
    auto weight_e = Monitored::Scalar<float>("weight_e",1.);
    fill(m_tools[m_histoGroups.at(hg)],slot,ft,weight_e);
  }

  const float q_fB=rcBS.quality();
  const float q_fD=rcDig.quality();
  const float t_fB=rcBS.time();   

  if ((rcDig.provenance() & 0x2000) == 0 || q_fD==0 || t_fB==0 || q_fB==0 || timeOffline==0) {
      // Skip time/Quality comparison if
      // * provenance bits indicate the LArRawChannel builder didn't calculate these quantities
      // * the offline time is zero (may happen if the OFC amplitude < 0.1 )
      // * online value are zero
      // * offline quality is 0 (avoid div-by-zero later on)
      ATH_MSG_VERBOSE("Skip time/Quality comparison, not computed either online or offline");
      return result;
  } 

  auto DiffT = Monitored::Scalar<float>("Tdiff",timeOffline - t_fB);
  //Find expected precision given the cell-time:
  auto t_Precision=std::upper_bound(m_T_precision.begin(),m_T_precision.end(),std::abs(t_fB),pairValueCmp);

  auto Ton = Monitored::Scalar<float>("Ton",t_fB);
  auto Toff = Monitored::Scalar<float>("Toff",timeOffline);
  fill(m_tools[m_histoGroups.at(hg)],DiffT,Ton,Toff);
  fill(m_MonGroupName,DiffT);
  if (fabs(DiffT) > t_Precision->second) {
    auto weight_t = Monitored::Scalar<float>("weight_t",1.);
    fill(m_tools[m_histoGroups.at(hg)],slot,ft,weight_t);
    result.t_on=t_fB;
    result.t_off=timeOffline;
  } 


  //Quality check:
  float qdiff = 65535.0; // max possible quality
  if (q_fD > 0.) qdiff = (q_fD - q_fB)/std::sqrt(q_fD);

  //Find expected precision given the cell-quality:
  auto q_Precision=std::upper_bound(m_Q_precision.begin(),m_Q_precision.end(),std::abs(q_fB),pairValueCmp);

  auto DiffQ = Monitored::Scalar<float>("Qdiff", qdiff);
  auto Qon = Monitored::Scalar<float>("Qon",q_fB);
  auto Qoff = Monitored::Scalar<float>("Qoff",q_fD);
  fill(m_tools[m_histoGroups.at(hg)],DiffQ,Qon,Qoff);
  fill(m_MonGroupName,DiffQ);

  if (fabs(DiffQ) > q_Precision->second) {
    auto weight_q = Monitored::Scalar<float>("weight_q",1.);
    fill(m_tools[m_histoGroups.at(hg)],slot,ft,weight_q);
    result.q_on=q_fB;
    result.q_off=q_fD;
  }
  return result;
}

void LArRODMonAlg::detailedOutput(const LArRODMonAlg::diff_t& cmp,
                                  const LArDigit& dig,  
                                  const EventContext& ctx) const{


  const HWIdentifier chid=dig.channelID();
  const auto gain=dig.gain();

  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,ctx};
  const CaloNoise *noisep = *noiseHdl;

  SG::ReadCondHandle<ILArPedestal>    pedestalHdl{m_keyPedestal, ctx};
  const ILArPedestal* pedestals=*pedestalHdl;

  SG::ReadCondHandle<ILArOFC>         ofcHdl{m_keyOFC, ctx};
  const ILArOFC* ofcs=*ofcHdl;
  const ILArOFC::OFCRef_t& OFCa = ofcs->OFC_a(chid,gain);
  const ILArOFC::OFCRef_t& OFCb = ofcs->OFC_b(chid,gain);
  SG::ReadCondHandle<ILArShape>       shapeHdl{m_keyShape, ctx};

  const ILArShape* shapes=*shapeHdl;
  const ILArShape::ShapeRef_t& shape = shapes->Shape(chid,gain);
  const ILArShape::ShapeRef_t& shapeDer = shapes->ShapeDer(chid,gain);

  SG::ReadCondHandle<ILArHVScaleCorr> hvScaleCorrHdl{m_keyHVScaleCorr, ctx};
  const ILArHVScaleCorr* hvScaleCorrs=*hvScaleCorrHdl;

  SG::ReadCondHandle<LArADC2MeV> adc2MeVHdl{m_adc2mevKey, ctx};
  const LArADC2MeV* adc2mev=*adc2MeVHdl;
  const auto& polynom_adc2mev=adc2mev->ADC2MEV(chid,gain);
  const float escale = (polynom_adc2mev)[1];
  float ramp0 = (polynom_adc2mev)[0];
  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling=*cablingHdl;

  const float hvscale = hvScaleCorrs->HVScaleCorr(chid);
  const int channel=m_LArOnlineIDHelper->channel(chid);
  const HWIdentifier febid=m_LArOnlineIDHelper->feb_Id(chid);
  const std::vector<short>& samples=dig.samples();  
  if (gain == 0) ramp0 = 0.; // no ramp intercepts in HG
  const float ped = pedestals->pedestal(chid,dig.gain());


  //Log-file output if requested ... 
  if (m_ndump<m_max_dump || m_printEnergyErrors) {
    ATH_MSG_WARNING("Channel " << m_LArOnlineIDHelper->channel_name(chid) << ", gain " << gain 
      <<", run " <<   ctx.eventID().run_number() << ", evt " << ctx.eventID().event_number());
    if (cmp.e_on!=cmp.e_off) { 
        ATH_MSG_WARNING("DSP Energy Error:");
        ATH_MSG_WARNING( "   Eonl = " << cmp.e_on << " , Eoff = " << cmp.e_off
                      << " , Eoff - Eonl = " <<  cmp.e_off-cmp.e_on);
    } 
    else {
        ATH_MSG_WARNING("Eonl=Eofl="<< cmp.e_on);
    }
    if(cmp.t_off!=cmp.t_on ) {    
      ATH_MSG_WARNING( "DSP Time Error:");
      ATH_MSG_WARNING( "   Tonl = " << cmp.t_on << " , Toff = " << cmp.t_off
	       		        << " , Toff - Tonl = " << cmp.t_off - cmp.t_on);
    }
    if (cmp.q_off!=cmp.q_on) {
      ATH_MSG_WARNING( "DSP Quality Error");
      ATH_MSG_WARNING( "   Qonl = " << cmp.q_on << " , Qoff = " << cmp.q_off
	     		          << " (Qoff - Qnl)/sqrt(Qoff) = " << (cmp.q_off - cmp.q_on)/std::sqrt(cmp.q_off));
    }
  }
  if (m_ndump<m_max_dump) {
    std::string output;
    output="Digits : ";
    for (const short s : samples) {output+=std::to_string(s)+ " ";}
    ATH_MSG_INFO(output);

    output="OFCa : ";
    for (const auto o : OFCa) {output+=std::to_string(o)+" ";}
    ATH_MSG_INFO(output);

    output="OFCb : ";
    for (const auto o : OFCb) {output+=std::to_string(o)+" ";}
    ATH_MSG_INFO(output);

    output="Shape : ";
    for (const auto s : shape) {output+=std::to_string(s)+" ";}
    ATH_MSG_INFO(output);
    
    output="ShapeDer : ";
    for (const auto s : shapeDer) {output+=std::to_string(s)+" ";}
    ATH_MSG_INFO( output );
    
    ATH_MSG_INFO( "Escale: "<<escale<<" intercept: "<<ramp0<<" pedestal: "<<ped<<" gain: "<<dig.gain() );
    const Identifier cellid=cabling->cnvToIdentifier(chid);
    const float noise=noisep->getNoise(cellid,gain);
    ATH_MSG_INFO( "Noise: "<<noise);
    ATH_MSG_INFO( "HVScaleCorr: "<<hvscale);
    double emon=0.;
    const unsigned nOFCSamp=std::min(samples.size(),OFCa.size());
    for (unsigned k=0; k<nOFCSamp; ++k) emon += (samples.at(k)-ped)*OFCa.at(k);
    emon *= escale;
    emon += ramp0;
    ATH_MSG_INFO( "intercept + Escale*Sum[(sample-ped)*OFCa] "<<emon);
  }//end log-file dump

  if(m_doCellsDump) {
    float sumai = 0.;  
    for (const float a : OFCa) {sumai += a;}
    float pedplusoffset=0;
    if (escale*sumai != 0) pedplusoffset = ped - ramp0/(escale*sumai);
    else pedplusoffset = 0;
    const float inv_Escale = 1. / escale;

    m_fai << channel<<"\t"<<  ped<<"\t"<< pedplusoffset<<"\t"
          << OFCa[0]*escale<<"\t"<<  OFCa[1]*escale<<"\t"<<  OFCa[2]*escale<<"\t"<< OFCa[3]*escale<<"\t"<<  OFCa[4]*escale<<"\t"
         << OFCb[0]*escale<<"\t"<<  OFCb[1]*escale<<"\t"<<  OFCb[2]*escale<<"\t"<< OFCb[3]*escale<<"\t"<<  OFCb[4]*escale<<"\t"
         << shape[0]*inv_Escale<<"\t"<<  shape[1]*inv_Escale<<"\t"<<  shape[2]*inv_Escale<<"\t"<<  shape[3]*inv_Escale<<"\t"<<  shape[4]*inv_Escale<<"\t"
         << shapeDer[0]*inv_Escale<<"\t"<<  shapeDer[1]*inv_Escale<<"\t"<<  shapeDer[2]*inv_Escale<<"\t"<< shape[3]*inv_Escale<<"\t"<<  shapeDer[4]*inv_Escale << std::endl; 

    // Dumping file with offline energy and online energ
    m_fen << m_ndump << "   "  << cmp.e_off << "   " << cmp.e_on ;
    m_fen << "     // FEB " << febid.get_identifier32().get_compact() << " ( channel " << channel << " ), event " << ctx.eventID().event_number() << std::endl;
 
    // Dumping file with digits
    m_fdig << channel << "   ";
    for (const short d : samples) {
     m_fdig << d << " ";
    }
    m_fdig << "     // FEB " << febid.get_identifier32().get_compact() << " ( channel " << channel << " ), event " << ctx.eventID().event_number() << std::endl;
  } //end if m_doCellsDump
  return;
}

void LArRODMonAlg::dumpCellInfo(const HWIdentifier chid,
		                            const int gain,
                                const EventContext& ctx,
                                const diff_t & cmp) const {
  
  // Dumping cell information in a text file so that it can be compared to unhappy channels lists for instance ...

  const int barrel_ec=m_LArOnlineIDHelper->barrel_ec(chid);
  const int posneg=m_LArOnlineIDHelper->pos_neg(chid);
  const int slot = m_LArOnlineIDHelper->slot(chid);
  const int FT = m_LArOnlineIDHelper->feedthrough(chid);
  const int channel =  m_LArOnlineIDHelper->channel(chid);
  const HWIdentifier febid= m_LArOnlineIDHelper->feb_Id(chid);
  m_fdump << "0x" << std::hex << std::setw(8)<<febid.get_identifier32().get_compact() << " " << std::dec << std::setw(3) << std::right << channel << " 0x" << std::hex << std::setw(8)<<chid.get_identifier32().get_compact() 
          <<std::dec << std::setw(3) << std::right << slot << std::setw(3) << std::right << FT << std::setw(3) << std::right << barrel_ec << std::setw(3) << std::right<< posneg << std::setw(6) << std::right << getPartitionName(chid) 
          << " " << gain << " " << " " << cmp.e_off << " "<< cmp.e_on << " "<<cmp.t_off << " "<<cmp.t_on <<" "<<cmp.q_off << " "<<cmp.q_on <<ctx.eventID().event_number()<<std::endl;
  return;          
}

void LArRODMonAlg::ERRCOUNTER::clear() {
  errors_E.fill(0);
  errors_T.fill(0);
  errors_Q.fill(0);
  return;
}

StatusCode LArRODMonAlg::finalize() {
  m_fai.close();
  m_fdig.close();
  m_fen.close();
  m_fdump.close();
  return StatusCode::SUCCESS;
}