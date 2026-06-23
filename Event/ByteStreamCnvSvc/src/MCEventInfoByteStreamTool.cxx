/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MCEventInfoByteStreamTool.h"

#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"
#include "CxxUtils/span.h"
#include "AthContainers/AuxElement.h"

#include <bit>
#include <format>

namespace {
  const SG::AuxElement::Accessor<uint64_t> acc_pileUpMixtureLow("pileUpMixtureIDLowBits");
  const SG::AuxElement::Accessor<uint64_t> acc_pileUpMixtureHigh("pileUpMixtureIDHighBits");
}

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

MCEventInfoByteStreamTool::MCEventInfoByteStreamTool(const std::string& type,
                                                     const std::string& name,
                                                     const IInterface* parent)
  : base_class(type, name, parent) {}

StatusCode MCEventInfoByteStreamTool::initialize() {
  ATH_MSG_DEBUG("Initializing MCEventInfoByteStreamTool");

  // Check if we're in encoding mode (reading EventInfo from store)
  if (!m_eventInfoReadKey.empty()) {
    ATH_CHECK(m_eventInfoReadKey.initialize());
    ATH_MSG_DEBUG(std::format("Encoding mode: will read EventInfo from {}", m_eventInfoReadKey.key()));
  } else {
    ATH_MSG_DEBUG("Decoding mode: will decode MC EventInfo from ROB fragment");
  }

  ATH_MSG_DEBUG("ROB IDs: " << MSG::hex << m_robIds.value() << MSG::dec);
  return StatusCode::SUCCESS;
}

StatusCode MCEventInfoByteStreamTool::convertToBS(std::vector<WROBF*>& vrobf,
                                                  const EventContext& eventContext) {
  ATH_MSG_DEBUG("convertToBS called");

  if (m_eventInfoReadKey.empty()) {
    ATH_MSG_ERROR("EventInfoReadKey not configured for encoding");
    return StatusCode::FAILURE;
  }

  // Retrieve EventInfo
  auto evtInfoHandle = SG::makeHandle(m_eventInfoReadKey, eventContext);
  if (!evtInfoHandle.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve EventInfo from " << m_eventInfoReadKey.key());
    return StatusCode::FAILURE;
  }
  const xAOD::EventInfo* evtInfo = evtInfoHandle.cptr();

  // Check if this is simulation
  if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
    ATH_MSG_DEBUG("Event is not simulation, skipping MC EventInfo encoding");
    return StatusCode::SUCCESS;
  }

  // Clear BS data cache
  clearCache(eventContext);

  // Get MC fields
  uint32_t mcChannelNumber = evtInfo->mcChannelNumber();
  uint64_t mcEventNumber = evtInfo->mcEventNumber();
  float actualMu = evtInfo->actualInteractionsPerCrossing();
  float averageMu = evtInfo->averageInteractionsPerCrossing();
  uint32_t eventTypeBitmask = evtInfo->eventTypeBitmask();
  uint32_t extendedLevel1ID = evtInfo->extendedLevel1ID();
  uint32_t backgroundFlags = evtInfo->eventFlags(xAOD::EventInfo::Background);

  // Get pileup mixture ID
  uint64_t pileUpMixtureLow = 0;
  uint64_t pileUpMixtureHigh = 0;
  if (acc_pileUpMixtureLow.isAvailable(*evtInfo)) {
    pileUpMixtureLow = acc_pileUpMixtureLow(*evtInfo);
  }
  if (acc_pileUpMixtureHigh.isAvailable(*evtInfo)) {
    pileUpMixtureHigh = acc_pileUpMixtureHigh(*evtInfo);
  }

  // Get MC weights
  const std::vector<float>& weights = evtInfo->mcEventWeights();
  uint32_t nWeights = static_cast<uint32_t>(weights.size());

  // Calculate total data size
  size_t ndata = MIN_DATA_WORDS + nWeights;

  ATH_MSG_DEBUG("Encoding MC EventInfo: mcChannelNumber=" << mcChannelNumber
                << ", mcEventNumber=" << mcEventNumber
                << ", actualMu=" << actualMu
                << ", averageMu=" << averageMu
                << ", eventTypeBitmask=0x" << std::hex << eventTypeBitmask << std::dec
                << ", extendedLevel1ID=" << extendedLevel1ID
                << ", backgroundFlags=0x" << std::hex << backgroundFlags << std::dec
                << ", nWeights=" << nWeights);

  // Allocate ROD data
  uint32_t* data = newRodData(eventContext, ndata);

  // Fill ROD data
  size_t idx = 0;

  // Word 0: Version
  data[idx++] = FORMAT_VERSION;

  // Word 1: mcChannelNumber
  data[idx++] = mcChannelNumber;

  // Word 2-3: mcEventNumber (low, high)
  data[idx++] = static_cast<uint32_t>(mcEventNumber & 0xFFFFFFFF);
  data[idx++] = static_cast<uint32_t>(mcEventNumber >> 32);

  // Word 4: actualInteractionsPerCrossing
  data[idx++] = std::bit_cast<uint32_t>(actualMu);

  // Word 5: averageInteractionsPerCrossing
  data[idx++] = std::bit_cast<uint32_t>(averageMu);

  // Word 6: eventTypeBitmask
  data[idx++] = eventTypeBitmask;

  // Word 7-8: pileUpMixtureIDLowBits (low, high)
  data[idx++] = static_cast<uint32_t>(pileUpMixtureLow & 0xFFFFFFFF);
  data[idx++] = static_cast<uint32_t>(pileUpMixtureLow >> 32);

  // Word 9-10: pileUpMixtureIDHighBits (low, high)
  data[idx++] = static_cast<uint32_t>(pileUpMixtureHigh & 0xFFFFFFFF);
  data[idx++] = static_cast<uint32_t>(pileUpMixtureHigh >> 32);

  // Word 11: extendedLevel1ID
  data[idx++] = extendedLevel1ID;

  // Word 12: backgroundFlags
  data[idx++] = backgroundFlags;

  // Word 13: number of weights
  data[idx++] = nWeights;

  // Words 14+: mcEventWeights
  for (size_t i = 0; i < nWeights; ++i) {
    data[idx++] = std::bit_cast<uint32_t>(weights[i]);
  }

  // Create ROB fragment
  vrobf.push_back(newRobFragment(eventContext, m_robIds.value().at(0), ndata, data));

  ATH_MSG_DEBUG("Created MC EventInfo ROB fragment with " << ndata << " words");
  return StatusCode::SUCCESS;
}

