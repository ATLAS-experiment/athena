/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1INTERFACES_CTPRESULTUTILS_H
#define TRIGT1INTERFACES_CTPRESULTUTILS_H

// xAOD include
#include "xAODTrigger/CTPResult.h"

/**
* @brief Utility functions for xAOD::CTPResult objects that rely on tdaq-common.
* Declared here as tdaq-common is used in TrigT1Interfaces and not xAODTrigger. Prevents using XAOD_ANALYSIS macro in xAOD class.
*/

namespace CTPResultUtils {

  /**
  * @brief Enum describing the different word types in the CTP result.
  *
  * - TIP: Trigger inputs to the CTP.
  * - TBP: Trigger Before Prescale.
  * - TAP: Trigger After Prescale.
  * - TAV: Trigger After Veto.
  * - Extra: Additional data words beyond the standard categories.
  */
  enum class WordType {TIP, TBP, TAP, TAV, Extra};

    /**
    * @brief Initialize with a number of bunch crossings of the readout window.
    * @param ctpVersionNumber The version number of the CTP data format.
    * @param nBCs Number of bunch crossings in the readout window.
    * @param nExtraWords Number of extra data words.
    */
    void initialize(xAOD::CTPResult& ctpRes, uint32_t ctpVersionNumber, const uint32_t nBCs = 1,  uint32_t nExtraWords=0);

    /**
    * @brief Initialize with raw data words from one or more bunch crossings.
    * @param ctpVersionNumber The version number of the CTP data format.
    * @param data Vector of raw data words.
    * @param nExtraWords Number of extra data words.
    */
    void initialize(xAOD::CTPResult& ctpRes, uint32_t ctpVersionNumber, const std::vector<uint32_t>& data, uint32_t nExtraWords=0);

    /**
    * @brief Get the time in seconds.
    * @return Time in seconds.
    */
    uint32_t getTimeSec(const xAOD::CTPResult& ctpRes);

    /**
    * @brief Set the time in seconds.
    * @param sec Time in seconds.
    */
    void setTimeSec(xAOD::CTPResult& ctpRes, const uint32_t sec);

    /**
    * @brief Get the time in nanoseconds.
    * @return Time in nanoseconds.
    */
    uint32_t getTimeNanoSec(const xAOD::CTPResult& ctpRes);

    /**
    * @brief Set the time in nanoseconds.
    * @param nano Time in nanoseconds.
    */
    void setTimeNanoSec(xAOD::CTPResult& ctpRes, const uint32_t nano);

    /**
    * @brief Get the time since the last 1 accept.
    * @return Time since the last L1 accept.
    */
    uint32_t getTimeSinceLastL1A(const xAOD::CTPResult& ctpRes);

    /**
    * @brief Set the number of bunch crossings.
    * @param nBCs Number of bunch crossings.
    */
    void setNumberOfBunches(xAOD::CTPResult& ctpRes, const uint32_t nBCs);

    /**
    * @brief Set the number of additional data words.
    * @param nExtraWords Number of extra words.
    */
    void setNumberOfAdditionalWords(xAOD::CTPResult& ctpRes, const uint32_t nExtraWords);

    /**
    * @brief Get the TIP (Trigger Inputs to the CTP) words (in Run3 512 items)
    * @return Vector of TIP words.
    */
    std::vector< uint32_t > getTIPWords(const xAOD::CTPResult& ctpRes);
  
    /**
    * @brief Get the TBP (Trigger Before Prescale) words.
    * @return Vector of TBP words.
    */ 
    std::vector< uint32_t > getTBPWords(const xAOD::CTPResult& ctpRes);

    /**
    * @brief Get the TAP (Trigger After Prescale) words.
    * @return Vector of TAP words.
    */
    std::vector< uint32_t > getTAPWords(const xAOD::CTPResult& ctpRes);

    /**
    * @brief Get the TAV (Trigger After Veto) words.
    * @return Vector of TAV words.
    */
    std::vector< uint32_t > getTAVWords(const xAOD::CTPResult& ctpRes);
  
    /**
    * @brief Get additional words.
    * @return Vector of extra words.
    */
    std::vector< uint32_t > getExtraWords(const xAOD::CTPResult& ctpRes);
    
    /**
    * @brief Helper to retrieve specific words.
    * @param type The word type to retrieve.
    * @return Vector of words of the specified type.
    */
    std::vector<uint32_t> getWords(const xAOD::CTPResult& ctpRes, CTPResultUtils::WordType type);

} // namespace CTPResultUtils

#endif // TRIGT1INTERFACES_CTPRESULTUTILS_H