/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterdEdxCondAlg.h"
#include "GaudiKernel/EventIDRange.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

#include <nlohmann/json.hpp>
#include <vector>
#include <tuple>

PixelClusterdEdxCondAlg::PixelClusterdEdxCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelClusterdEdxCondAlg::initialize() {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::initialize()");
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
  
  SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey,ctx);
  const CondAttrListCollection* readCdo = *readHandle; 
  if (readCdo==nullptr) {
    ATH_MSG_FATAL("Null pointer to the read conditions object");
    return StatusCode::FAILURE;
  }
  // Get the validitiy range
  EventIDRange rangeW;
  if (not readHandle.range(rangeW)) {
    ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Size of AthenaAttributeList " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
  ATH_MSG_DEBUG("Range of input is " << rangeW);

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelClusterdEdxCondData> writeCdo(std::make_unique<PixelClusterdEdxCondData>());
  //If configuration flag is turned off, do nothing
  if (m_configFlag == false) {
    ATH_MSG_INFO("Turned off Pixel cluster dEdx equalization, the default behavior is to do nothing");
    const EventIDBase start {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                                                                 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    const EventIDBase stop  {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                             EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    EventIDRange rangeW{start, stop};
    writeCdo->setConfig(false);
    ATH_MSG_INFO("Recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
  }
  else {
      writeCdo->setConfig(m_configFlag);
      CondAttrListCollection::const_iterator itr;
      for (itr = readCdo->begin(); itr != readCdo->end(); ++itr){
        const coral::AttributeList &atr = itr->second;
        std::string dataString = *(static_cast<const std::string *>((atr["data_array"]).addressOfData())); 
        nlohmann::json dataJson = nlohmann::json::parse(dataString); //transform everything
        std::vector<std::tuple<IdentifierHash,float>> params;
        for (auto & objPair : dataJson.items()) {
          std::string module_key_str = objPair.key();
          int module_key = std::atoi(module_key_str.c_str());
          IdentifierHash wafer_hashID(module_key); 
          if (!wafer_hashID.is_valid()) {
            ATH_MSG_FATAL("INVALID HASH ID FOR PIXEL CLUSTER DEDX CALIBRATION");
            return StatusCode::FAILURE;
            }
          float scale_factor = objPair.value();
          if (objPair.value() < 0) {scale_factor = 1.0;} //-1.0 scale factor means no scale factor was derived -> default is to then apply no scaling.
          std::tuple<IdentifierHash,float> dataPair = std::make_tuple(wafer_hashID, scale_factor);
          params.push_back(dataPair);
        }
        writeCdo->setScaleFactors(params); 
      }
  }
  if (rangeW.stop().isValid() and rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelClusterdEdxCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}


