/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/LArHEC_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"
#include "LArHEC_region.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>


LArHEC_ID::LArHEC_ID() :
  LArHEC_Base_ID ("LArHEC_ID", "lar_hec", false)
{
}

LArHEC_ID::~LArHEC_ID() = default;

int  LArHEC_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
/*===================================================================*/
{
  ATH_MSG_DEBUG("initialize_from_dictionary");

  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
    return (0);
  }
  else {
    ATH_MSG_DEBUG("(Re)initialize");
  }

  // init base object
  if (LArHEC_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
    return (1);

    
  return 0;
}



