/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterdEdxCondAlg.h"
#include "GaudiKernel/EventIDRange.h"

#include <nlohmann/json.hpp>


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

    //Reading from COOL DB
    //DB Structure:
    //Using tag selection: PixelTest
    //[0,0] - [2147483647,4294967295) (0) [data_array (String16M) : {"0": 1234, "1": 5678, "2": 1345, "3": 910, "4": 1, "5": 190, "6": 780, "7": 2345, "8": 123, "9": 632, "10": 574}]
    CondAttrListCollection::const_iterator itr;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr){//Loop over channels (only one in this case)
      const coral::AttributeList &atr = itr->second;
      std::string data = *(static_cast<const std::string *>((atr["data_array"]).addressOfData())); // read everything from DB
      ATH_MSG_INFO("Rebecca Payload from DB:" << data);
      nlohmann::json jsondata = nlohmann::json::parse(data); //transform everything
      ATH_MSG_INFO("Rebecca -- parsed DB data: " << jsondata);
      //int nchannels=jsondata["nchannels"]; //Breaks here, null.
      nlohmann::json channeldata=jsondata["0"]; // get "data" column, table-inside-table
      ATH_MSG_INFO("Rebecca -- channel data: " << channeldata);
      int testData = -999;
      testData = channeldata;
      writeCdo->setVar(testData); //Gives error that testData is null
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


