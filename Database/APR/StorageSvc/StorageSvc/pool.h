/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//  Package    : StorageSvc (The pool framework)
//
//  Description: Management of the object Databases
//
//  @author      M.Frank
//====================================================================
#ifndef POOL_POOL_H
#define POOL_POOL_H 1

#include "GaudiKernel/IFileMgr.h"

// STL include files
#include <string>
#include <utility>

/* namespace pool
 *
 *  Description:
 *  All specific constants for a storage technology
 *
 *  @author  M.Frank
 *  @version 1.0
 */
namespace pool   {
  // Type definitions
  typedef void DbObject;
  typedef std::pair< long long, long long > DbLink;

  static const long long int INVALID = ~0x0LL;

  namespace Transaction {
    enum Action { TRANSACT_COMMIT, TRANSACT_FLUSH };
  }

  /// Translate access mode to string
  const char* accessMode(Io::IoFlag access_mode);

  /// Delete a pointer
  template<class T> inline void deletePtr(T*& p)  {
    if ( 0 != p )    {
      delete p;
      p = 0;
    }
  }
  /// Release a pointer
  template<class T> inline void releasePtr(T*& p)  {
    if ( 0 != p )    {
      p->release();
      p = 0;
    }
  }

  class RefCounter {
  private: 
    int m_count = 1;
  public:
    RefCounter() {}
    RefCounter( const RefCounter& ) { m_count = 1; }
    // cppcheck-suppress operatorEqVarError; deliberate
    RefCounter& operator= (const RefCounter&) { return *this; }
    /// Increase the reference count
    int addRef()   { return ++m_count; }
    /// Decrease the reference count 
    int subRef()   { return --m_count; }
  };


  /// Read an environment variable into string (returns empty string if not set)
  std::string getEnvStr(const std::string& key);

}
#endif  // POOL_POOL_H
