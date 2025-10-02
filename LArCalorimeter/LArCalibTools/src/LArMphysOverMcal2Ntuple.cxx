/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArMphysOverMcal2Ntuple.h"
#include "CaloIdentifier/CaloGain.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"


LArMphysOverMcal2Ntuple::LArMphysOverMcal2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator) { 
  m_ntTitle="MphysOverMcal";
  m_ntpath="/NTUPLES/FILE1/MPMC";

}

LArMphysOverMcal2Ntuple::~LArMphysOverMcal2Ntuple() 
= default;

StatusCode LArMphysOverMcal2Ntuple::initialize() {
  ATH_CHECK(m_contKey.initialize());
  return LArCond2NtupleBase::initialize();
}


StatusCode LArMphysOverMcal2Ntuple::stop() {

  // For compatibility with existing configurations, look in the detector
  // store first, then in conditions.
  const ILArMphysOverMcal* larMphysOverMcal= detStore()->tryConstRetrieve<ILArMphysOverMcal>(m_contKey.key());
  if (!larMphysOverMcal) {
    SG::ReadCondHandle<ILArMphysOverMcal> mpmcHandle{m_contKey};
    larMphysOverMcal=*mpmcHandle;
  }

  if (!larMphysOverMcal) {
    ATH_MSG_ERROR( "Unable to retrieve ILArMphysOverMcal with key " 
		   << m_contKey.key() << " from DetectorStore nor from ConditionsStore");
    return StatusCode::FAILURE;
  } 

  

  StatusCode sc;
  NTuple::Item<long> cellIndex,gain;
  NTuple::Item<float> mpmc;

 sc=m_nt->addItem("icell",cellIndex,0,2000);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
    return StatusCode::FAILURE;
   }

 sc=m_nt->addItem("gain",gain,0,3);
 if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'gain' failed" );
    return StatusCode::FAILURE;
 }


 sc=m_nt->addItem("mphysovermcal",mpmc,-1000.,2.);
 if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'mphysovermcal' failed" );
    return StatusCode::FAILURE;
 }


 const LArOnOffIdMapping *cabling=nullptr;
 if(m_isSC) {
   ATH_MSG_DEBUG( "LArMphysOverMcal2Ntuple: using SC cabling" );
   SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingSCKey};
   cabling=*cablingHdl;
 }else{
   ATH_MSG_DEBUG( "LArMphysOverMcal2Ntuple: using LAr cell cabling" );
   SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
   cabling=*cablingHdl;
 }
 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 //=================

 if(m_isSC){
   const LArOnline_SuperCellID* ll;
   sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
   if (sc.isFailure()) {
     msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
     return StatusCode::FAILURE;
   }
   else {
     m_onlineId = static_cast<const LArOnlineID_Base*>(ll);
     ATH_MSG_DEBUG("Found the SC LArOnlineID helper");
   }
 }else{
   const LArOnlineID* ll;
   sc = detStore()->retrieve(ll, "LArOnlineID");
   if (sc.isFailure()) {
     msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
     return StatusCode::FAILURE;
   }
   else {
     m_onlineId = static_cast<const LArOnlineID_Base*>(ll);
     ATH_MSG_DEBUG(" Found the LAr cell LArOnlineID helper. ");
   }
   
 }

 // ==============

 unsigned cellCounter=0;
 unsigned filledCell=0;
 unsigned nGain = m_isSC ? 1 : CaloGain::LARNGAIN;
 for(unsigned igain=CaloGain::LARHIGHGAIN; igain<nGain; igain++) {
   for (const HWIdentifier hwid: m_onlineId->channel_range()) {
     if ( cabling->isOnlineConnected(hwid) && !m_onlineId->isFCALchannel(hwid)) {
	 fillFromIdentifier(hwid);       
	 cellIndex = cellCounter;
         gain=igain;     
	 mpmc = larMphysOverMcal->MphysOverMcal(hwid,igain);
	 sc=ntupleSvc()->writeRecord(m_nt);
	 if (sc!=StatusCode::SUCCESS) {
	   ATH_MSG_ERROR( "writeRecord failed" );
	   return StatusCode::FAILURE;
	 }
	 filledCell++;
     }//end if isConnected
     cellCounter++;
  }//end loop over online ID
 } // ovr gains
 ATH_MSG_INFO("LArMphysOverMcal2Ntuple: filled "<<filledCell<<" out of "<<cellCounter);
 ATH_MSG_INFO( "LArMphysOverMcal2Ntuple has finished." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
