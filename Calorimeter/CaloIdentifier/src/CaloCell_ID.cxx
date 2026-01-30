/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/CaloCell_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"

#include <set>
#include <iostream>
#include <stdexcept>

CaloCell_ID::CaloCell_ID(const LArEM_ID*        em_id, 
                         const LArHEC_ID*       hec_id, 
                         const LArFCAL_ID*      fcal_id, 
                         const LArMiniFCAL_ID*  minifcal_id,
                         const TileID*          tile_id) : 
  CaloCell_Base_ID ("CaloCell_Base_ID",
                    em_id, hec_id, fcal_id, minifcal_id, tile_id, false)
{
}


CaloCell_ID::~CaloCell_ID() = default;


int
CaloCell_ID::initialize_from_dictionary(const IdDictMgr& dict_mgr)
{
  ATH_MSG_DEBUG("Initialize");
  
  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
    return (0);
  }
  else {
    ATH_MSG_DEBUG("(Re)initialize");
  }
  
  // init base object
  if(CaloCell_Base_ID::initialize_from_dictionary(dict_mgr)) return (1);
  
  return (0);
}

