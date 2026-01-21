/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/LArFCAL_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"
#include "LArFCAL_region.h"
#include "PathResolver/PathResolver.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#define MAX_BUFFER_LEN 1024

LArFCAL_ID::LArFCAL_ID() :
  LArFCAL_Base_ID("LArFCAL_ID", "lar_fcal", 0)
{
}

LArFCAL_ID::~LArFCAL_ID() = default;

int  LArFCAL_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
/*=================================================================*/
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
  if (LArFCAL_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
  {
    if(dictionaryVersion() == "H8TestBeam" )
      return 0;
    return (1);
  }

  return 0;

}

