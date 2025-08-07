/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterdEdxCondAlg.h"
#include "GaudiKernel/EventIDRange.h"

#include <nlohmann/json.hpp>
#include <vector>
#include <tuple>

PixelClusterdEdxCondAlg::PixelClusterdEdxCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelClusterdEdxCondAlg::initialize() {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::initialize() Rebecca Test");
  //ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PixelClusterdEdxCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::execute()");

  SG::WriteCondHandle<PixelClusterdEdxCondData> writeHandle(m_writeKey, ctx); //XXXChange here later!
  if (writeHandle.isValid()) {
    ATH_MSG_INFO("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  
  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelClusterdEdxCondData> writeCdo(std::make_unique<PixelClusterdEdxCondData>());

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};

  //Making readCdo
  if(!m_readKey.empty()){
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey,ctx);
    const CondAttrListCollection* readCdo = *readHandle;
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object -- Rebecca");
      return StatusCode::FAILURE;
    }

    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Rebecca -- Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }


  if (m_configStatus == false) {
    ATH_MSG_INFO("Turned off PixelToTPIDTool scalefactors. The default behavior is to do nothing. -- Rebecca");
    writeCdo->setConfig(0);
    const std::tuple<std::tuple<int,int,int>,float> & sf_placeholder = std::make_tuple(std::make_tuple(0,0,0), -1.0);
    const std::vector<std::tuple<std::tuple<int,int,int>,float>> & params_placeholder = {sf_placeholder};
    writeCdo->setVar(params_placeholder);
    ATH_MSG_INFO("Rebecca - recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
    return StatusCode::SUCCESS;
  }
    //Reading from COOL DB
    //DB Structure:
    //Using tag selection: PixelTest
    //[0,0] - [2147483647,4294967295) (0) [data_array (String16M) : {"0": 1234, "1": 5678, "2": 1345, "3": 910, "4": 1, "5": 190, "6": 780, "7": 2345, "8": 123, "9": 632, "10": 574}]
    CondAttrListCollection::const_iterator itr;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr){//Loop over channels (only one in this case)
      const coral::AttributeList &atr = itr->second;
      std::string dataString = *(static_cast<const std::string *>((atr["data_array"]).addressOfData())); // read everything from DB
      // ATH_MSG_INFO("Rebecca Payload from DB:" << dataString);
      nlohmann::json dataJson = nlohmann::json::parse(dataString); //transform everything
      ATH_MSG_INFO("Rebecca -- parsed DB data: " << dataJson);
      std::vector<int> bec_data = dataJson["bec"];
      std::vector<int> layerID_data = dataJson["layerID"]; 
      std::vector<std::vector<int>> etaM_data = dataJson["etaM"];
      std::vector<std::vector<float>> SF_data = dataJson["SF"];
      //Data structure:
      // // IBL: bec=0; layer=0; etaM=[-10,9] {Planars=[-6,5] & 3D=[6,9;-10,7]}; phiM=[0,13]
      // B-layer: bec=0; layer=1; etaM=[-6,6]; phiM=[0,21]
      // Layer-1: bec=0; layer=2; etaM=[-6,6]; phiM=[0,37]
      // Layer-2: bec=0; layer=3; etaM=[-6,6]; phiM=[0,51]
      // EC_C: bec=-2 & EC_A: bec=+2
      //D1: layer=0; etaM=0; phiM=[0,47]
      //D2: layer=1; etaM=0; phiM=[0,47]
      //D3: layer=2; etaM=0; phiM=[0,47]
      // Want to store like: [((bec,layerID,etaM),SF)),...] 
      std::vector<std::tuple<std::tuple<int,int,int>,float>> params;
      for (size_t i = 0; i < bec_data.size(); ++i) { // Assumption of bec.size() == layerID.size()
        for (size_t j = 0; j < etaM_data[i].size(); ++j) { //Similarly etaM[i].size() == SF.size()
          std::tuple<int,int,int> sf_coordinates = std::make_tuple(bec_data[i],layerID_data[i],etaM_data[i][j]);
          std::tuple<std::tuple<int,int,int>,float> sf_coordinate_value = std::make_tuple(sf_coordinates, SF_data[i][j]); 
          params.push_back(sf_coordinate_value);
        }
      }
      std::cout << "Rebecca -- Here's the parameter vector" << std::endl;

      // Iterate through the vector and print each element
      for (const auto& outer_tuple : params) {
        // Extract the inner tuple and the long double
        auto inner_tuple = std::get<0>(outer_tuple);
        long double value = std::get<1>(outer_tuple);

        // Print the elements of the inner tuple
        std::cout << "("
                  << std::get<0>(inner_tuple) << ", "
                  << std::get<1>(inner_tuple) << ", "
                  << std::get<2>(inner_tuple) << ") "
                  << "-> " << value << std::endl;
      }
      //int testData = -999;
      //testData = channeldata;
      writeCdo->setVar(params); //Gives error that testData is null
   }
  }

  else { // no readKey and no jsonFiles have been defined.
    ATH_MSG_DEBUG("No readKey and jsonFile have been passed to PixelClusterdEdxCondAlg.");
  }

  if (rangeW.stop().isValid() and rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelDeadMapCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Rebecca - recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}


