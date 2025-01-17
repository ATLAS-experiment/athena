/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterdEdxCondAlg.h"
#include "GaudiKernel/EventIDRange.h"
#include "StringUtilities.h"

#include "Identifier/Identifier32.h"
#include "Identifier/IdentifierHash.h"
#include <nlohmann/json.hpp>

#include <fstream>

using PixelConditionsAlgorithms::parseDeadMapString;
using json = nlohmann::json;

PixelClusterdEdxCondAlg::PixelClusterdEdxCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelClusterdEdxCondAlg::initialize() {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::initialize() Rebecca Test");

  ATH_CHECK (detStore()->retrieve(m_pixelID, "PixelID") );
  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(m_writeKey2.initialize()); //XXXChange in future

  return StatusCode::SUCCESS;
}

StatusCode PixelClusterdEdxCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_INFO("PixelClusterdEdxCondAlg::execute()");

  SG::WriteCondHandle<PixelDeadMapCondData> writeHandle(m_writeKey, ctx); //XXXChange here later!
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }

  SG::WriteCondHandle<PixelClusterdEdxCondData> writeHandle2(m_writeKey2, ctx); //XXXChange here later!
  if (writeHandle2.isValid()) {
    ATH_MSG_INFO("CondHandle " << writeHandle2.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  
  
//Rebecca-- Now it's getting more complicted with no data object structure...
  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelDeadMapCondData> writeCdo(std::make_unique<PixelDeadMapCondData>());
  std::unique_ptr<PixelClusterdEdxCondData> writeCdo2(std::make_unique<PixelClusterdEdxCondData>()); //XXXChange later

  writeCdo2->setVar(999); //Testing randomnumber 


  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};

 if (writeHandle2.record(rangeW, std::move(writeCdo2)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelClusterdEdxCond " << writeHandle2.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("recorded new CDO " << writeHandle2.key() << " with range ");

  return StatusCode::SUCCESS;


 /* 
  if (!m_readKey.empty()) {
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey, ctx);
    const CondAttrListCollection* readCdo = *readHandle; 
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }
    
    for (const auto & attrList : *readCdo) {

      const CondAttrListCollection::AttributeList &payload = attrList.second;
      // RUN-3 format
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
	const std::string &stringStatus = payload["data_array"].data<std::string>();
        const auto & hashStatusVector = parseDeadMapString(stringStatus);
	
        for (const auto & [hash, status] : hashStatusVector){
          //status ==0 means its the module status to be set to '1'
          //if (status==0) writeCdo->setModuleStatus(hash, 1); //XXXNow it is pointing to a fake conditions data object, so need to change
          //...any other status will set the chip status
          //else writeCdo->setChipStatus(hash, status); //XXXChange
        }
      }
    }
  } // readKey not empty
  else if (!m_JsonLocation.empty()) {   // use json for dead modules if defined

    ATH_MSG_DEBUG("Reading in the json file:"<<m_JsonLocation);
    std::ifstream json_file(m_JsonLocation);
    if (!json_file.is_open()) {
      ATH_MSG_ERROR("Failed to open the json file with dead pixel modules");
     // return StatusCode::FAILURE;
    }
   
  //Reading relies on some knowledge of the json file structure 
    json dead_module_data = json::parse(json_file);
    
    for (const auto& i : dead_module_data) {
      std::string id_mod = i["Decimal_ID"];
      unsigned long long idull = std::stoull(id_mod);
      Identifier id(idull);
      
      // This assumes that I am passing identifiers only valid for pixel
      IdentifierHash idhash = m_pixelID->wafer_hash(id);

      // Switch off sensor in condition store
      ATH_MSG_DEBUG("Disabling sensor:"<<id_mod);
      
      //writeCdo->setModuleStatus(idhash,1);
    } 
  } 
  else { // no readKey and no jsonFiles have been defined.
    ATH_MSG_DEBUG("No readKey and jsonFile have been passed to PixelDeadMapCondAlg.");
  }

  if (rangeW.stop().isValid() and rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    //return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelDeadMapCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    //return StatusCode::FAILURE;
  }
  
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
    
  //return StatusCode::SUCCESS;*/
}


