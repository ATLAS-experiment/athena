/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/TgcDigitASDposCondAlg.h"
#include "CxxUtils/StringUtils.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "CoralBase/Blob.h"
#include <string_view>


TgcDigitASDposCondAlg::TgcDigitASDposCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthCondAlgorithm(name, pSvcLocator)
{}

StatusCode TgcDigitASDposCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_readKey_ASDpos.initialize());
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TgcDigitASDposCondAlg::execute(const EventContext& ctx) const
{
  SG::WriteCondHandle<TgcDigitASDposData> writeHandle{m_writeKey, ctx};
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
		  << ". In theory this should not be called, but may happen"
		  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<CondAttrListCollection> readHandle_ASDpos{m_readKey_ASDpos, ctx};
  if (!readHandle_ASDpos.isValid()) {
    ATH_MSG_ERROR("Null pointer to the read conditions object");
    return StatusCode::FAILURE;
  }
  writeHandle.addDependency(readHandle_ASDpos);

  // Fill
  auto outputCdo = std::make_unique<TgcDigitASDposData>();
  constexpr std::string_view delimiter{";"};
  for(const auto &[channel, attribute] : **readHandle_ASDpos) {
    const coral::Blob& blob = attribute["bASDPos"].data<coral::Blob>();
    const std::string blobline{static_cast<const char*>(blob.startingAddress())};
    std::vector<std::string> tokens = CxxUtils::tokenize(blobline, delimiter);
    auto it = std::begin(tokens);
    uint16_t station = static_cast<uint16_t>(CxxUtils::atoi(*it));
    ++it;
    uint16_t eta = static_cast<uint16_t>(CxxUtils::atoi(*it));
    ++it;
    uint16_t phi = (CxxUtils::atoi(*it) == -99) ? 0x1f : static_cast<uint16_t>(CxxUtils::atoi(*it));
    uint16_t chamberId = (station << 8)  + (eta << 5) + phi;

    std::vector<float> strip_pos(TgcDigitASDposData::N_STRIPASDPOS, 0);
    //strip_pos initialized to size N_STRIPASDPOS
    for (int i=0; i < TgcDigitASDposData::N_STRIPASDPOS; i++) {
      ++it;
      strip_pos[i] = CxxUtils::atof(*it);
    }
    outputCdo->stripAsdPos.emplace(chamberId, std::move(strip_pos));

    std::vector<float> wire_pos(TgcDigitASDposData::N_WIREASDPOS, 0);
    //TgcDigitASDposData initialized to size N_WIREASDPOS
    for (int i=0; i < TgcDigitASDposData::N_WIREASDPOS; i++) {
      ++it;
      wire_pos[i] = CxxUtils::atof(*it);
    }
    outputCdo->wireAsdPos.emplace(chamberId, std::move(wire_pos));
  }  // end of for(attrmap)

  // Record
  ATH_CHECK(writeHandle.record(std::move(outputCdo)));
  ATH_MSG_DEBUG("recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS;
}
