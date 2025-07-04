/*
   Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
 */

#include "LArNNRawChannelBuilder.h"
#include "LArRawChannelBuilderAlg.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "PathResolver/PathResolver.h"

#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArElecCalib/LArProvenance.h"
#include <cmath>

#include <map>
#include <fstream>
#include <sstream>
#include <onnxruntime_cxx_api.h>
#include "CoraCool/CoraCoolDatabase.h"
#include "CoraCool/CoraCoolDatabaseSvcFactory.h"
#include "CoraCool/CoraCoolDatabaseSvc.h"
#include <typeinfo>

using namespace cool;

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
  ATH_CHECK(m_nnClustersDb.initialize());

  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_calocellID,"CaloCell_ID"));
  
  ATH_CHECK(m_onnxRuntimeSvc.retrieve());

  return StatusCode::SUCCESS;
}


StatusCode LArNNRawChannelBuilder::execute(const EventContext& ctx) const {
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);

  std::vector<int> hashIdToCluster;
  std::vector<std::shared_ptr<Ort::Session>> clusterToOnnx;

  const CondAttrListCollection *catr{nullptr};
  ATH_CHECK(SG::get(catr, m_nnClustersDb, ctx));

  if(!catr){
    ATH_MSG_ERROR("CondAttrListCollection can't be opened");
    return StatusCode::FAILURE;
  }

  CondAttrListCollection::const_iterator chanIt=catr->begin();
  const coral::Blob& bls = chanIt->second["clusters"].data<coral::Blob>();
  const unsigned char* blobData = static_cast<const unsigned char*>(bls.startingAddress());

  // Reading BLOB part by part
  int blob_ctr = 0;
  // Nb of IDs encoded as 3 bytes
  int nHash = static_cast<unsigned char>(blobData[blob_ctr]) << 16 |
                static_cast<unsigned char>(blobData[blob_ctr+1]) << 8 |
                static_cast<unsigned char>(blobData[blob_ctr+2]);
  blob_ctr += 3;
  hashIdToCluster.resize(nHash,-1);

  // Nb of clusters encoded as 2 bytes
  int nCluster = static_cast<unsigned char>(blobData[blob_ctr]) << 8 | 
                    static_cast<unsigned char>(blobData[blob_ctr+1]);
  blob_ctr += 2;
  clusterToOnnx.resize(nCluster,nullptr);

  // Reading clusters for each ID
  for(int i=0; i<nHash;i++){
    int cluster;
    cluster = static_cast<unsigned char>(blobData[blob_ctr]) << 8 | 
                static_cast<unsigned char>(blobData[blob_ctr+1]);
    blob_ctr += 2;
    hashIdToCluster[i] = cluster;
  }

  // Creating ONNX model instances
  for(int i=0; i<nCluster; i++){
    // Size of the instance written in the BLOB
    int nnInstanceSize = static_cast<unsigned char>(blobData[blob_ctr]) << 16 |
                    static_cast<unsigned char>(blobData[blob_ctr+1]) << 8 |
                    static_cast<unsigned char>(blobData[blob_ctr+2]);
    blob_ctr += 3;
    std::vector<char> nnInstanceContent(blobData + blob_ctr, blobData + blob_ctr + nnInstanceSize);
    blob_ctr += nnInstanceSize;
    // One session per model
    clusterToOnnx[i] = std::make_shared<Ort::Session>(m_onnxRuntimeSvc->env(), nnInstanceContent.data(), nnInstanceContent.size(), session_options);
  }
  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer>inputContainer(m_digitKey, ctx);
  ATH_CHECK(inputContainer.isValid());
  //Write output via write handle
  auto outputContainerLRPtr = std::make_unique<LArRawChannelContainer>();
  //Get Conditions input
  SG::ReadCondHandle<ILArPedestal>pedHdl(m_pedestalKey, ctx);
  ATH_CHECK(pedHdl.isValid());
  const ILArPedestal* peds = *pedHdl;
   const LArADC2MeV* adc2MeVs{nullptr};
   ATH_CHECK(SG::get(adc2MeVs, m_adc2MeVKey, ctx));
  SG::ReadCondHandle<LArOnOffIdMapping>cabling(m_cablingKey, ctx);
  ATH_CHECK(cabling.isValid());

  // Same instance of input tensors are used
  std::vector<Ort::Value> input_tensors;
  // inputSamples variable memory is being used for the input tensors --> modify inputSamples to modify what's inside the inout tensors
  std::vector<std::vector<float>> inputSamples(24, std::vector<float>(1, 0.0f));
  // Same shapes should be provided for every neural networks
  std::vector<std::vector<int64_t>> inputShape;
  // Same input and output names sould be provided for every neural networks
  std::vector<char*> input_names;
  std::vector<const char*> output_names;
  // Indices are sorted differently with the ORT, so it's needed to keep in memory to go faster than reading for each cell
  std::vector<int> indicesOrder(24,-1);
  // Boolean for the first iteration
  int firstIter = 1;
  //Loop over digits:
  for (const LArDigit* digit : *inputContainer) {
    const HWIdentifier id = digit->hardwareID();
    Identifier idCell;
    try {
        idCell = (*cabling)->cnvToIdentifier(id);
    } catch ( LArID_Exception & except ) {
        ATH_MSG_DEBUG( "A Cabling exception was caught for channel 0x!" 
                    << MSG::hex << id.get_compact() << MSG::dec  );
        continue ;
    }
    const IdentifierHash oflHash=m_calocellID->calo_cell_hash(idCell);
    const bool connected = (*cabling)->isOnlineConnected(id);

    ATH_MSG_VERBOSE("Working on channel " << m_onlineId->channel_name(id));
    const std::vector<short>& samples = digit->samples();
    const int gain = digit->gain();
    const float pedestal_value = peds->pedestal(id, gain);
    int clusterFromHash = hashIdToCluster[oflHash];
    unsigned nnNumInputs = clusterToOnnx[clusterFromHash]->GetInputCount();
    unsigned nnNumOutputs = clusterToOnnx[clusterFromHash]->GetOutputCount();

    if(firstIter==1){
      inputShape.resize(nnNumInputs);
      indicesOrder.resize(nnNumInputs);
      input_names.resize(nnNumInputs);
      for(unsigned int i = 0; i < nnNumInputs; i++){
        auto type_info = clusterToOnnx[clusterFromHash]->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        Ort::AllocatedStringPtr nnVariableNameStrPtr = clusterToOnnx[clusterFromHash]->GetInputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
        // Retrieving the ownership of the unique_ptr that was keeping the variable name
        char * nnVariableName = nnVariableNameStrPtr.release();
        input_names[i] = nnVariableName;
        inputSamples.resize(static_cast<int>(nnNumInputs));
        if(std::strlen(nnVariableName) <= 7){
          ATH_MSG_ERROR("Input name must starts with \"sample_\", then \"m\" (< 0) or \"p\" (>= 0) and end with an index (example : sample_m2)");
          return StatusCode::FAILURE;
        }
        if(!(std::strncmp(nnVariableName, "sample_", 7) == 0)){
          ATH_MSG_ERROR("Input name must starts with \"sample_\", then \"m\" (< 0) or \"p\" (>= 0) and end with an index (example : sample_m2)");
          return StatusCode::FAILURE;
        }
        char index_sign = nnVariableName[7];
        int index = std::atoi(nnVariableName + 8);
        if(index_sign == 'm'){
          index*=-1;
        }
        else if(index_sign != 'p'){
          ATH_MSG_ERROR("Wrong sign used, you have to use \"m\" (< 0) or \"p\" (>= 0)");
          return StatusCode::FAILURE;
        }
        indicesOrder[i] = index;
        for(auto el : tensor_info.GetShape()){
          inputShape[i].push_back((int) abs(el));
        }
        input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, inputSamples[i].data(), inputSamples[i].size(), inputShape[i].data(), inputShape[i].size()));
      }
      for(unsigned int i = 0; i < nnNumOutputs; i++){
        auto outputName = clusterToOnnx[clusterFromHash]->GetOutputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
        output_names.push_back(outputName.release());
      }
    }

    firstIter = 0;
    for(unsigned int i = 0; i < nnNumInputs; i++){
      char index_sign = input_names[i][7];
      int index = std::atoi(input_names[i] + 8);
      if(index_sign == 'm'){
        index*=-1;
      }
      else if(index_sign != 'p'){
        ATH_MSG_ERROR("Wrong sign used, you have to use \"m\" (< 0) or \"p\" (>= 0)");
        return StatusCode::FAILURE;
      }
      inputSamples[i][0] = ((samples[index+m_firstSample]-pedestal_value)/(4096.0-pedestal_value));
    }
    //The following autos will resolve either into vectors or vector-proxies
    const auto& adc2mev = adc2MeVs->ADC2MEV(id, gain);

    if (ATH_UNLIKELY(pedestal_value == ILArPedestal::ERRORCODE)) {
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
    // Check saturation AND discount pedestal on samples used by the NN
    std::vector<float>samp_no_ped(nnNumInputs, 0.0);
    for (unsigned int i = 0; i < nnNumInputs; i++) {
      int index = indicesOrder[i]+m_firstSample;
      if (samples[index] == 4096 || samples[index] == 0) saturated = true;
      samp_no_ped[i] = samples[index]-pedestal_value;
    }

    std::vector<Ort::Value> outputs;
    if(clusterFromHash != -1){     
      outputs = clusterToOnnx[clusterFromHash]->Run(Ort::RunOptions{nullptr}, input_names.data(), input_tensors.data(), input_tensors.size(), output_names.data(), output_names.size());
    }
    else{
        ATH_MSG_ERROR("Hardware ID --> " << id.get_identifier32().get_compact() << " or HashId --> " << oflHash << " not in the input mapping file");
        return StatusCode::FAILURE;
    }

    //normalised output
    An = outputs.front().GetTensorMutableData<float>()[0];

    //taking the normalisation into account
    A = An*(4096.0-pedestal_value);

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
  
  for(auto el : input_names){
    delete el;
  }

  for(auto el : output_names){
    delete el;
  }
  ATH_CHECK(outputContainer.record(std::move(outputContainerLRPtr) ) );
  
  return StatusCode::SUCCESS;
}
