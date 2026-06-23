/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__ROOT_UTILS_H
#define ROOT_CORE_UTILS__ROOT_UTILS_H

/// This module defines simple utility functions for root.



#include <RootCoreUtils/Global.h>

class TDirectory;
class TObject;

namespace RCU
{
  /// effects: set the directory this object is associated with
  /// returns: whether the object type actively knows its directory,
  ///   otherwise this is a no-op
  /// guarantee: strong
  /// failures: directory add errors
  /// requires: object != 0
  /// rationale: this is mainly meant to allow calling SetDirectory(0)
  ///   on arbitrary objects, but it also tries to do the right thing
  ///   when adding objects to a directory.  For the most part it is a
  ///   workaround for TH1 objects keeping track of which directory
  ///   they belong to.
  bool SetDirectory (TObject *object, TDirectory *directory);
}

#endif
