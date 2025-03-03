/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "LArNoisyROAlg.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"
#include "Identifier/Identifier.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArIdentifier/LArOnlineID.h" 
#include "LArRecEvent/LArEventBitInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/WriteDecorHandle.h"

#include <cmath>

LArNoisyROAlg::LArNoisyROAlg(const std::string &name,ISvcLocator *pSvcLocator):
  AthReentrantAlgorithm (name, pSvcLocator), 
  m_noisyROTool("LArNoisyROTool",this)
{
  declareProperty( "Tool",m_noisyROTool);
}

StatusCode LArNoisyROAlg::initialize() {
  ATH_CHECK(m_noisyROTool.retrieve());
  ATH_CHECK(m_CaloCellContainerName.initialize());
  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_eventInfoDecorKey.initialize());
  ATH_CHECK(m_knownBadFEBsVecKey.initialize(!m_isMC) );
  ATH_CHECK(m_knownMNBFEBsVecKey.initialize(!m_isMC) );
  ATH_CHECK(m_hvMapKey.initialize(!(m_isMC || m_hvMapKey.empty())));
  ATH_CHECK(m_caloDetDescrMgrKey.initialize(!m_isMC));
  ATH_CHECK(m_hvCablingKey.initialize(!( m_isMC || m_hvMapKey.empty())));

  return StatusCode::SUCCESS;
}

