/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawChannelBuilderSCAlg.h" 
#include "GaudiKernel/SystemOfUnits.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "AthAllocators/DataPool.h"
#include "LArElecCalib/LArProvenance.h"
#include <cmath>

  
StatusCode LArRawChannelBuilderSCAlg::initialize() {
  ATH_CHECK(m_digitKey.initialize());	 
  ATH_CHECK(m_cellKey.initialize());
  ATH_CHECK(m_pedestalKey.initialize());	 
  ATH_CHECK(m_adc2MeVKey.initialize());	 
  ATH_CHECK(m_ofcKey.initialize());	 
  ATH_CHECK(m_shapeKey.initialize());
  ATH_CHECK(m_cablingKey.initialize() );
  ATH_CHECK( m_bcContKey.initialize(SG::AllowEmpty) );
 
  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnline_SuperCellID"));
  ATH_CHECK(m_caloSuperCellMgrKey.initialize());  

  return StatusCode::SUCCESS;
}     

StatusCode LArRawChannelBuilderSCAlg::finalize() {
  return StatusCode::SUCCESS;
} 

StatusCode LArRawChannelBuilderSCAlg::execute(const EventContext& ctx) const {

  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer> inputContainer(m_digitKey,ctx);

  //Write output via write handle
  auto outputContainerCellPtr = std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS);

  DataPool<CaloCell> dataPool(ctx);
  unsigned int hash_max = m_onlineId->channelHashMax();
  if (dataPool.allocated()==0){
      dataPool.reserve (hash_max);
  }
  outputContainerCellPtr->reserve( hash_max );
  
	    
  //Get Conditions input
  SG::ReadCondHandle<ILArPedestal> pedHdl(m_pedestalKey,ctx);
  const ILArPedestal* peds=*pedHdl;

  SG::ReadCondHandle<LArADC2MeV> adc2mevHdl(m_adc2MeVKey,ctx);
  const LArADC2MeV* adc2MeVs=*adc2mevHdl;

  SG::ReadCondHandle<ILArOFC> ofcHdl(m_ofcKey,ctx);
  const ILArOFC* ofcs=*ofcHdl;

  SG::ReadCondHandle<ILArShape> shapeHdl(m_shapeKey,ctx);
  const ILArShape* shapes=*shapeHdl;

  SG::ReadCondHandle<LArOnOffIdMapping> cabling(m_cablingKey,ctx);

  SG::ReadCondHandle<CaloSuperCellDetDescrManager> caloSuperCellMgrHandle{m_caloSuperCellMgrKey,ctx};
  const CaloSuperCellDetDescrManager* caloMgr = *caloSuperCellMgrHandle;

  const LArBadChannelCont* badchannel(nullptr);
  if( !m_bcContKey.empty() ){
     SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
     badchannel = *larBadChan;
  }

  //Loop over digits:
  for (const LArDigit* digit : *inputContainer) {

    const HWIdentifier id=digit->hardwareID();
    if (!(*cabling)->isOnlineConnected(id)) continue;
    
    ATH_MSG_VERBOSE("Working on channel " << m_onlineId->channel_name(id));

    const std::vector<short>& samples=digit->samples();
    const size_t nSamples=samples.size();
    const int gain=digit->gain();
    const float p=peds->pedestal(id,gain);
   
    //The following autos will resolve either into vectors or vector-proxies
    const auto& ofca=ofcs->OFC_a(id,gain);
    const auto& adc2mev=adc2MeVs->ADC2MEV(id,gain);
    
    //Sanity check on input conditions data:
    if (ATH_UNLIKELY(p==ILArPedestal::ERRORCODE)) {
      ATH_MSG_ERROR("No valid pedestal for channel " << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    if(ATH_UNLIKELY(adc2mev.size()<2)) { 
        ATH_MSG_ERROR("No valid ADC2MeV for channel " << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    // Subtract pedestal
    std::vector<float> samp_no_ped(nSamples,0.0);
    for (size_t i=0;i<nSamples;++i) {
      samp_no_ped[i]=samples[i]-p;
    }

    //Apply OFCs to get amplitude
    // Evaluate sums in double-precision to get consistent results
    // across platforms.
    double A=0;
    bool passBCIDmax=false;
    //const size_t len=std::min(ofca.size(),samples.size());
    size_t nOFC=ofca.size();
    if (ATH_UNLIKELY(nSamples<nOFC+2)) {
        ATH_MSG_ERROR("Not enough ADC samples for channel " << m_onlineId->channel_name(id) << " gain " << gain
                << ". Found " << nSamples << ", expect at least " << nOFC+2 <<".");
    }   
    //Calculate Amplitude for BC
    for (size_t i=0;i<nOFC;++i) {
       A+=static_cast<double>(samp_no_ped[i+1])*ofca[i];
    }
    //Calcuclate Amplitude for preceeding BC
    double Abefore=0.;
    for (size_t i=0;i<nOFC;++i) {
       Abefore+=static_cast<double>(samp_no_ped[i])*ofca[i];
    }
    //Calculate Amplitude for trailing BC
    double Aafter=0.;
    for (size_t i=0;i<nOFC;++i) {
      Aafter+=static_cast<double>(samp_no_ped[i+2])*ofca[i];
    }
    //set passBCIDmax if Amplitude at assume BC is larger than for the BC before and after
    if ( (A>Abefore) && (A>Aafter) ) passBCIDmax=true;
    
    
    //Apply Ramp
    const float E=adc2mev[0]+A*adc2mev[1];
    
    uint16_t iquaShort=0;
    float tau=0;

    const float E1=m_absECutFortQ.value() ? std::fabs(E) : E;
    
    if (E1 > m_eCutFortQ) {
      ATH_MSG_VERBOSE("Channel " << m_onlineId->channel_name(id) << " gain " << gain << " above threshold for tQ computation");

      //Get time by applying OFC-b coefficients:
      const auto& ofcb=ofcs->OFC_b(id,gain);
      double At=0;
      for (size_t i=0;i<nOFC;++i) {
	At+=static_cast<double>(samp_no_ped[i+1])*ofcb[i];
      }
      //Divide A*t/A to get time
      tau=(std::fabs(A)>0.1) ? At/A : 0.0;
      const auto& fullShape=shapes->Shape(id,gain);
      
      //Get Q-factor
      size_t firstSample=m_firstSample;
      // fixing HEC to move +1 in case of 4 samples and firstSample 0 (copied from old LArRawChannelBuilder)
      if (fullShape.size()>nSamples && nSamples==4 && m_firstSample==0) {
	if (m_onlineId->isHECchannel(id)) {
	  firstSample=1;
	}
      }

      if (ATH_UNLIKELY(fullShape.size()<nOFC+firstSample)) {
        ATH_MSG_DEBUG("No valid shape for channel " <<  m_onlineId->channel_name(id) 
                << " gain " << gain); 
        ATH_MSG_DEBUG("Got size " << fullShape.size() << ", expected at least " << nSamples+firstSample);
        //return StatusCode::FAILURE;
        nOFC=fullShape.size()-firstSample;
      }

      const float* shape=&*fullShape.begin()+firstSample;

      double q=0;
      bool useShapeDer=m_useShapeDer; 
      if (useShapeDer) {
	const auto& fullshapeDer=shapes->ShapeDer(id,gain);
	if (ATH_UNLIKELY(fullshapeDer.size()<nOFC)) {
	  ATH_MSG_DEBUG("No valid shape derivative for channel " <<  m_onlineId->channel_name(id) 
			<< " gain " << gain << ". Will not use shape derivative.");
	  useShapeDer=false;
	}
	if (useShapeDer) {
	  const float* shapeDer=&*fullshapeDer.begin()+firstSample;
	  for (size_t i=0;i<nOFC;++i) {
	    q += std::pow((A*(shape[i]-tau*shapeDer[i])-(samp_no_ped[i+1])),2);
	  }
	}//end if useShapeDer
      }
      if (!useShapeDer){
	//Q-factor w/o shape derivative
	for (size_t i=0;i<nOFC;++i) {
	  q += std::pow((A*shape[i]-(samp_no_ped[i+1])),2);
	}
      }

      int iqua = static_cast<int>(q);
      if (iqua > 0xFFFF) iqua=0xFFFF;
      iquaShort = static_cast<uint16_t>(iqua & 0xFFFF);

      tau-=ofcs->timeOffset(id,gain);
    }//end if above cut

    CaloCell* ss = dataPool.nextElementPtr();
    Identifier offId = cabling->cnvToIdentifier(id);
    
    const CaloDetDescrElement* dde = caloMgr->get_element (offId);
    ss->setCaloDDE(dde);
    ss->setEnergy(E);
    ss->setTime(tau);
    ss->setGain((CaloGain::CaloGain)0);
    float et = ss->et()*1e-3; // et in GeV
    // for super-cells provenance and time are slightly different
    uint16_t prov = LArProv::QTPRESENT;//0x2000;
    if(et>10e3 && tau>-8 && tau<16) prov |= LArProv::SCTIMEPASS; //0x200;
    else if(et<=10e3 && std::fabs(tau)<8) prov |= LArProv::SCTIMEPASS; //0x200; 
    if ( passBCIDmax ) prov |= LArProv::SCPASSBCIDMAX; //0x40;
    // set some provenance to indicate bad channel
    if(badchannel) {
       LArBadChannel bc = badchannel->offlineStatus(offId);
       if ( !bc.good() && bc.statusBad(LArBadChannel::LArBadChannelSCEnum::maskedOSUMBit) ){
         prov |= 0x80;
       }
    }

    ss->setProvenance(prov);
    
    ss->setQuality(iquaShort);
    outputContainerCellPtr->push_back(ss);

  }//end loop over input digits  

  SG::WriteHandle<CaloCellContainer>outputContainer(m_cellKey,ctx);
  ATH_CHECK(outputContainer.record(std::move(outputContainerCellPtr) ) );

  return StatusCode::SUCCESS;
}
