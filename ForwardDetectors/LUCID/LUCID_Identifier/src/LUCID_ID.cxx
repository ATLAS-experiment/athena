/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "LUCID_Identifier/LUCID_ID.h"
#include "LUCID_Identifier/LUCID_DetElemHash.h"
#include "Identifier/IdentifierHash.h"
#include <set>
#include <algorithm>
#include <iostream>
#include  <cassert>


//______________________________________________________
LUCID_ID::LUCID_ID()
  : AtlasDetectorID ("LUCID_ID", "")
{
} 
//______________________________________________________
LUCID_ID::~LUCID_ID(){

}
