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

namespace xAOD {

   /**
   * @class CTPResultAuxInfo_v1
   * @brief Auxiliary store for CTPResult_v1.
   *
   * Stores data such as the number of bunches, L1A position, and raw CTP data words.
   */
  
   class CTPResultAuxInfo_v1 : public AuxInfoBase {

   public:
      /// Default constuctor
      CTPResultAuxInfo_v1();

   private:
     uint32_t numberOfBunches;
     uint32_t l1AcceptBunchPosition;
     uint32_t turnCounter;
     uint32_t numberOfAdditionalWords;
     uint32_t ctpVersionNumber;
     std::vector< uint32_t > dataWords;

   }; // class CTPResultAuxInfo_v1

} // namespace xAOD

// Declare the inheritance of the type:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::CTPResultAuxInfo_v1, xAOD::AuxInfoBase );

#endif // XAODTRIGGER_VERSIONS_CTPRESULTAUXINFO_V1_H
