/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigger/versions/CTPResultAuxInfo_v1.h"

namespace xAOD {

  CTPResultAuxInfo_v1::CTPResultAuxInfo_v1()
    : AuxInfoBase() {
   
    AUX_VARIABLE( numberOfBunches );
    AUX_VARIABLE( l1AcceptBunchPosition );
    AUX_VARIABLE( turnCounter );
    AUX_VARIABLE( numberOfAdditionalWords );
    AUX_VARIABLE( dataWords );
    AUX_VARIABLE( ctpVersionNumber );

  }

} // namespace xAOD
