/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CaloIdentifier/LArHEC_SuperCell_ID.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2013
 * @brief Helper class for LArHEC offline identifiers for supercells
 */


#include "CaloIdentifier/LArHEC_SuperCell_ID.h"


LArHEC_SuperCell_ID::LArHEC_SuperCell_ID()
  : LArHEC_Base_ID ("LArHEC_SuperCell_ID", "slar_hec", true)
{
}


int  LArHEC_SuperCell_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
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
  if (LArHEC_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
    return (1);

  return 0;
}

