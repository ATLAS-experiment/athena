/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

// Framework include files
#include "StorageSvc/DbStatus.h"

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
  typedef int  DbAccessMode;
  typedef std::pair< long long, long long > DbLink;

  static const long long int INVALID = ~0x0LL;

  /// Definition of access modes
  enum AccessMode {
    NONE        =  0,
    NOT_OPEN    =  1<<0,
    READ        =  1<<1, 
    UPDATE      =  1<<2, 
    CREATE      =  1<<3, 
    RECREATE    = (1<<4)+(1<<3),
    WRITE       =  1<<3,
    DESTROY     =  1<<5
  };

  static const DbStatus Success      (DbStatus::Success);
  static const DbStatus Warning      (DbStatus::Warning);
  static const DbStatus Error        (DbStatus::Error);
  static const DbStatus ConnTimeout  (static_cast<unsigned int>(DbStatus::Error)+2);

  /// Issue a debug break
  void      debugBreak();

  /// Issue a debug break with error message (to std::cout !!)
  void      debugBreak( const std::string& src, 
                        const std::string& msg,
                        bool rethrow=true);

  /// Debug break with printout and exception chaining
  void      debugBreak( const std::string& src, 
                        const std::string& msg,
                        const std::exception& e,
                        bool rethrow=true);

  /// Check for tracing
  bool      doTrace();

  /// Translate access mode to string
  const char* accessMode(pool::DbAccessMode access_mode);

  /// Delete a pointer
  template<class T> inline DbStatus deletePtr(T*& p)  {
    if ( 0 != p )    {
      delete p;
      p = 0;
    }
    return Success;
  }
  /// Release a pointer
  template<class T> inline DbStatus releasePtr(T*& p)  {
    if ( 0 != p )    {
      p->release();
      p = 0;
    }
    return Success;
  }

  /// Release Reference countable pointer
  template<class T> inline int decrementPtr(T*& p)  {
    if ( p )    {
      int cnt = p->release();
      if ( 0 >= cnt )  {
        p = 0;
      }
      return cnt;
    }
    return ~0x0;
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
    
}
#endif  // POOL_POOL_H
