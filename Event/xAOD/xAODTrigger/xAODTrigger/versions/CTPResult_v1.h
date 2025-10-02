/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_VERSIONS_CTPRESULT_V1_H
#define XAODTRIGGER_VERSIONS_CTPRESULT_V1_H

// System include(s):
#include <cstdint>
#include <vector>

// EDM include(s):
#include "AthContainers/AuxElement.h"

namespace xAOD {

  /** 
  * @class CTPResult_v1
  * @brief This is the trigger result for each item before prescale, after prescale and after veto.
  * Utility functions for xAOD::CTPResult objects are defined in TrigT1Interfacts/CTPResultUtils.h that depend on tdaq-common code.
  */
  
  class CTPResult_v1 : public SG::AuxElement {
    
  public:

    /**
    * @struct CTPBunchCrossing
    * @brief This is the trigger result for each item before prescale, after prescale and after veto for a single bunch crossing.
    */
    struct CTPBunchCrossing {
      std::vector< uint32_t > tipWords{0};   // Trigger input words for the bunch crossing
      std::vector< uint32_t > tbpWords{0};   // Trigger before prescale words for the bunch crossing
      std::vector< uint32_t > tapWords{0};   // Trigger after prescale words for the bunch crossing
      std::vector< uint32_t > tavWords{0};   // Trigger after veto words for the bunch crossing
    };

    /**
    * @brief Get the CTP version number.
    * @return CTP version number.
    */
    uint32_t ctpVersionNumber() const;

    /**
    * @brief Set the CTP version number.
    * @param ctpNumber Version number to set.
    */
    void setCtpVersionNumber(const uint32_t ctpNumber);

    /**
    * @brief Retrieve the header marker word.
    * @return Header marker word.
    */
    uint32_t headerMarker() const;

    /**
    * @brief Retrieve the number of words in the header.
    * @return Header size in words.
    */
    uint32_t headerSize() const;

    /**
    * @brief Retrieve the format version of the header.
    * @return Format version word.
    */
    uint32_t headerFormatVersion() const;

    /**
    * @brief Retrieve the sub-detector source ID.
    * @return Source ID word.
    */
    uint32_t sourceID() const;

    /**
    * @brief Retrieve the run number.
    * @return Run number word.
    */
    uint32_t runNumber() const;

    /**
    * @brief Retrieve the extended LVL1 ID.
    * @return LVL1 ID word.
    */
    uint32_t L1ID() const;

    /**
    * @brief Retrieve the bunch crossing ID.
    * @return BCID
    */
    uint32_t BCID() const;

    /**
    * @brief Retrieve the LVL1 trigger type.
    * @return Trigger type word.
    */
    uint32_t triggerType() const;

    /**
    * @brief Retrieve the LVL1 event type.
    * @return Event type word.
    */
    uint32_t eventType() const;

    /**
    * @brief Set the header marker word.
    * @param word Header marker to set.
    */
    void setHeaderMarker(const uint32_t word);

    /**
    * @brief Set the number of words in the header.
    * @param word Header size in words.
    */
    void setHeaderSize(const uint32_t word);

    /**
    * @brief Set the format version of the header.
    * @param word Format version to set.
    */
    void setHeaderFormatVersion(const uint32_t word);

    /**
    * @brief Set the sub-detector source ID.
    * @param word Source ID to set.
    */
    void setSourceID(const uint32_t word);

    /**
    * @brief Set the run number.
    * @param word Run number to set.
    */
    void setRunNumber(const uint32_t word);

    /**
    * @brief Set the extended LVL1 ID.
    * @param word LVL1 ID to set.
    */
    void setL1ID(const uint32_t word);

    /**
    * @brief Set the bunch crossing ID.
    * @param word BCID to set.
    */
    void setBCID(const uint32_t word);

    /**
    * @brief Set the LVL1 trigger type.
    * @param word Trigger type to set.
    */
    void setTriggerType(const uint32_t word);

    /**
    * @brief Set the LVL1 event type.
    * @param word Event type to set.
    */
    void setEventType(const uint32_t word);

    /**
    * @brief Get the CTPBunchCrossing object for a specific bunch in the readout window.
    * @return Read-only CTPBunchCrossing object.
    */
    const CTPResult_v1::CTPBunchCrossing getBC(const int bunch=-1) const;

    /**
    * @brief Get the number of bunch crossings.
    * @return Number of BCs.
    */
    uint32_t numberOfBunches() const;

    /**
    * @brief Set the number of bunch crossings.
    * @param nBCs Number of bunch crossings.
    */
    void setNumberOfBunches(const uint32_t nBCs);

    /**
    * @brief Get the L1 accept bunch position.
    * @return Bunch position for the L1 accept.
    */ 
    uint32_t l1AcceptBunchPosition() const;
    
    /**
    * @brief Set the L1Accept bunch position.
    * @param pos Bunch position.
    */
    void setL1AcceptBunchPosition(const uint32_t pos);

    /**
    * @brief Get the TIP words for all bunch crossings.
    * @return Vector of vectors for all TIP words for all bunches.
    */
    const std::vector< std::vector< uint32_t > >& tipWords() const;

