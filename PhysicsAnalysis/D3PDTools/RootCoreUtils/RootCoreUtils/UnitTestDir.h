/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS_UNIT_TEST_DIR_HH
#define ROOT_CORE_UTILS_UNIT_TEST_DIR_HH

/// This module defines a class that manages a temporary directory for
/// unit tests.



#include <string>

namespace RCU
{
  class UnitTestDir
  {
    //
    // public interface
    //

    /// effects: standard constructor
    /// guarantee: strong
    /// failures: out of memory
    /// failures: directory creation errors
  public:
    UnitTestDir (const std::string& package, const std::string& name);


    /// rationale: deleted to avoid copying
  public:
    UnitTestDir (const UnitTestDir&) = delete;
    UnitTestDir& operator = (const UnitTestDir&) = delete;


    /// effects: standard destructor
    /// guarantee: no-fail
  public:
    ~UnitTestDir ();


    /// description: the path to the directory
    /// guarantee: no-fail
  public:
    const std::string& path () const;


    /// description: whether we clean up on completion
    /// guarantee: no-fail
  public:
    bool cleanup () const;
    void cleanup (bool val_cleanup);



    //
    // private interface
    //

    /// description: members directly corresponding to accessors
  private:
    std::string m_path;
  private:
    bool m_cleanup;
  };
}

#endif
