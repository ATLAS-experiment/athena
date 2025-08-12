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
    * @brief Get the number of bunch crossings.
    * @return Number of BCs.
    */
    uint32_t numberOfBunches() const;

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
    * @brief Get the number of additional data words.
    * @return Number of extra words.
    */
    uint32_t numberOfAdditionalWords() const;

    /**
    * @brief Get the raw data words.
    * @return Vector of data words.
    */
    const std::vector< uint32_t >& dataWords() const;

    /**
    * @brief Set the raw data words.
    * @param words Vector of raw data words.
    */
    void setDataWords(const std::vector< uint32_t >& words);
    
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
    * @brief Get the turn counter.
    * @return Turn counter value.
    */
    uint32_t turnCounter() const;

    /**
    * @brief Set the turn counter.
    * @param val Turn counter value.
    */
    void setTurnCounter(const uint32_t val);

  }; // class
  
} // namespace xAOD

#endif // XAODTRIGGER_VERSIONS_CTPRESULT_V1_H