StatusCode MCEventInfoByteStreamTool::convertFromBS(const ROBF* rob,
                                                    xAOD::EventInfo& evtInfo) const {
  if (!rob) {
    ATH_MSG_ERROR("Null ROB fragment pointer");
    return StatusCode::FAILURE;
  }

  // Get ROD data
  const uint32_t ndata = rob->rod_ndata();
  const uint32_t* data = rob->rod_data();

  ATH_MSG_DEBUG("Decoding MC EventInfo ROB fragment with " << ndata << " words");

  if (ndata < MIN_DATA_WORDS) {
    ATH_MSG_ERROR("ROB data too short: " << ndata << " words, expected at least " << MIN_DATA_WORDS);
    return StatusCode::FAILURE;
  }

  // Read version
  uint32_t version = data[0];
  if ((version & 0xFFFF0000) != (FORMAT_VERSION & 0xFFFF0000)) {
    ATH_MSG_ERROR("Unsupported MC EventInfo format version: 0x" << std::hex << version << std::dec);
    return StatusCode::FAILURE;
  }

  // Read MC fields
  uint32_t mcChannelNumber = data[1];
  uint64_t mcEventNumber = static_cast<uint64_t>(data[2]) | (static_cast<uint64_t>(data[3]) << 32);
  float actualMu = std::bit_cast<float>(data[4]);
  float averageMu = std::bit_cast<float>(data[5]);
  uint32_t eventTypeBitmask = data[6];
  uint64_t pileUpMixtureLow = static_cast<uint64_t>(data[7]) | (static_cast<uint64_t>(data[8]) << 32);
  uint64_t pileUpMixtureHigh = static_cast<uint64_t>(data[9]) | (static_cast<uint64_t>(data[10]) << 32);
  uint32_t extendedLevel1ID = data[11];
  uint32_t backgroundFlags = data[12];
  uint32_t nWeights = data[13];

  // Validate weight count
  if (ndata < MIN_DATA_WORDS + nWeights) {
    ATH_MSG_ERROR("ROB data too short for " << nWeights << " weights: " << ndata << " words");
    return StatusCode::FAILURE;
  }

  // Read weights
  std::vector<float> weights(nWeights);
  for (size_t i = 0; i < nWeights; ++i) {
    weights[i] = std::bit_cast<float>(data[MIN_DATA_WORDS + i]);
  }

  ATH_MSG_DEBUG("Decoded MC EventInfo: mcChannelNumber=" << mcChannelNumber
                << ", mcEventNumber=" << mcEventNumber
                << ", actualMu=" << actualMu
                << ", averageMu=" << averageMu
                << ", eventTypeBitmask=0x" << std::hex << eventTypeBitmask << std::dec
                << ", extendedLevel1ID=" << std::dec << extendedLevel1ID
                << ", backgroundFlags=0x" << std::hex << backgroundFlags << std::dec
                << ", nWeights=" << nWeights);

  // Fill EventInfo
  evtInfo.setMCChannelNumber(mcChannelNumber);
  evtInfo.setMCEventNumber(mcEventNumber);
  evtInfo.setMCEventWeights(weights);
  evtInfo.setActualInteractionsPerCrossing(actualMu);
  evtInfo.setAverageInteractionsPerCrossing(averageMu);
  evtInfo.setEventTypeBitmask(eventTypeBitmask);
  evtInfo.setExtendedLevel1ID(extendedLevel1ID);
  evtInfo.setEventFlags(xAOD::EventInfo::Background, backgroundFlags);

  // Set pileup mixture ID as auxiliary data
  acc_pileUpMixtureLow(evtInfo) = pileUpMixtureLow;
  acc_pileUpMixtureHigh(evtInfo) = pileUpMixtureHigh;

  return StatusCode::SUCCESS;
}

void MCEventInfoByteStreamTool::clearCache(const EventContext& eventContext) {
  m_cache.get(eventContext)->clear();
}

uint32_t* MCEventInfoByteStreamTool::newRodData(const EventContext& eventContext, size_t size) {
  Cache* cache = m_cache.get(eventContext);
  cache->rodData.push_back(std::make_unique<uint32_t[]>(size));
  return cache->rodData.back().get();
}

WROBF* MCEventInfoByteStreamTool::newRobFragment(const EventContext& eventContext,
                                                 uint32_t source_id,
                                                 uint32_t ndata,
                                                 const uint32_t* data) {
  Cache* cache = m_cache.get(eventContext);
  const EventIDBase& eid = eventContext.eventID();
  cache->robFragments.push_back(std::make_unique<WROBF>(
    source_id,
    eid.run_number(),
    0,  // lvl1_id will be overwritten downstream
    eid.bunch_crossing_id(),
    0,  // lvl1_type will be overwritten downstream
    0,  // detev_type
    ndata,
    data,
    eformat::STATUS_BACK
  ));
  return cache->robFragments.back().get();
}
