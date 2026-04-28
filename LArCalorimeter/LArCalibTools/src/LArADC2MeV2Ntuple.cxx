/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArADC2MeV2Ntuple.h"

StatusCode LArADC2MeV2Ntuple::initialize() {
  m_ntTitle="ADC2MeV";
  m_ntName="ADC2MeV";
  m_ntpath=std::string("/NTUPLES/FILE1/")+m_ntName;

  ATH_CHECK(m_adc2MeVKey.initialize());
  return LArCond2NtupleBase::initialize();
}



StatusCode LArADC2MeV2Ntuple::stop() {

  NTuple::Array<float> coeffs;
  NTuple::Item<unsigned long> coeffIndex;
  NTuple::Item<unsigned long> gain;
  SG::ReadCondHandle<LArADC2MeV> adc2MeVHdl{m_adc2MeVKey};
  const LArADC2MeV* adc2MEV{*adc2MeVHdl};
   
  ATH_CHECK(m_nt->addItem("Xi",coeffIndex,0,3));   
  ATH_CHECK(m_nt->addItem("X",coeffIndex,coeffs));
  ATH_CHECK(m_nt->addItem("gain",gain,0,3)); 
  for(long igain=CaloGain::LARHIGHGAIN; igain<m_NGains; igain++) {
     for (const HWIdentifier hwid: m_onlineId->channel_range()) {
          auto adc2mevCoeff=adc2MEV->ADC2MEV(hwid,igain);
          if (adc2mevCoeff.size()>0) {
            fillFromIdentifier(hwid);
            gain=igain;
            for (coeffIndex=0;coeffIndex<adc2mevCoeff.size();coeffIndex++) {
                coeffs[coeffIndex]=adc2mevCoeff[coeffIndex];
            }        
            ATH_CHECK(ntupleSvc()->writeRecord(m_nt));
          } //end if have ADC2MeV values for this cell and gain
       }//end loop over cells
     }//end loop over gians

 ATH_MSG_INFO( "LArADC2MeV2Ntuple has finished." );
 return StatusCode::SUCCESS;

} 