StatusCode LArNoisyROAlg::execute (const EventContext& ctx) const
{
  SG::ReadHandle<CaloCellContainer> cellContainer(m_CaloCellContainerName, ctx);
  if (!cellContainer.isValid()) { 
    ATH_MSG_WARNING( " Can not retrieve CaloCellContainer: " << m_CaloCellContainerName.key());
    ATH_MSG_WARNING( " Empty object recorded !");

    const std::set<unsigned int> knownBadFEBs;
    const std::vector<HWIdentifier> knownMNBFEBs;
    SG::WriteHandle<LArNoisyROSummary> noisyRO(m_outputKey, ctx);
    ATH_CHECK(noisyRO.record(m_noisyROTool->process(ctx,nullptr, &knownBadFEBs, &knownMNBFEBs, nullptr, nullptr, nullptr)));

    return StatusCode::SUCCESS;      
  } 

  const EventIDBase& EIHandle = ctx.eventID();
  long int thisTimeStamp    = (EIHandle).time_stamp();
  long int thisTimeStampns  = (EIHandle).time_stamp_ns_offset();
  uint32_t thisLB           = (EIHandle).lumi_block();
  unsigned long long thisEv = (EIHandle).event_number();
  ATH_MSG_DEBUG ( name() << " processing EN : " << thisEv << " in LB : " << thisLB << " TS : " << thisTimeStamp << " TSNS : " << thisTimeStampns );
  
  std::set<unsigned int> bf;
  std::vector<HWIdentifier> MNBfeb;
  const LArHVNMap* hvmap=nullptr;
  const CaloDetDescrManager* cddm=nullptr;
  const LArHVIdMapping* hvid=nullptr;
  if (! m_isMC) {
    SG::ReadCondHandle<LArBadFebCont> badHdl(m_knownBadFEBsVecKey, ctx);
    const LArBadFebCont* badCont=*badHdl;
    if(badCont) {
      for(LArBadFebCont::BadChanVec::const_iterator i = badCont->begin(); i!=badCont->end(); ++i) {
        bf.insert(i->first);
      }
    }
  
    SG::ReadCondHandle<LArBadFebCont> MNBHdl(m_knownMNBFEBsVecKey, ctx);
    const LArBadFebCont* MNBCont=*MNBHdl;
    if(MNBCont) {
      for(LArBadFebCont::BadChanVec::const_iterator i = MNBCont->begin(); i!=MNBCont->end(); ++i) {
        MNBfeb.emplace_back(i->first);
      } 
    }

    if(!m_hvMapKey.empty()) {
       SG::ReadCondHandle<LArHVNMap> hvMapHdl(m_hvMapKey, ctx);
       if(!hvMapHdl.isValid()) {
          ATH_MSG_WARNING( " Can not retrieve HVline nCells: " << m_hvMapKey.key());
          ATH_MSG_WARNING( " Will not flag HV lines noise !");
       } else {
          hvmap = *hvMapHdl;
       }
    }

    SG::ReadCondHandle<CaloDetDescrManager> cddmHdl(m_caloDetDescrMgrKey, ctx);
    if(!cddmHdl.isValid()) {
       ATH_MSG_WARNING( " Can not retrieve CaloDetDesrManager: " << m_caloDetDescrMgrKey.key());
       ATH_MSG_WARNING( " Will not flag HV lines noise !");
    } else {
       cddm = *cddmHdl;
    }

    if(!m_hvCablingKey.empty()){
       SG::ReadCondHandle<LArHVIdMapping> hvidHdl(m_hvCablingKey, ctx);
       if(!hvidHdl.isValid()) {
          ATH_MSG_WARNING( " Can not retrieve LArHVIdMapping: " << m_hvCablingKey.key());
          ATH_MSG_WARNING( " Will not flag HV lines noise !");
       } else {
          hvid = *hvidHdl;
       }
    }
  }

  ATH_MSG_DEBUG("Number of known Bad FEBs: "<<bf.size());
  ATH_MSG_DEBUG("Number of known MNB FEBs: "<<MNBfeb.size());



  SG::WriteHandle<LArNoisyROSummary> noisyRO(m_outputKey, ctx);
  ATH_CHECK(noisyRO.record(m_noisyROTool->process(ctx,cellContainer.cptr(), &bf, &MNBfeb, hvmap, cddm, hvid)));
  ATH_MSG_DEBUG("Recorded LArNoisyROSummary with key: "<<m_outputKey.key());


  bool badFEBFlag=noisyRO->BadFEBFlaggedPartitions();
  bool badFEBFlag_W=noisyRO->BadFEB_WFlaggedPartitions();
  bool badSaturatedTightCut=noisyRO->SatTightFlaggedPartitions();
  bool MNBLooseCut=noisyRO->MNBLooseFlaggedPartitions();
  bool MNBTightCut=noisyRO->MNBTightFlaggedPartitions();
  bool MNBTight_PsVetoCut=noisyRO->MNBTight_PsVetoFlaggedPartitions();
  bool badHVlinesFlag=noisyRO->HVlineFlaggedPartitions();
  
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
  if ( badFEBFlag || badFEBFlag_W || badSaturatedTightCut || MNBLooseCut || MNBTightCut || MNBTight_PsVetoCut || badHVlinesFlag ) 
  {
    // retrieve EventInfo
    bool failSetWARN=false;
    bool failSetWARNREASON=false;
    // set warning flag except if the error flag has been already set
    if ( eventInfo->errorState(xAOD::EventInfo::LAr) != xAOD::EventInfo::Error) {
      if ( badFEBFlag ) {
	failSetWARN |= (!eventInfo->updateErrorState(xAOD::EventInfo::LAr,xAOD::EventInfo::Warning));
	failSetWARNREASON |= (!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS));
      }//endif badFEBFlag

      if ( badFEBFlag_W ) {
	//Set WARNING Flag
	failSetWARN |=(!eventInfo->updateErrorState(xAOD::EventInfo::LAr,xAOD::EventInfo::Warning));
	// Set reason why event was flagged
	failSetWARNREASON |=(!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS_W));
      }// end if badFEBFlag_W

      if ( badSaturatedTightCut ){
	failSetWARNREASON |= (!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ));
      }
    }

    if ( MNBTightCut ) {
      failSetWARN |=(!eventInfo->updateErrorState(xAOD::EventInfo::LAr,xAOD::EventInfo::Warning));
      // Set reason why event was flagged
      failSetWARNREASON |=(!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::MININOISEBURSTTIGHT));
    }

    if ( MNBTight_PsVetoCut ) {
      failSetWARN |=(!eventInfo->updateErrorState(xAOD::EventInfo::LAr,xAOD::EventInfo::Warning));
      // Set reason why event was flagged
      failSetWARNREASON |=(!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::MININOISEBURSTTIGHT_PSVETO));
    }

    if ( MNBLooseCut ) { //FIXME Tight cut actually implies loose cut too
      failSetWARNREASON |=(!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::MININOISEBURSTLOOSE));
    }

    if ( badHVlinesFlag ) {
      failSetWARN |= (!eventInfo->updateErrorState(xAOD::EventInfo::LAr,xAOD::EventInfo::Warning));
      failSetWARNREASON |= (!eventInfo->updateEventFlagBit(xAOD::EventInfo::LAr,LArEventBitInfo::BADHVLINES));
    }//endif badFEBFlag

    if (failSetWARN) ATH_MSG_WARNING( "Failure during EventInfo::setEventErrorState(EventInfo::LAR,EventInfo::WARNING)"  );
    if (failSetWARNREASON) ATH_MSG_WARNING( "Failure during setEventFlagBit(EventInfo::LAr,...)"  );
  
  }
  return StatusCode::SUCCESS;
}

StatusCode LArNoisyROAlg::finalize()
{  
  return StatusCode::SUCCESS;
}

