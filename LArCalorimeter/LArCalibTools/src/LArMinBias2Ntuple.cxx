/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArMinBias2Ntuple.h"
#include "CaloIdentifier/CaloGain.h"


LArMinBias2Ntuple::LArMinBias2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator),
  m_isPileup(false) { 

  declareProperty("NtupleTitle",m_ntTitle="MinBias");
  declareProperty("NtupleName",m_ntpath="/NTUPLES/FILE1/MINBIAS");

}

LArMinBias2Ntuple::~LArMinBias2Ntuple() 
= default;

StatusCode LArMinBias2Ntuple::initialize() {
  m_isPileup = m_contKey.key().empty() && (m_contKeyAv.key().find("Pileup") != std::string::npos);
  ATH_CHECK(m_contKey.initialize(!m_isPileup));
  ATH_CHECK(m_contKeyAv.initialize());
  return LArCond2NtupleBase::initialize();
}

StatusCode LArMinBias2Ntuple::stop() {
   
  const ILArMinBias* LArMinBias = nullptr;
  const ILArMinBiasAverage* LArMinBiasAv = nullptr;
  
  if(!m_isPileup) {
     LArMinBias=m_detStore->tryConstRetrieve<ILArMinBias>(m_contKey.key());
     if (!LArMinBias) {
        SG::ReadCondHandle<ILArMinBias> mbHandle{m_contKey};
        LArMinBias=*mbHandle;
     }
  }

  LArMinBiasAv=m_detStore->tryConstRetrieve<ILArMinBiasAverage>(m_contKeyAv.key());
  if(!LArMinBiasAv) {
    SG::ReadCondHandle<ILArMinBiasAverage> mbaHandle{m_contKeyAv};
    LArMinBiasAv=*mbaHandle;
  }

 NTuple::Item<float> minbias;
 NTuple::Item<float> minbias_av;

 SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
 const LArOnOffIdMapping* cabling=*cablingHdl;
 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 if(!m_isPileup) ATH_CHECK( m_nt->addItem("MinBias",minbias) );
 ATH_CHECK( m_nt->addItem("MinBiasAv",minbias_av) );

 unsigned cellCounter=0;
 for (const HWIdentifier hwid: m_onlineId->channel_range()) {
   if ( cabling->isOnlineConnected(hwid)) {
     fillFromIdentifier(hwid);       
     if(!m_isPileup) minbias = LArMinBias->minBiasRMS(hwid);
     minbias_av = LArMinBiasAv->minBiasAverage(hwid);
     ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
   }//end if isConnected
   cellCounter++;
 }//end loop over online ID

 ATH_MSG_INFO(  "LArMinBias2Ntuple has finished, " << cellCounter << " cells written." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
