/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "CTPResultByteStreamTool.h"

// TDAQ include(s):
#include "eformat/SourceIdentifier.h"
#include "CTPfragment/CTPfragment.h"
#include "CTPfragment/CTPdataformat.h"
#include "CTPfragment/CTPdataformatVersion.h"

// Trigger include(s):
#include "xAODTrigger/CTPResult.h"
#include "xAODTrigger/CTPResultAuxInfo.h"
#include "TrigT1Interfaces/CTPResultUtils.h"

// Athena includes
#include "CxxUtils/span.h"

using DataType = OFFLINE_FRAGMENTS_NAMESPACE::PointerType;
using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

// -----------------------------------------------------------------------------
// Initialisation
// -----------------------------------------------------------------------------
StatusCode CTPResultByteStreamTool::initialize() {
  ConversionMode mode = getConversionMode(m_inKeyCTPResult, m_outKeyCTPResult, msg());
  ATH_CHECK(mode!=ConversionMode::Undefined);
  ATH_CHECK(m_outKeyCTPResult.initialize(mode==ConversionMode::Decoding));
  ATH_CHECK(m_inKeyCTPResult.initialize(mode==ConversionMode::Encoding));
  ATH_MSG_DEBUG((mode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " ROB IDs: "
                  << MSG::hex << m_robIds.value() << MSG::dec);
  if (m_robIds.value().size() != 1) {
      ATH_MSG_ERROR("This tool implementation assumes there is exactly one CTP ROB, but "
          << m_robIds.size() << " were configured");
      return StatusCode::SUCCESS;
  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------
// BS->xAOD conversion
// -----------------------------------------------------------------------------
StatusCode CTPResultByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& eventContext) const {

  //
  // Find the ROB fragment to decode
  //
  const eformat::helper::SourceIdentifier sid(m_robIds.value().at(0));
  auto it = std::find_if(vrobf.begin(), vrobf.end(), [&sid](const ROBF* rob){return rob->rob_source_id() == sid.code();});
  if (it == vrobf.end()) {
    ATH_MSG_ERROR("No CTP ROB fragment with ID 0x" << std::hex << sid.code() << std::dec << " was found!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("Found CTP ROD with source ID 0x" << MSG::hex << sid.code() << MSG::dec);

  //
  // Retrieve the ROD data
  //
  const ROBF* rob = *it;
  DataType it_data;
  rob->rod_data( it_data );
  const uint32_t ndata = rob->rod_ndata();

  //
  // Header information
  //
  uint32_t headerMarker = eformat::ROD;
  uint32_t formatVersion = rob->rod_version();
  uint32_t L1ID = rob->rod_lvl1_id();
  uint32_t sourceID = rob->rod_source_id();

  //
  // Trailer information
  //
  DataStatus dataStatus;
  dataStatus.rob_error = false;
  dataStatus.rod_error = false;
  dataStatus.status_word = 0;
  dataStatus.status_info = 0;

  try {
    if (rob->check_rob()) ATH_MSG_VERBOSE("ROB fragment checked ok");
  }
  catch (std::exception const & ex) {
    ATH_MSG_WARNING("ROB fragment not valid: " << ex.what());
    dataStatus.rob_error = true;
  }

  try {
    if (rob->check_rod()) ATH_MSG_VERBOSE("ROD fragment checked ok");
  }
  catch (std::exception const & ex) {
    ATH_MSG_WARNING("ROD fragment not valid: " << ex.what());
    dataStatus.rod_error = true;
  }

  // Process the status words
  DataType status;
  rob->rod_status(status);
  uint32_t nstatus = rob->rod_nstatus();
  if (msgLvl(MSG::VERBOSE)) {
    ATH_MSG_VERBOSE("Number of status words: " << nstatus);
    for (uint32_t i=0; i<nstatus; ++i, ++status) {
      ATH_MSG_VERBOSE("   Status word: 0x" << MSG::hex << std::setw(8) << std::setfill('0') << *status);
    }
  }
  rob->rod_status(status);

  if(nstatus > 0) dataStatus.status_word = static_cast<uint32_t>(*status);
  if(nstatus > 1) {
      ++status;
      dataStatus.status_info = static_cast<uint32_t>(*status);
  }

  uint32_t errStatus = dataStatus.status_word;  // error status
  uint32_t statInfo = dataStatus.status_info;   // status info

  //
  // Payload information
  //
  std::vector<uint32_t> vDataWords;
  vDataWords.reserve(ndata);
  if (msgLvl(MSG::VERBOSE)) {
    ATH_MSG_VERBOSE("   Dumping xAOD::CTPResult Words:");
    for (const uint32_t word : CxxUtils::span{it_data, ndata}) {
      ATH_MSG_VERBOSE("       0x" << MSG::hex << std::setfill('0') << std::setw(8) << word << MSG::dec);
    }
  }
  vDataWords.assign(CxxUtils::span{it_data, ndata}.begin(), CxxUtils::span{it_data, ndata}.end());
  
  uint32_t nExtraWords=0;
  nExtraWords=CTPfragment::numberExtraPayloadWords(rob);
  unsigned int ctpVersionNumber = CTPfragment::ctpFormatVersion(rob);

  //
  // Create xAOD
  //
  auto result = std::make_unique<xAOD::CTPResult>();
  auto resultAux = std::make_unique<xAOD::CTPResultAuxInfo>();
  result->setStore(resultAux.get());

  // Initialize takes care of the most payload info
  CTPResultUtils::initialize(*result, ctpVersionNumber, vDataWords, nExtraWords);
  
  // Initialize the remaining words
  result->setHeader(headerMarker, formatVersion, sourceID, L1ID);       // Header words
  result->setTrailer(ndata, errStatus, statInfo);                       // Trailer words
  result->setL1AcceptBunchPosition(CTPfragment::lvl1AcceptBunch(rob));  // L1A bunch position
  result->setTurnCounter(CTPfragment::turnCounter(rob));                // Turn counter

  // Record result
  SG::WriteHandle<xAOD::CTPResult> ctpResultWriteHandle = SG::makeHandle( m_outKeyCTPResult, eventContext );
  ATH_CHECK( ctpResultWriteHandle.record( std::move(result), std::move(resultAux) ));
  ATH_MSG_DEBUG("Created xAOD::CTPResult from bytestream");

  return StatusCode::SUCCESS;
}


// -----------------------------------------------------------------------------
// xAOD->BS conversion
// -----------------------------------------------------------------------------
StatusCode CTPResultByteStreamTool::convertToBS(std::vector<WROBF*>& vrobf, const EventContext& eventContext) {

  // Retrieve the xAOD::CTPResult object
  SG::ReadHandle<xAOD::CTPResult> result = SG::makeHandle<xAOD::CTPResult>(m_inKeyCTPResult, eventContext);
  ATH_CHECK(result.isValid());

  // Get CTP version
  unsigned int ctpVersionNumber = result->ctpVersionNumber();
  CTPdataformatVersion ctpDataFormat(ctpVersionNumber);

  // Compute rodSize from object content
  size_t totalPerBunchWords = 0;
  for (uint32_t bunch = 0; bunch < result->numberOfBunches(); ++bunch) {
    totalPerBunchWords += result->getTIPWords(bunch).size();
    totalPerBunchWords += result->getTBPWords(bunch).size();
    totalPerBunchWords += result->getTAPWords(bunch).size();
    totalPerBunchWords += result->getTAVWords(bunch).size();
  }

  size_t rodSize = ctpDataFormat.getNumberTimeWords()   // time words
                  + totalPerBunchWords                  // TIP/TBP/TAP/TAV words
                  + result->additionalWords().size();   // extra words

  // Allocate memory
  clearCache(eventContext);
  uint32_t* data = newRodData(eventContext, rodSize);
  std::vector<uint32_t> vDataWords(rodSize, 0);

  size_t idx = 0;

  // Time words
  if (ctpDataFormat.getNumberTimeWords() >= 2) {
    vDataWords[idx++] = result->timeNanoSec();
    vDataWords[idx++] = result->timeSec();
  } else {
    ATH_MSG_WARNING("Unexpected number of time words: " << ctpDataFormat.getNumberTimeWords());
  }

  // TIP/TBP/TAP/TAV words per bunch
  for (uint32_t bunch = 0; bunch < result->numberOfBunches(); ++bunch) {

    // Helper lambda function
    auto copyWords = [&](std::vector<uint32_t> words, const char* tag) -> StatusCode {
      for (size_t i = 0; i < words.size(); ++i) {
        if (idx >= vDataWords.size()) {
          ATH_MSG_ERROR(tag << " write out-of-bounds: idx = " << idx << " >= ROD size = " << vDataWords.size());
          return StatusCode::FAILURE;
        }
        vDataWords[idx++] = words[i];
      }
      return StatusCode::SUCCESS;
    };

    ATH_CHECK(copyWords(result->getTIPWords(bunch), "TIP"));
    ATH_CHECK(copyWords(result->getTBPWords(bunch), "TBP"));
    ATH_CHECK(copyWords(result->getTAPWords(bunch), "TAP"));
    ATH_CHECK(copyWords(result->getTAVWords(bunch), "TAV"));
  }

  // Additional words
  for (size_t i = 0; i < result->additionalWords().size(); ++i) {
    if (idx >= vDataWords.size()) {
      ATH_MSG_ERROR("Additional word write out-of-bounds at idx = " << idx);
      return StatusCode::FAILURE;
    }
    vDataWords[idx++] = result->additionalWords()[i];
  }

  // Sanity check that all words accounted for given xAOD::CTPResult object
  ATH_CHECK(idx == rodSize);

  // Fill ROD
  for (size_t i = 0; i < rodSize; ++i) {
    data[i] = vDataWords[i];
  }

  // Create ROB fragment
  vrobf.push_back(newRobFragment(eventContext, result->sourceID(), rodSize, data, m_detEvType));
  OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment* rob = vrobf.back();
  rob->rod_minor_version(result->headerFormatVersion() & 0xffffu);

  ATH_MSG_DEBUG("Encoded xAOD::CTPResult to bytestream");

  return StatusCode::SUCCESS;
}