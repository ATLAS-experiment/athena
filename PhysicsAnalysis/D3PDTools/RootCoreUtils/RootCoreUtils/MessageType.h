/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__MESSAGE_TYPE_H
#define ROOT_CORE_UTILS__MESSAGE_TYPE_H

/// This module defines macros for reporting errors.



#include <RootCoreUtils/Global.h>

namespace RCU
{
  enum MessageType
  {
    /// description: print a regular message
    MESSAGE_REGULAR,

    /// description: print a warning
    MESSAGE_WARNING,

    /// description: print an error
    MESSAGE_ERROR,

    /// description: send out an exception
    MESSAGE_EXCEPTION,

    /// description: print and abort
    MESSAGE_ABORT,

    /// description: unspecified message type
    MESSAGE_UNSPECIFIED
  };
}

#endif
