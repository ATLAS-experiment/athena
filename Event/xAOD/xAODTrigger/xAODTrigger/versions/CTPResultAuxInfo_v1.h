/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_VERSIONS_CTPRESULTAUXINFO_V1_H
#define XAODTRIGGER_VERSIONS_CTPRESULTAUXINFO_V1_H

// System include(s):
#include <cstdint>
#include <vector>
#include <string>

// EDM include(s):
#include "xAODCore/AuxInfoBase.h"

// Local include(s):
#include "xAODTrigger/versions/CTPResult_v1.h"

namespace xAOD {

   /**
   * @class CTPResultAuxInfo_v1
   * @brief Auxiliary store for CTPResult_v1.
   *
   * Stores data equivalent to the ROD data fragment such as the header, payload and trailer words. Trigger bits for CTP result stored using CTPBunchCrossing struct.
   */
  
   class CTPResultAuxInfo_v1 : public AuxInfoBase {

   public:
      /// Default constuctor
      CTPResultAuxInfo_v1();

   private:

     uint32_t numberOfBunches;
     uint32_t l1AcceptBunchPosition;
     uint32_t ctpVersionNumber;

     // Header information
     uint32_t headerMarker;           // Header marker word
     uint32_t headerSize;             // Number of words in header
     uint32_t headerFormatVersion;    // Version of header format
     uint32_t sourceID;               // Sub detector source ID
     uint32_t runNumber;              // Run number
     uint32_t L1ID;                   // Extended LVL1 ID
     uint32_t BCID;                   // Bunch crossing ID
     uint32_t triggerType;            // LVL1 trigger type
     uint32_t eventType;              // LVL1 event type

     // Payload information
     uint32_t timeNanoSec;                                // Timestamp in nanoseconds
     uint32_t timeSec;                                    // Timestamp in seconds
     uint32_t turnCounter;                                // Turn counter
     std::vector< std::vector<uint32_t> > tipWords;  // Trigger input words
     std::vector< std::vector<uint32_t> > tbpWords;  // Trigger before prescale words
     std::vector< std::vector<uint32_t> > tapWords;  // Trigger after prescale words
     std::vector< std::vector<uint32_t> > tavWords;  // Trigger after veto words 
     std::vector< uint32_t > additionalWords;             // Additional words

     // Trailer information
     uint32_t errorStatus;     // Error status word
     uint32_t infoStatus;      // Info status eord
     uint32_t numStatusWords;  // Number of status words
     uint32_t numDataWords;    // Number of data words
     uint32_t statusPosition;  // Position of status information in ROD (LVL1 assumes 1)

   }; // class CTPResultAuxInfo_v1

} // namespace xAOD

// Declare the inheritance of the type:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::CTPResultAuxInfo_v1, xAOD::AuxInfoBase );

#endif // XAODTRIGGER_VERSIONS_CTPRESULTAUXINFO_V1_H
