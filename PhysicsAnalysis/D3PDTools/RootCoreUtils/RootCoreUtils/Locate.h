/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__LOCATE_H
#define ROOT_CORE_UTILS__LOCATE_H

#include <RootCoreUtils/Global.h>

#include <string>

namespace RCU
{
  /// effects: find the file with the given name from a list of
  ///   locations separated by "::".  the list may contain either
  ///   files or URLs starting with "http://".  URLs will be
  ///   downloaded into the data/ directory, where they will stay
  ///   permanently.
  /// returns: the path in the local filesystem
  /// guarantee: strong
  /// failures: out of memory III
  /// failures: inconsistent file names
  /// failures: download errors
  /// rationale: depending on where you are executing your code, you
  ///   may or may not have access to /cvmfs where important data
  ///   files are kept.  this mechanism allows to pick it up from
  ///   various alternate places.
  std::string locate (const std::string& locations);
}

#endif
