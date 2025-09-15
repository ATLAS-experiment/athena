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
    AUX_VARIABLE( ctpVersionNumber );
    AUX_VARIABLE( headerMarker );
    AUX_VARIABLE( headerSize );
    AUX_VARIABLE( headerFormatVersion );
    AUX_VARIABLE( sourceID );
    AUX_VARIABLE( runNumber );
    AUX_VARIABLE( L1ID );
    AUX_VARIABLE( BCID );
    AUX_VARIABLE( triggerType );
    AUX_VARIABLE( eventType );
    AUX_VARIABLE( timeSec );
    AUX_VARIABLE( timeNanoSec );
    AUX_VARIABLE( turnCounter );
    AUX_VARIABLE( tipWords );
    AUX_VARIABLE( tbpWords );
    AUX_VARIABLE( tapWords );
    AUX_VARIABLE( tavWords );
    AUX_VARIABLE( additionalWords );
    AUX_VARIABLE( errorStatus );
    AUX_VARIABLE( infoStatus );
    AUX_VARIABLE( numStatusWords );
    AUX_VARIABLE( numDataWords );
    AUX_VARIABLE( statusPosition );
  }

} // namespace xAOD
