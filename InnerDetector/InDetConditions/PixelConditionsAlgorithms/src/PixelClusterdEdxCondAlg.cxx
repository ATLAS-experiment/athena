/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterdEdxCondAlg.h"
#include "GaudiKernel/EventIDRange.h"
#include "StringUtilities.h"

#include "Identifier/Identifier32.h"
#include "Identifier/IdentifierHash.h"

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
  
  
//Rebecca-- Now it's getting more complicted with no data object structure...
  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelClusterdEdxCondData> writeCdo(std::make_unique<PixelClusterdEdxCondData>()); //XXXChange later

  writeCdo->setVar("Rebecca was here 2!"); //Testing randomnumber 


  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};

 if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelClusterdEdxCond " << writeHandle.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range ");

  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey,ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object -- Rebecca");
      return StatusCode::FAILURE;
    }

  // Get the validitiy range
  EventIDRange rangeW;
  if (not readHandle.range(rangeW)) {
     ATH_MSG_FATAL("Rebecca -- Failed to retrieve validity range for " << readHandle.key());
     return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Rebecca -- Range of input is " << rangeW);

  for (const auto & attrList : *readCdo) {

      const CondAttrListCollection::AttributeList &payload = attrList.second;
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
	      const std::string &stringStatus = payload["data_array"].data<std::string>();
        ATH_MSG_INFO("Rebecca payload: " << stringStatus);    
      }
  } // readKey not empty



  return StatusCode::SUCCESS;

}


