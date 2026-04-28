/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArNoise2Ntuple.h"
#include "LArRawConditions/LArNoiseComplete.h"
#include "LArRawConditions/LArNoiseMC.h"
#include "CaloIdentifier/CaloGain.h"


LArNoise2Ntuple::LArNoise2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator) { 
  declareProperty("ContainerKey",m_contKey);

  m_ntTitle="Noise";
  m_ntpath="/NTUPLES/FILE1/NOISE";

}

LArNoise2Ntuple::~LArNoise2Ntuple() 
= default;

StatusCode LArNoise2Ntuple::stop() {
  const ILArNoise* larNoise = nullptr;
  StatusCode sc;
  sc=m_detStore->retrieve(larNoise,m_contKey);
  if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "Unable to retrieve ILArNoise with key " 
               << m_contKey << " from DetectorStore" );
     return StatusCode::FAILURE;
  }

 NTuple::Item<long> cellIndex,gain;
 NTuple::Item<float> noise;

 sc=m_nt->addItem("icell",cellIndex,0,2000);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
    return StatusCode::FAILURE;
   }

 sc=m_nt->addItem("gain",gain,0,3);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'gain' failed" );
    return StatusCode::FAILURE;
   }


 sc=m_nt->addItem("noise",noise);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'noise' failed" );
    return StatusCode::FAILURE;
   }

 SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
 const LArOnOffIdMapping* cabling=*cablingHdl;
 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 unsigned cellCounter=0;
 for(long igain=CaloGain::LARHIGHGAIN; igain<m_NGains.value(); igain++) {
  for (const HWIdentifier hwid: m_onlineId->channel_range()) {
     if ( cabling->isOnlineConnected(hwid)) {
	 fillFromIdentifier(hwid);       
	 cellIndex = cellCounter;
         gain=igain;     
	 noise = larNoise->noise(hwid,igain);
	 sc=ntupleSvc()->writeRecord(m_nt);
	 if (sc!=StatusCode::SUCCESS) {
	   ATH_MSG_ERROR( "writeRecord failed" );
	   return StatusCode::FAILURE;
	 }
     }//end if isConnected
     cellCounter++;
  }//end loop over online ID
 } // ovr gains

 ATH_MSG_INFO( "LArNoise2Ntuple has finished." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
