/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LAruA2MeV2Ntuple.h"
#include "LArElecCalib/ILAruA2MeV.h"
#include "LArElecCalib/ILArDAC2uA.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArIdentifier/LArOnlineID.h"
#include "StoreGate/StoreGateSvc.h"

LAruA2MeV2Ntuple::LAruA2MeV2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): LArCond2NtupleBase(name, pSvcLocator) { 
  m_ntTitle="ADC to Mev Conversion";
  m_ntpath="/NTUPLES/FILE1/ADCMEV";

}

StatusCode LAruA2MeV2Ntuple::initialize() {
  ATH_CHECK(m_uA2MeVKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_DAC2uAKey.initialize(SG::AllowEmpty));

  if (m_uA2MeVKey.empty() and m_DAC2uAKey.empty()) {
    ATH_MSG_ERROR("Configuration problem: Both uA2MeVKey and DACuAKey are emtpy");
    return StatusCode::FAILURE;
  }
  return LArCond2NtupleBase::initialize();
}
  
LAruA2MeV2Ntuple::~LAruA2MeV2Ntuple() 
= default;

StatusCode LAruA2MeV2Ntuple::stop() {
  StatusCode sc;
  NTuple::Item<long> cellIndex;
  NTuple::Item<float> uA2MeV;
  NTuple::Item<float> DAC2uA;
 
  sc=m_nt->addItem("icell",cellIndex,0,2000);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
    return StatusCode::FAILURE;
  }

  if (!m_uA2MeVKey.empty()) {  
    sc=m_nt->addItem("uAMeV",uA2MeV,-1000.,5000.);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'uAMeV' failed" );
      return StatusCode::FAILURE;
    }  
  }

  if (!m_DAC2uAKey.empty()) {
    sc=m_nt->addItem("DAC2uA",DAC2uA,-1000.,5000.);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'DAC2uA' failed" );
      return StatusCode::FAILURE;
    }  
  }

  // For compatibility with existing configurations, look in the detector
  // store first, then in conditions store. 
  const ILAruA2MeV* laruA2MeVComplete=nullptr;
  if (!m_uA2MeVKey.empty()) {
    laruA2MeVComplete=detStore()->tryRetrieve<ILAruA2MeV>(m_uA2MeVKey.key());
    if (!laruA2MeVComplete) {
      SG::ReadCondHandle<ILAruA2MeV> ua2MeVHdl{m_uA2MeVKey};
      laruA2MeVComplete=*ua2MeVHdl;
    }  
    if (!laruA2MeVComplete) {
      ATH_MSG_ERROR("Failed to retrieve ILAruA2MeV with key " 
		    << m_uA2MeVKey.key() << "from DetectorStore nor from ConditonsStore");
      return StatusCode::FAILURE;
    }
  }  

  const ILArDAC2uA* larDAC2uAComplete=nullptr;
  if (!m_DAC2uAKey.empty()) {
    larDAC2uAComplete=detStore()->tryRetrieve<ILArDAC2uA>(m_DAC2uAKey.key());
    if (!larDAC2uAComplete) {
      SG::ReadCondHandle<ILArDAC2uA> DAC2uAHdl{m_DAC2uAKey};
      larDAC2uAComplete=*DAC2uAHdl;
    }  
    if (!larDAC2uAComplete) {
      ATH_MSG_ERROR("Failed to retrieve ILArDAC2uA with key " 
		    << m_DAC2uAKey.key() << "from DetectorStore nor from ConditonsStore");
      return StatusCode::FAILURE;
    }  
  }

  const LArOnOffIdMapping* cabling=nullptr;
  if(m_isSC) {
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingSCKey};
     cabling=*cablingHdl;
  } else {
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
     cabling=*cablingHdl;
  }
  if(!cabling) {
    ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
    return StatusCode::FAILURE;
  }

  unsigned cellCounter=0;
  for (const HWIdentifier hwid: m_onlineId->channel_range()) {
    if (cabling->isOnlineConnected(hwid)) {
      if (laruA2MeVComplete) uA2MeV=laruA2MeVComplete->UA2MEV(hwid);
      if (larDAC2uAComplete) DAC2uA=larDAC2uAComplete->DAC2UA(hwid);
      fillFromIdentifier(hwid);
      cellIndex=cellCounter;
      sc=ntupleSvc()->writeRecord(m_nt);

      if (sc!=StatusCode::SUCCESS) {
	ATH_MSG_ERROR( "writeRecord failed" );
	return StatusCode::FAILURE;
      }
      cellCounter++;
    }//end if connected
  }//end loop over online ID

  ATH_MSG_INFO( "LAruA2MeV2Ntuple has finished." );
  return StatusCode::SUCCESS;
}// end finalize-method.
   
