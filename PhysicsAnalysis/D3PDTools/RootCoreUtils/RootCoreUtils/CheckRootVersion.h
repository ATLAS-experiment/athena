/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ROOT_CORE_UTILS__CHECK_ROOT_VERSION_H
#define ROOT_CORE_UTILS__CHECK_ROOT_VERSION_H

#include <RootCoreUtils/Global.h>

namespace RCU
{
  /// effects: check whether we are using a consistent root version
  /// guarantee: strong
  /// failures: version missmatch
  void check_root_version ();

  /// effects: disable the root version check
  /// guarantee: no-fail
  void disable_root_version_check ();
}

#endif
