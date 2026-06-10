/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__EXCEPTION_MSG_H
#define ROOT_CORE_UTILS__EXCEPTION_MSG_H

// This module defines an exception that contains nothing more than an
// error message.



//protect
#include <RootCoreUtils/Global.h>

#include <exception>
#include <string>

namespace RCU
{
  class ExceptionMsg : public std::exception
  {
    //
    // public interface
    //

    // effects: test the invariant of this object
    // guarantee: no-fail
  public:
    void testInvariant () const;


    // effects: create an exception with the given message
    // guarantee: strong
    // failures: out of memory II
    // requires: val_file != 0
    // requires: val_line != 0
    // requires: !val_message.empty()
  public:
    ExceptionMsg (const char *const val_file, const unsigned val_line,
		  const std::string& val_message);


    // effects: destroy this object
    // guarantee: no-fail
  public:
    virtual ~ExceptionMsg () throw ();



    //
    // interface inherited from std::exception
    //

    // returns: the message associated with this exception
    // guarantee: no-fail
  public:
    virtual const char *what () const throw ();



    //
    // private interface
    //

    /// description: the actual message
  private:
    std::string m_message;
  };
}

#endif
