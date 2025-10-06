/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistencySvc/DatabaseConnectionPolicy.h"
#include <format>
#include <stdexcept>

constexpr const char* const CompName = "PersistencySvc::DatabaseConnectionPolicy";
using namespace std;

bool
pool::DatabaseConnectionPolicy::setWriteModeForExisting( pool::DatabaseConnectionPolicy::Mode mode )
{
  if ( mode == pool::DatabaseConnectionPolicy::CREATE ) {
    throw runtime_error( format("{}: CREATE is not allowed as an option in setWriteModeForExisting()", CompName) );
  }
  else if ( mode == pool::DatabaseConnectionPolicy::READ ) {
    throw runtime_error( format("{}: READ is not allowed as an option in setWriteModeForExisting()", CompName) );
  }
  else {
    if ( mode == pool::DatabaseConnectionPolicy::RAISE_ERROR ||
         mode == pool::DatabaseConnectionPolicy::UPDATE ||
         mode == pool::DatabaseConnectionPolicy::OVERWRITE ) {
      m_writeModeForExisting = mode;
      return true;
    }
    else {
      throw runtime_error( format("{}: Unrecognizable option specified in setWriteModeForExisting()", CompName) );
    }
  }
  return false;
}

bool
pool::DatabaseConnectionPolicy::setWriteModeForNonExisting( pool::DatabaseConnectionPolicy::Mode mode )
{
  if ( mode ==  pool::DatabaseConnectionPolicy::READ ) {
    throw runtime_error( format("{}: READ is not allowed as an option in setWriteModeForNonExisting()", CompName) );
  }
  else if ( mode == pool::DatabaseConnectionPolicy::OVERWRITE ) {
    throw runtime_error( format("{}: OVERWRITE is not allowed as an option in setWriteModeForExisting()", CompName) );
  }
  else {
    if ( mode == pool::DatabaseConnectionPolicy::RAISE_ERROR ||
         mode == pool::DatabaseConnectionPolicy::CREATE ||
         mode == pool::DatabaseConnectionPolicy::UPDATE ) {
      m_writeModeForNonExisting = mode;
      return true;
    }
    else {
      throw runtime_error( format("{}: Unrecognizable option specified in setWriteModeForNonExisting()", CompName) );
    }
  }
  return false;
}

bool
pool::DatabaseConnectionPolicy::setReadMode( pool::DatabaseConnectionPolicy::Mode mode )
{
  if ( mode == pool::DatabaseConnectionPolicy::CREATE ) {
    throw runtime_error( format("{}: CREATE is not allowed as an option in setReadMode()", CompName) );
  }
  else if ( mode == pool::DatabaseConnectionPolicy::OVERWRITE ) {
    throw runtime_error( format("{}: OVERWRITE is not allowed as an option in setReadMode()", CompName) );
  }
  else if ( mode == pool::DatabaseConnectionPolicy::RAISE_ERROR ) {
    throw runtime_error( format("{}: RAISE_ERROR is not allowed as an option in setReadMode()", CompName) );
  }
  else {
    if ( mode == pool::DatabaseConnectionPolicy::READ ||
         mode == pool::DatabaseConnectionPolicy::UPDATE ) {
      m_readMode = mode;
      return true;
    }
    else {
      throw runtime_error( format("{}: Unrecognizable option specified in setReadMode()", CompName) );
    }
  }
  return false;
}
