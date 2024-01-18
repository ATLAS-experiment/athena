/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawChannelBuilderAlg.h" 
#include "GaudiKernel/SystemOfUnits.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include <cmath>
#include <memory>

  
StatusCode LArRawChannelBuilderAlg::initialize() {
  ATH_CHECK(m_digitKey.initialize());	 
  ATH_CHECK(m_rawChannelKey.initialize());
  ATH_CHECK(m_pedestalKey.initialize());	 
  ATH_CHECK(m_adc2MeVKey.initialize());	 
  ATH_CHECK(m_ofcKey.initialize());	 
  ATH_CHECK(m_shapeKey.initialize());
  ATH_CHECK(m_cablingKey.initialize() );
  ATH_CHECK(m_run1DSPThresholdsKey.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_run2DSPThresholdsKey.initialize(SG::AllowEmpty) );
  if (m_useDBFortQ) {
    if (m_run1DSPThresholdsKey.empty() && m_run2DSPThresholdsKey.empty()) {
      ATH_MSG_ERROR ("useDB requested but neither Run1DSPThresholdsKey nor Run2DSPThresholdsKey initialized.");
      return StatusCode::FAILURE;
    }
  }


  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnlineID"));
	

  const std::string cutmsg = m_absECutFortQ.value() ? " fabs(E) < " : " E < "; 
  ATH_MSG_INFO("Energy cut for time and quality computation: " << cutmsg << 
               " taken from COOL folder "<<
               m_run1DSPThresholdsKey.key() << " (run1) " <<
               m_run2DSPThresholdsKey.key() << " (run2) ");
  return StatusCode::SUCCESS;
}     

StatusCode LArRawChannelBuilderAlg::finalize() {
  return StatusCode::SUCCESS;
} 

