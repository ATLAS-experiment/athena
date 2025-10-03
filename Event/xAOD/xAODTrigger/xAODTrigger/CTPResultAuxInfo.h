/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_CTPRESULTAUXINFO_H
#define XAODTRIGGER_CTPRESULTAUXINFO_H

// Local include(s):
#include "xAODTrigger/versions/CTPResultAuxInfo_v1.h"

namespace xAOD{
   // Define the latest version of the CTPResultAuxInfo class
   typedef CTPResultAuxInfo_v1 CTPResultAuxInfo;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::CTPResultAuxInfo , 1199574956 , 1 ) 

#endif // XAODTRIGGER_CTPRESULTAUXINFO_H