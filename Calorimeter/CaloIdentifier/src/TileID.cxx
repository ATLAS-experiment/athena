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

#include "GaudiKernel/MsgStream.h"

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
  MsgStream log(m_msgSvc, "TileID" );
  
  std::string strg = "initialize_from_dictionary";
  log << MSG::DEBUG << strg << endmsg;

  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    log << MSG::DEBUG << "Request to reinitialize not satisfied - tags have not changed" << endmsg;
    return (0);
  }
  else {
    log << MSG::DEBUG << "(Re)initialize" << endmsg;
  }

  // init base object
  if (Tile_Base_ID::initialize_base_from_dictionary(dict_mgr, group()))
    return (1);

  return 0;
}


