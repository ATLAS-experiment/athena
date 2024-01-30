/*
   Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
 */

#include "LArNNRawChannelBuilder.h"
#include "LArRawChannelBuilderAlg.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "LArRawEvent/LArRawChannelContainer.h"

#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArElecCalib/LArProvenance.h"
#include <cmath>

#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"
#include <map>
#include <fstream>



StatusCode LArNNRawChannelBuilder::initialize() {
  ATH_CHECK(m_digitKey.initialize());
  ATH_CHECK(m_rawChannelKey.initialize());
  ATH_CHECK(m_pedestalKey.initialize());
  ATH_CHECK(m_adc2MeVKey.initialize());
  ATH_CHECK(m_cablingKey.initialize() );
  ATH_CHECK(m_ofcKey.initialize());	 
  ATH_CHECK(m_shapeKey.initialize());
  ATH_CHECK(m_run1DSPThresholdsKey.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_run2DSPThresholdsKey.initialize(SG::AllowEmpty) );
  if (m_useDBFortQ) {
    if (m_run1DSPThresholdsKey.empty() && m_run2DSPThresholdsKey.empty()) {
      ATH_MSG_ERROR ("useDB requested but neither Run1DSPThresholdsKey nor Run2DSPThresholdsKey initialized.");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnlineID"));

  return StatusCode::SUCCESS;
}


StatusCode LArNNRawChannelBuilder::execute(const EventContext& ctx) const {

  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer>inputContainer(m_digitKey, ctx);

  //Write output via write handle

  auto outputContainerLRPtr = std::make_unique<LArRawChannelContainer>();

  //Get Conditions input
  SG::ReadCondHandle<ILArPedestal>pedHdl(m_pedestalKey, ctx);
  const ILArPedestal* peds = *pedHdl;

  SG::ReadCondHandle<LArADC2MeV>adc2mevHdl(m_adc2MeVKey, ctx);
  const LArADC2MeV* adc2MeVs = *adc2mevHdl;

  SG::ReadCondHandle<LArOnOffIdMapping>cabling(m_cablingKey, ctx);

  //Loop over digits:
  for (const LArDigit* digit : *inputContainer) {

    const HWIdentifier id = digit->hardwareID();
    const bool connected = (*cabling)->isOnlineConnected(id);


    ATH_MSG_VERBOSE("Working on channel " << m_onlineId->channel_name(id));

    const std::vector<short>& samples = digit->samples();
    const size_t nSamples = 5; //hardcoded as the network always uses 5 samples

    if ( (samples.size() - m_firstSample) < nSamples) {
      ATH_MSG_ERROR("mismatched effective sample size: "<< samples.size() - m_firstSample << ", must be > " << nSamples); 
      return StatusCode::FAILURE;
    }

    //The following creates a sub-sample vector of the correct size to be passed on to the NN
    std::vector<short>subsamples(nSamples, 0.0);
    for (size_t i = m_firstSample; i < m_firstSample+nSamples; ++i) {
      subsamples[i-m_firstSample] = samples[i];
    }

    const int gain = digit->gain();
    const float p = peds->pedestal(id, gain);


    //The following autos will resolve either into vectors or vector-proxies
    const auto& adc2mev = adc2MeVs->ADC2MEV(id, gain);

    if (ATH_UNLIKELY(p == ILArPedestal::ERRORCODE)) {
      if (!connected) continue;       //No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid pedestal for connected channel " << m_onlineId->channel_name(id)
                                                               << " gain " << gain);
      return StatusCode::FAILURE;
    }

    if (ATH_UNLIKELY(adc2mev.size() < 2)) {
      if (!connected) continue;       //No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid ADC2MeV for connected channel " << m_onlineId->channel_name(id)
                                                              << " gain " << gain);
      return StatusCode::FAILURE;
    }

    // Compute amplitude
    float An = 0;
    float A = 0;
    bool saturated = false;

    // Check saturation AND discount pedestal
    std::vector<float>samp_no_ped(nSamples, 0.0);
    for (size_t i = 0; i < nSamples; ++i) {
      if (subsamples[i] == 4096 || subsamples[i] == 0) saturated = true;
      samp_no_ped[i] = subsamples[i]-p;
    }

    // LWTNN configuration
    std::map<std::string, std::map<std::string, double> >inputs;
    std::map<std::string, std::map<std::string, std::vector<double> > >input_sequences;


    // Read in the network file
    std::ifstream in_file(m_nn_json);


    std::vector<double>nn_in;

    nn_in.clear();

    for (auto d : subsamples) {

      nn_in.push_back((d-p)/4096.0);

    }


    auto config = lwt::parse_json_graph(in_file);
    lwt::LightweightGraph graph(config, m_network_output);

    // Define some sequences used for the recurrent NN
    input_sequences["node_0"] = {
      {m_input_node,  nn_in}
    };

    // More variables for the two other inputs defined
    inputs["node_0"] = {{"variable_0", 0}};

    // Calculate the output value of the NN based on the inputs given
    auto outputs = graph.compute(inputs, input_sequences);

    //normalised output
    An = outputs.begin()->second;

    //taking the normalisation into account
    A = An*4096.0;

    //Apply Ramp
    const float E = adc2mev[0]+A*adc2mev[1];

    uint16_t iquaShort = 0;
    float tau = 0;


    uint16_t prov = LArProv::PEAKNN | LArProv::RAMPDB | LArProv::PEDDB;
    if (saturated) prov |= LArProv::SATURATED;


    outputContainerLRPtr->emplace_back(id, static_cast<int>(std::floor(E+0.5)),
                                        static_cast<int>(std::floor(tau+0.5)),
                                        iquaShort, prov, (CaloGain::CaloGain)gain);
    
  }

  SG::WriteHandle<LArRawChannelContainer>outputContainer(m_rawChannelKey, ctx);
  ATH_CHECK(outputContainer.record(std::move(outputContainerLRPtr) ) );
  

  return StatusCode::SUCCESS;
}
