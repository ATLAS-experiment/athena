/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__MESSAGE_H
#define ROOT_CORE_UTILS__MESSAGE_H

#include <RootCoreUtils/Global.h>

#include <RootCoreUtils/MessageType.h>

namespace RCU
{
  struct Message
  {
    //
    // public interface
    //

    /// description: the location where the message was send
  public:
    const char *package;
  public:
    const char *file;
  public:
    unsigned line;

    /// description: the type of this message
  public:
    MessageType type;

    /// description: the actual message we are sending or 0 if there
    ///   isn't one
  public:
    const char *message;


    /// effects: standard constructor
    /// guarantee: no-fail
  public:
    Message ();


    /// effects: print the given message
    /// guarantee: basic
    /// failures: i/o errors
  public:
    void send () const;
  };
}

#endif
