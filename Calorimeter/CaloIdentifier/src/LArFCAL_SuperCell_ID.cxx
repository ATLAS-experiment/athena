/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/LArFCAL_SuperCell_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>

LArFCAL_SuperCell_ID::LArFCAL_SuperCell_ID() :
  LArFCAL_Base_ID("LArFCAL_SuperCell_ID", "slar_fcal", 1)
{
}

LArFCAL_SuperCell_ID::~LArFCAL_SuperCell_ID() = default;

int  LArFCAL_SuperCell_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
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
    return (1);
  
  return 0;
}

