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

  // get the number of bunches
  AUXSTORE_PRIMITIVE_GETTER(CTPResult_v1, uint32_t, numberOfBunches)

  // get the L1 Accept Bunch Position
  AUXSTORE_PRIMITIVE_GETTER(CTPResult_v1, uint32_t, l1AcceptBunchPosition)

  // set the L1 Accept Bunch Position
  void CTPResult_v1::setL1AcceptBunchPosition(const uint32_t pos) {
    if(pos < numberOfBunches()) {
      static const SG::Accessor< uint32_t > acc("l1AcceptBunchPosition");
      acc( *this ) = pos;
    }
  }

  // get the number of additional words
  AUXSTORE_PRIMITIVE_GETTER(CTPResult_v1, uint32_t, numberOfAdditionalWords)

  // get/set the raw data words
  AUXSTORE_OBJECT_SETTER_AND_GETTER(CTPResult_v1, std::vector<uint32_t>, dataWords, setDataWords)

  // get/set the number of the CTP version to be used
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( CTPResult_v1, uint32_t, ctpVersionNumber,
          setCtpVersionNumber )

  // get/set the turn counter
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( CTPResult_v1, uint32_t, turnCounter,
          setTurnCounter )
  
}
