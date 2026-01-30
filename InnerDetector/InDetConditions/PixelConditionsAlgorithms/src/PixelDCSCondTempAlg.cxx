/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelDCSCondTempAlg.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventIDRange.h"

#include <memory>

namespace {
  const std::string parameterName{"temperature"};
}

PixelDCSCondTempAlg::PixelDCSCondTempAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthCondAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelDCSCondTempAlg::initialize() {
  ATH_MSG_DEBUG("PixelDCSCondTempAlg::initialize()");

  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));

  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelDCSCondTempAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelDCSCondTempAlg::execute()");

  SG::WriteCondHandle<PixelDCSTempData> writeHandle(m_writeKey, ctx);
  // Do we have a valid Write Cond Handle for current time?
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelDCSTempData> writeCdo(std::make_unique<PixelDCSTempData>());


  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};
  //
  std::vector<int> channelsOutOfRange{}; //keep track of which channels are out of range, if any
  std::vector<int> channelsWithNoMeasurement{}; //similar for those with no value
  const float defaultTemperature = m_defaultTemperature;
  int countChannels=0;
  if (!m_readKey.empty()) {
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey, ctx);
    const CondAttrListCollection* readCdo(*readHandle); 
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    // Read temperature info
    auto outsideValidRange = [] (const float t){
      return ( (t > 100.0f)  or ( t< -80.f) ) ;
    };
    
    countChannels=readCdo->size();
    for (const auto & attrList : *readCdo) {
      const auto channelNumber{attrList.first};
      const auto & payload{attrList.second};
      if (payload.exists(parameterName) and not payload[parameterName].isNull()) {
        float val = payload[parameterName].data<float>();
        if (outsideValidRange(val)) {
          channelsOutOfRange.push_back(channelNumber);
          val = defaultTemperature;
        };
        writeCdo->setTemperature((int)channelNumber, val);
      } else {
        channelsWithNoMeasurement.push_back(channelNumber);
        writeCdo->setTemperature((int)channelNumber, defaultTemperature);
      }
    }
  } else {
    for (int i=0; i<(int)m_pixelID->wafer_hash_max(); i++) {
      writeCdo->setTemperature(i, defaultTemperature);
    }
  }
  if (const int nInvalid = channelsWithNoMeasurement.size(); nInvalid>0){
    ATH_MSG_INFO("Out of "<<countChannels<<", "<<nInvalid<<" channels have no temperature measurement, and were set to "<<defaultTemperature);
    if ( msgLevel( MSG::DEBUG )){
      std::string dbgMsg{"Enumerating the channel numbers:\n"};
      for (auto i:channelsWithNoMeasurement){
        dbgMsg += std::to_string(i) + " ";
      }
      dbgMsg +="\n";
      ATH_MSG_DEBUG(dbgMsg);
    }
  }
  if (const int nOutOfRange = channelsOutOfRange.size(); nOutOfRange>0){
    ATH_MSG_INFO("Out of "<<countChannels<<", "<<nOutOfRange<<" channels have temperatures outside the range -80 to 100, and were set to "<<defaultTemperature);
    if ( msgLevel( MSG::DEBUG )){
      std::string dbgMsg{"Enumerating the channel numbers:\n"};
      for (auto i:channelsOutOfRange){
        dbgMsg += std::to_string(i) + " ";
      }
      dbgMsg +="\n";
      ATH_MSG_DEBUG(dbgMsg);
    }
  }
  
  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelDCSTempData " << writeHandle.key() << " with EventRange " 
		  << writeHandle.getRange() << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " 
	       << writeHandle.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS;
}

