/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ROOT_CORE_UTILS__HADD_H
#define ROOT_CORE_UTILS__HADD_H

#include <string>
#include <vector>

namespace RCU
{
  /// effects: perform the hadd functionality
  /// guarantee: basic
  /// failures: out of memory III
  /// failures: i/o errors
  void hadd (const std::string& output_file,
	     const std::vector<std::string>& input_files,
	     unsigned max_files = 0);
}

#endif
