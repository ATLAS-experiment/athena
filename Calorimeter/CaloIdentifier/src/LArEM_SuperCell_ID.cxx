/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CaloIdentifier/src/LArEM_SuperCell_ID.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2012
 * @brief Helper class for LArEM offline identifiers for supercells
 */


#include "CaloIdentifier/LArEM_SuperCell_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"
#include "LArEM_region.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>



LArEM_SuperCell_ID::LArEM_SuperCell_ID() :
  LArEM_Base_ID("LArEM_SuperCell_ID", "slar_em", 1)
{
}

LArEM_SuperCell_ID::~LArEM_SuperCell_ID() = default;

int  LArEM_SuperCell_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
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
  if (LArEM_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
    return (1);

  return 0;
}

