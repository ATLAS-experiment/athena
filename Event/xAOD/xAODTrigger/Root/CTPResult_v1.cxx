/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// System include(s):
#include <iostream>
#include <stdexcept>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigger/versions/CTPResult_v1.h"

namespace xAOD {

  // get/set the number of the CTP version to be used
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, ctpVersionNumber, setCtpVersionNumber);

  // Get/set the header marker word
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, headerMarker, setHeaderMarker);

  // Get/set the number of header words
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, headerSize, setHeaderSize);

  // Get/set the version of header format
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, headerFormatVersion, setHeaderFormatVersion);

  // Get/set the sub detector source ID
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, sourceID, setSourceID);

  // Get/set the run number
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, runNumber, setRunNumber);

  // Get/set the extended LVL1 ID
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, L1ID, setL1ID);

  // Get/set the bunch crossing ID
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, BCID, setBCID);

  // Get/set the LVL1 trigger type
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, triggerType, setTriggerType);

  // Get/set the LVL1 event type
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, eventType, setEventType);

  // get/set the vector TIP words for all bunch crossings
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<std::vector<uint32_t>>, tipWords, setTIPWords)

  // get/set the vector TBP words for all bunch crossings
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<std::vector<uint32_t>>, tbpWords, setTBPWords)

  // get/set the vector TAP words for all bunch crossings
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<std::vector<uint32_t>>, tapWords, setTAPWords)

  // get/set the vector TAV words for all bunch crossings
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<std::vector<uint32_t>>, tavWords, setTAVWords)

  // get the time stamp in seconds
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, timeSec, setTimeSec)

  // get the time stamp in nanoseconds
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, timeNanoSec, setTimeNanoSec)

  // get/set the raw data words
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<uint32_t>, additionalWords, setAdditionalWords)

  // get/set the turn counter
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( CTPResult_v1, uint32_t, turnCounter, setTurnCounter )

  // Get/set the error status word
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, errorStatus, setErrorStatus);

  // Get/set the info status word
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, infoStatus, setInfoStatus);

  // Get/set the number of status words
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, numStatusWords, setNumStatusWords);

  // Get/set the number data words
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, numDataWords, setNumDataWords);

  // Get/set the position of status information in ROD (LVL1 assumes 1)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CTPResult_v1, uint32_t, statusPosition, setStatusPosition);

  // get the number of bunches
  AUXSTORE_PRIMITIVE_GETTER(CTPResult_v1, uint32_t, numberOfBunches)

  // get the L1 Accept Bunch Position
  AUXSTORE_PRIMITIVE_GETTER(CTPResult_v1, uint32_t, l1AcceptBunchPosition)

  // Get the CTPBunchCrossing object for a specific bunch in the readout window. Returns L1A bunch by default.
  const CTPResult_v1::CTPBunchCrossing CTPResult_v1::getBC(const int bunch) const {

    // If there are no bunches, then return an empty CTPBunchCrossing struct
    if (numberOfBunches() == 0) return {};

    // Test to see what index to use
    int idx = (bunch == -1) ? l1AcceptBunchPosition() : bunch;

    // Grab words
    const auto& tip = tipWords();
    const auto& tbp = tbpWords();
    const auto& tap = tapWords();
    const auto& tav = tavWords();

    // Check the index is valid, if not then return an empty CTPBunchCrossing struct
    if (idx < 0 ||
        idx >= static_cast<int>(numberOfBunches()) ||
        idx >= static_cast<int>(tbp.size()) ||
        idx >= static_cast<int>(tip.size()) ||
        idx >= static_cast<int>(tap.size()) ||
        idx >= static_cast<int>(tav.size())) {
      return {};
    }

    // Return the appropriate bunch information
    CTPBunchCrossing bc;
    bc.tipWords = tip[idx];
    bc.tbpWords = tbp[idx];
    bc.tapWords = tap[idx];
    bc.tavWords = tav[idx];

    return bc;
  }

  // set the number of bunches
  void CTPResult_v1::setNumberOfBunches(const uint32_t nBCs) {
    static const SG::AuxElement::Accessor< uint32_t > accNumOfBunches("numberOfBunches");
    static const SG::AuxElement::Accessor< std::vector<std::vector<uint32_t> > > accTIPWords("tipWords");
    static const SG::AuxElement::Accessor< std::vector<std::vector<uint32_t> > > accTBPWords("tbpWords");
    static const SG::AuxElement::Accessor< std::vector<std::vector<uint32_t> > > accTAPWords("tapWords");
    static const SG::AuxElement::Accessor< std::vector<std::vector<uint32_t> > > accTAVWords("tavWords");
    accNumOfBunches( *this ) = nBCs;
    accTIPWords( *this ).resize(nBCs);
    accTBPWords( *this ).resize(nBCs);
    accTAPWords( *this ).resize(nBCs);
    accTAVWords( *this ).resize(nBCs);
  }

  // set the L1 Accept Bunch Position
  void CTPResult_v1::setL1AcceptBunchPosition(uint32_t pos) {
    if (numberOfBunches() == 1 && pos > 0) {pos=0;}    
    static const SG::Accessor<uint32_t> acc("l1AcceptBunchPosition");
    acc(*this) = pos;
  }

  // Get TIP words for a specific bunch crossing
  std::vector<uint32_t> CTPResult_v1::getTIPWords(const int bunch) const {
    return getBC(bunch).tipWords;
  }

  // Get TBP words for a specific bunch crossing
  std::vector<uint32_t> CTPResult_v1::getTBPWords(const int bunch) const {
    return getBC(bunch).tbpWords;
  }

  // Get TAP words for a specific bunch crossing
  std::vector<uint32_t> CTPResult_v1::getTAPWords(const int bunch) const {
    return getBC(bunch).tapWords;
  }

  // Get TAV words for a specific bunch crossing
  std::vector<uint32_t> CTPResult_v1::getTAVWords(const int bunch) const {
    return getBC(bunch).tavWords;
  }

  // Get a vector of header words
  const std::vector<uint32_t> CTPResult_v1::header() const {
    return {
      headerMarker(),
      headerSize(),
      headerFormatVersion(),
      sourceID(),
      runNumber(),
      L1ID(),
      BCID(),
      triggerType(),
      eventType()
    };
  }

  // Get the header words
  void CTPResult_v1::setHeader(const uint32_t marker, const uint32_t version, const uint32_t sourceid, const uint32_t l1id, const uint32_t runNum, const uint32_t bcid, const uint32_t trigType, const uint32_t evtType) {
    setHeaderMarker(marker);
    setHeaderSize(8);
    setHeaderFormatVersion(version);
    setSourceID(sourceid);
    setRunNumber(runNum);
    setL1ID(l1id);
    setBCID(bcid);
    setTriggerType(trigType);
    setEventType(evtType);
  }

  // Get a vector of trailer words
  const std::vector<uint32_t> CTPResult_v1::trailer() const {
    return {
      errorStatus(),
      infoStatus(),
      numStatusWords(),
      numDataWords(),
      statusPosition()
    };
  }

  // Set the trailer words
  void CTPResult_v1::setTrailer(const uint32_t numData, const uint32_t errStat, const uint32_t infoStat, const uint32_t numStat, const uint32_t statPos) {
    setErrorStatus(errStat);
    setInfoStatus(infoStat);
    setNumStatusWords(numStat);
    setNumDataWords(numData);
    setStatusPosition(statPos);
  }

 // Check the object using requirements of the CTP data format (for Run 3)
  std::vector<std::string> CTPResult_v1::checkForIssues() const {

    // Create vector to store warnings
    std::vector<std::string> msg;

    // Check for completely empty payload
    if (numberOfBunches() == 0 && additionalWords().empty()) {
      msg.push_back("CTPResult has a completely empty payload!");
    }

    // Expect exactly 2 status words
    if (numStatusWords() != 2) {
      msg.push_back("CTPResult does not have exactly 2 status words!");
    }

    // Both status words must be zero
    if (errorStatus() != 0 || infoStatus() != 0) {
      msg.push_back("CTPResult has non-zero status words!");
    }

    // Status information position must be 1
    if (statusPosition() != 1) {
      msg.push_back("CTPResult has incorrect status info position!");
    }

    // L1 accept position must be valid
    uint32_t nBCs = numberOfBunches(); {
    if (nBCs != 0 && l1AcceptBunchPosition() >= nBCs)
      msg.push_back("CTPResult has an invalid L1A accept position!");
    }

    return msg;
  }
}
