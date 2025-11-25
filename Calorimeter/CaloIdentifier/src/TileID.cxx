/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Access to Tile Calorimeter raw data
 -----------------------------------------
 ***************************************************************************/


#include "CaloIdentifier/TileID.h"
#include "Identifier/IdentifierHash.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>


TileID::TileID()
  : Tile_Base_ID ("TileID", "tile", false)
{
}

int TileID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
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
  if (Tile_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
    return (1);

  return 0;
}


