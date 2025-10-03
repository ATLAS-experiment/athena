/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1INTERFACES_CTPRESULTUTILS_H
#define TRIGT1INTERFACES_CTPRESULTUTILS_H

// xAOD include
#include "xAODTrigger/CTPResult.h"

// Forward declaration(s):
class MsgStream;

/**
* @brief Utility functions for xAOD::CTPResult objects that rely on tdaq-common.
* Declared here as tdaq-common is used in TrigT1Interfaces and not xAODTrigger. Prevents using XAOD_ANALYSIS macro in xAOD class.
*/

namespace CTPResultUtils {

  /**
  * @brief Initialize the object using xAOD::Header, xAOD::Trailer, and the payload data. Optionally, extra words can be included.
  *
  * @param ctpRes Reference to the CTPResult object to initialize.
  * @param ctpVersionNumber Version number of the CTPdataformat.
  * @param data Vector of words representing the payload data (excluding timestamp words).
  * @param nExtraWords Number of additional words (default is 0).
  */
  void initialize(xAOD::CTPResult& ctpRes, const uint32_t ctpVersionNumber, std::vector<uint32_t>& data, const uint32_t nExtraWords=0);

  /**
  * @brief Print object content to default message stream
  */
  void dumpData(xAOD::CTPResult& ctpRes);

  /**
  * @brief Print object content to given message stream
  * @param log Message stream.
  */
  void dumpData(xAOD::CTPResult& ctpRes, MsgStream& log);

} // namespace CTPResultUtils

#endif // TRIGT1INTERFACES_CTPRESULTUTILS_H