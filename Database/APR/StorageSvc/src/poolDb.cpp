/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  Package    : System (The POOL project)
//
//  Description: Misc methods
//
//  @author      M.Frank
//====================================================================

#include "StorageSvc/pool.h"
#include <cstdlib>

/// Translate access mode to string
const char* pool::accessMode(Io::IoFlag mode)   {
  if      ( mode == Io::READ      ) 
    return "READ     ";
  else if ( mode == Io::WRITE     )
    return "WRITE    ";
  else if ( mode == Io::APPEND    )
    return "APPEND   ";
  else if ( mode == Io::INVALID  ) 
    return "NOT_OPEN ";
  else                                 
    return "UNKNOWN  ";
}


std::string pool::getEnvStr(const std::string& key) {
  const char *var = getenv( key.c_str() );
  return var? std::string(var) : std::string();
}