    /**
    * @brief Set the TIP words for all bunch crossings.
    * @param words Vector of vectors for all TIP words for all bunches.
    */
    void setTIPWords(const std::vector< std::vector< uint32_t > >& words);

    /**
    * @brief Get the TBP words for all bunch crossings.
    * @return Vector of vectors for all TBP words for all bunches.
    */
    const std::vector< std::vector< uint32_t > >& tbpWords() const;

    /**
    * @brief Set the TBP words for all bunch crossings.
    * @param words Vector of vectors for all TBP words for all bunches.
    */
    void setTBPWords(const std::vector< std::vector< uint32_t > >& words);

    /**
    * @brief Get the TAP words for all bunch crossings.
    * @return Vector of vectors for all TAP words for all bunches.
    */
    const std::vector< std::vector< uint32_t > >& tapWords() const;

    /**
    * @brief Set the TAP words for all bunch crossings.
    * @param words Vector of vectors for all TAP words for all bunches.
    */
    void setTAPWords(const std::vector< std::vector< uint32_t > >& words);

    /**
    * @brief Get the TAV words for all bunch crossings.
    * @return Vector of vectors for all TAV words for all bunches.
    */
    const std::vector< std::vector< uint32_t > >& tavWords() const;

    /**
    * @brief Set the TAV words for all bunch crossings.
    * @param words Vector of vectors for all TAV words for all bunches.
    */
    void setTAVWords(const std::vector< std::vector< uint32_t > >& words);
    
    /**
    * @brief Get the time in seconds.
    * @return Time in seconds.
    */
    uint32_t timeSec() const;

    /**
    * @brief Set the time in seconds.
    * @param sec Time in seconds.
    */
    void setTimeSec(const uint32_t sec);

    /**
    * @brief Get the time in nanoseconds.
    * @return Time in nanoseconds.
    */
    uint32_t timeNanoSec() const;

    /**
    * @brief Set the time in nanoseconds.
    * @param nano Time in nanoseconds.
    */
    void setTimeNanoSec(const uint32_t nano);

    /**
    * @brief Get the number of additional data words.
    * @return Number of extra words.
    */
    uint32_t numberOfAdditionalWords() const;

    /**
    * @brief Set the number of additional data words.
    * @param nExtraWords Number of extra words.
    */
    void setNumberOfAdditionalWords(const uint32_t nExtraWords);

    /**
    * @brief Get the additional data words.
    * @return Vector of additional data words.
    */
    const std::vector< uint32_t >& additionalWords() const;

    /**
    * @brief Set the additional data words.
    * @param words Vector of additional data words.
    */
    void setAdditionalWords(const std::vector< uint32_t >& words);

    /**
    * @brief Get the turn counter.
    * @return Turn counter value.
    */
    uint32_t turnCounter() const;

    /**
    * @brief Set the turn counter.
    * @param val Turn counter value.
    */
    void setTurnCounter(const uint32_t val);

    /**
    * @brief Get the TIP (Trigger Inputs to the CTP) words (in Run3 512 items)
    * @return Vector of TIP words.
    */
    std::vector< uint32_t > getTIPWords(const int bunchPosition=-1) const;

    /**
    * @brief Get the TBP (Trigger Before Prescale) words.
    * @return Vector of TBP words.
    */
    std::vector< uint32_t > getTBPWords(const int bunchPosition=-1) const;

    /**
    * @brief Get the TAP (Trigger After Prescale) words.
    * @return Vector of TAP words.
    */
    std::vector< uint32_t > getTAPWords(const int bunchPosition=-1) const;

    /**
    * @brief Get the TAV (Trigger After Veto) words.
    * @return Vector of TAV words.
    */
    std::vector< uint32_t > getTAVWords(const int bunchPosition=-1) const;

    /**
    * @brief Retrieve the error status word.
    * @return Error status word.
    */
    uint32_t errorStatus() const;

    /**
    * @brief Retrieve the info status word.
    * @return Info status word.
    */
    uint32_t infoStatus() const;

    /**
    * @brief Retrieve the number of status words in the trailer.
    * @return Number of status words.
    */
    uint32_t numStatusWords() const;

    /**
    * @brief Retrieve the number of data words.
    * @return Number of data words.
    */
    uint32_t numDataWords() const;

    /**
    * @brief Retrieve the position of status information in the ROD.
    * LVL1 assumes this value is 1.
    * @return Position of status information.
    */
    uint32_t statusPosition() const;

    /**
    * @brief Set the error status word.
    * @param word Error status to set.
    */
    void setErrorStatus(const uint32_t word);

    /**
    * @brief Set the info status word.
    * @param word Info status to set.
    */
    void setInfoStatus(const uint32_t word);

    /**
    * @brief Set the number of status words in the trailer.
    * @param word Number of status words to set.
    */
    void setNumStatusWords(const uint32_t word);

    /**
    * @brief Set the position of status information in the ROD.
    * LVL1 assumes this value is 1.
    * @param word Status position to set.
    */
    void setStatusPosition(const uint32_t word);

    /**
    * @brief Set the number of data words.
    * @param num Number of data words to set.
    */
    void setNumDataWords(const uint32_t num);

  }; // class CTPResult_v1
  
} // namespace xAOD

#endif // XAODTRIGGER_VERSIONS_CTPRESULT_V1_H


