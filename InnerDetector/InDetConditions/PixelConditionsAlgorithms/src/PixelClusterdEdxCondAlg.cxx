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
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::initialize() Rebecca");
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PixelClusterdEdxCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::execute()");

  SG::WriteCondHandle<PixelClusterdEdxCondData> writeHandle(m_writeKey, ctx); 
  if (writeHandle.isValid()) {
    ATH_MSG_INFO("CondHandle " << writeHandle.fullKey() << " is already valid. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  
  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelClusterdEdxCondData> writeCdo(std::make_unique<PixelClusterdEdxCondData>());

  const EventIDBase start {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                                                                 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop  {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
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

    //If configuration flag is turned off
    if (m_configFlag == false) {
      ATH_MSG_INFO("Turned off PixelToTPIDTool scale factors. The default behavior is to do nothing. -- Rebecca");
      writeCdo->setConfig(false);
      //const std::tuple<std::tuple<int,int,int>,float> & sf_placeholder = std::make_tuple(std::make_tuple(0,0,0), -1.0);
      //const std::vector<std::tuple<std::tuple<int,int,int>,float>> & params_placeholder = {sf_placeholder};
      //writeCdo->setVar(params_placeholder);
      //ATH_MSG_INFO("Rebecca - recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
    }
    else {
        writeCdo->setConfig(m_configFlag);
        //Reading from COOL DB
        //Database Structure:
        //Using tag selection: PixelTest
        //[0,0] - [2147483647,4294967295) (0)[data_array (String16M) : 
        //{"bec":[0,0,0,0,-2,-2,-2,2,2,2], 
        //"layerID":[0,1,2,3,0,1,2,0,1,2], 
        //"etaM":[[-10.0,-9.0,...],...], 
        //"SF":[[-1.0,-1.0,-1.0,-1.0,1.2693007542467394,..],...]}
        CondAttrListCollection::const_iterator itr;
        for (itr = readCdo->begin(); itr != readCdo->end(); ++itr){
          const coral::AttributeList &atr = itr->second;
          std::string dataString = *(static_cast<const std::string *>((atr["data_array"]).addressOfData())); // read everything from DB
          nlohmann::json dataJson = nlohmann::json::parse(dataString); //transform everything
          std::vector<int> bec_data = dataJson["bec"];
          std::vector<int> layerID_data = dataJson["layerID"]; 
          std::vector<std::vector<int>> etaM_data = dataJson["etaM"];
          std::vector<std::vector<float>> SF_data = dataJson["SF"];
          // Want to store like: [((bec,layerID,etaM),SF)),...]
          // Assumption of bec size == layerID size and that there is a vector corresponding to etaM and SF for each of those entries. 
          std::vector<std::tuple<std::tuple<int,int,int>,float>> params;
          for (size_t i = 0; i < bec_data.size(); ++i) { // Assumption of bec.size() == layerID.size()
            for (size_t j = 0; j < etaM_data[i].size(); ++j) { //Similarly assumption of etaM[i].size() == SF.size()
              std::tuple<int,int,int> sf_coordinates = std::make_tuple(bec_data[i],layerID_data[i],etaM_data[i][j]);
              //-1 indicates no scale factor was derived. Set to 1.0
              float scale_factor = -999.0;
              if(SF_data[i][j] < 0) {scale_factor = 1.0;}
              else {scale_factor = SF_data[i][j];}
              std::tuple<std::tuple<int,int,int>,float> sf_coordinate_value = std::make_tuple(sf_coordinates, scale_factor); 
              params.push_back(sf_coordinate_value);
              }
            }
          std::cout << "Rebecca -- The parameter vector" << std::endl;
          // Iterate through the vector and print each element
          // for (const auto& outer_tuple : params) {
            // Extract the inner tuple and the long double
           // auto inner_tuple = std::get<0>(outer_tuple);
           // long double value = std::get<1>(outer_tuple);

            // Print the elements of the inner tuple
          //  std::cout << "("
          //            << std::get<0>(inner_tuple) << ", "
          //            << std::get<1>(inner_tuple) << ", "
          //            << std::get<2>(inner_tuple) << ") "
          //            << "-> " << value << std::endl;
          //}
          writeCdo->setVar(params); 
          }
    }
  }

  else { // no readKey 
    ATH_MSG_DEBUG("No readKey has been passed to PixelClusterdEdxCondAlg.");
    return StatusCode::FAILURE;
  }

  if (rangeW.stop().isValid() and rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelClusterdEdxCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Rebecca - recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}