StatusCode LArRawChannelBuilderAlg::execute(const EventContext& ctx) const {

  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer> inputContainer(m_digitKey,ctx);

  //Write output via write handle
  auto outputContainer = std::make_unique<LArRawChannelContainer>();
	    
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
  
  std::unique_ptr<LArDSPThresholdsFlat> run2DSPThresh;
  const LArDSPThresholdsComplete* run1DSPThresh = nullptr;
  if (m_useDBFortQ) {
    if (!m_run2DSPThresholdsKey.empty()) {
      SG::ReadCondHandle<AthenaAttributeList> dspThrshAttr (m_run2DSPThresholdsKey, ctx);
      run2DSPThresh = std::make_unique<LArDSPThresholdsFlat>(*dspThrshAttr);
      if (ATH_UNLIKELY(!run2DSPThresh->good())) {
        ATH_MSG_ERROR( "Failed to initialize LArDSPThresholdFlat from attribute list loaded from " << m_run2DSPThresholdsKey.key()
                       << ". Aborting." ); 
        return StatusCode::FAILURE;
      }
    }
    else if (!m_run1DSPThresholdsKey.empty()) {
      SG::ReadCondHandle<LArDSPThresholdsComplete> dspThresh (m_run1DSPThresholdsKey, ctx);
      run1DSPThresh = dspThresh.cptr();
    }
    else {
      ATH_MSG_ERROR( "No DSP threshold configured.");
      return StatusCode::FAILURE;
    }
  }

  //Loop over digits:
  for (const LArDigit* digit : *inputContainer) {
  
    size_t firstSample=m_firstSample;

    const HWIdentifier id=digit->hardwareID();
    const bool connected=(*cabling)->isOnlineConnected(id);
    

    ATH_MSG_VERBOSE("Working on channel " << m_onlineId->channel_name(id));

    const std::vector<short>& samples=digit->samples();
    const int gain=digit->gain();
    const float p=peds->pedestal(id,gain);
   


    //The following autos will resolve either into vectors or vector-proxies
    const auto& ofca=ofcs->OFC_a(id,gain);
    const auto& adc2mev=adc2MeVs->ADC2MEV(id,gain);

    const size_t nOFC=ofca.size();
    
    //Sanity check on input conditions data:
    // ensure that the size of the samples vector is compatible with ofc_a size when preceeding samples are saved
    const size_t nSamples=samples.size()-firstSample;
    if (nSamples<nOFC) {
      ATH_MSG_ERROR("effective sample size: "<< nSamples << ", must be >= OFC_a size: " << ofca.size());
      return StatusCode::FAILURE;
    }
    
    
    if (ATH_UNLIKELY(p==ILArPedestal::ERRORCODE)) {
      if (!connected) continue; //No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid pedestal for connected channel " << m_onlineId->channel_name(id) 
		    << " gain " << gain);
      return StatusCode::FAILURE;
    }

    if(ATH_UNLIKELY(adc2mev.size()<2)) {
      if (!connected) continue; //No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid ADC2MeV for connected channel " << m_onlineId->channel_name(id) 
		    << " gain " << gain);
      return StatusCode::FAILURE;
    }


    //Apply OFCs to get amplitude
    // Evaluate sums in double-precision to get consistent results
    // across platforms.
    double A=0;
    bool saturated=false;

    // Check saturation AND discount pedestal
    std::vector<float> samp_no_ped(nOFC,0.0);
    for (size_t i=0;i<nOFC;++i) {
      if (samples[i+firstSample]==4096 || samples[i+firstSample]==0) saturated=true; 
      samp_no_ped[i]=samples[i+firstSample]-p;
    }
    for (size_t i=0;i<nOFC;++i) {
       A+=static_cast<double>(samp_no_ped[i])*ofca[i];
    }
    
    //Apply Ramp
    const float E=adc2mev[0]+A*adc2mev[1];
    
    uint16_t iquaShort=0;
    float tau=0;


    uint16_t prov=0xa5; //Means all constants from DB
    if (saturated) prov|=0x0400;

    const float E1=m_absECutFortQ.value() ? std::fabs(E) : E;
    float ecut(0.);
    if (m_useDBFortQ) {
      if (run2DSPThresh) {
        ecut = run2DSPThresh->tQThr(id);
      }
      else if (run1DSPThresh) {
        ecut = run1DSPThresh->tQThr(id);
      }
      else {
        ATH_MSG_ERROR ("DSP threshold problem");
        return StatusCode::FAILURE;
      }
    }
    else {
      ecut = m_eCutFortQ;
    }
    if (E1 > ecut) {
      ATH_MSG_VERBOSE("Channel " << m_onlineId->channel_name(id) << " gain " << gain << " above threshold for tQ computation");
      prov|=0x2000; //  fill bit in provenance that time+quality information are available

      //Get time by applying OFC-b coefficients:
      const auto& ofcb=ofcs->OFC_b(id,gain);
      double At=0;
      for (size_t i=0;i<nOFC;++i) {
	At+=static_cast<double>(samp_no_ped[i])*ofcb[i];
      }
      //Divide A*t/A to get time
      tau=(std::fabs(A)>0.1) ? At/A : 0.0;
      const auto& fullShape=shapes->Shape(id,gain);
      
      //Get Q-factor
      //fixing HEC to move +1 in case of 4 samples and firstSample 0 (copied from old LArRawChannelBuilder)
      const size_t nSamples=samples.size();
      if (fullShape.size()>nSamples && nSamples==4 && m_firstSample==0) {
	if (m_onlineId->isHECchannel(id)) {
	  firstSample=1;
	}
      }

      if (ATH_UNLIKELY(fullShape.size()<nOFC+firstSample)) {
	if (!connected) continue; //No conditions for disconnected channel, who cares?
	  ATH_MSG_ERROR("No valid shape for channel " <<  m_onlineId->channel_name(id) 
		      << " gain " << gain);
	  ATH_MSG_ERROR("Got size " << fullShape.size() << ", expected at least " << nSamples+firstSample);
	  return StatusCode::FAILURE;
      }

      const float* shape=&*fullShape.begin()+firstSample;

      double q=0;
      if (m_useShapeDer) {
	const auto& fullshapeDer=shapes->ShapeDer(id,gain);
	if (ATH_UNLIKELY(fullshapeDer.size()<nOFC+firstSample)) {
	  ATH_MSG_ERROR("No valid shape derivative for channel " <<  m_onlineId->channel_name(id) 
			<< " gain " << gain);
	  ATH_MSG_ERROR("Got size " << fullshapeDer.size() << ", expected at least " << nOFC+firstSample);
	  return StatusCode::FAILURE;
	}

	const float* shapeDer=&*fullshapeDer.begin()+firstSample;
	for (size_t i=0;i<nOFC;++i) {
	  q += std::pow((A*(shape[i]-tau*shapeDer[i])-(samp_no_ped[i])),2);
	}
      }//end if useShapeDer
      else {
	//Q-factor w/o shape derivative
	for (size_t i=0;i<nOFC;++i) {
	  q += std::pow((A*shape[i]-(samp_no_ped[i])),2);
	}
      }

      int iqua = static_cast<int>(q);
      if (iqua > 0xFFFF) iqua=0xFFFF;
      iquaShort = static_cast<uint16_t>(iqua & 0xFFFF);

      tau-=ofcs->timeOffset(id,gain);
      tau*=(Gaudi::Units::nanosecond/Gaudi::Units::picosecond); //Convert time to ps
    }//end if above cut

    outputContainer->emplace_back(id,static_cast<int>(std::floor(E+0.5)),
				  static_cast<int>(std::floor(tau+0.5)),
				  iquaShort,prov,(CaloGain::CaloGain)gain);
    }

 
  SG::WriteHandle<LArRawChannelContainer>outputHandle(m_rawChannelKey,ctx);
  ATH_CHECK(outputHandle.record(std::move(outputContainer) ) );
  
  return StatusCode::SUCCESS;
}


