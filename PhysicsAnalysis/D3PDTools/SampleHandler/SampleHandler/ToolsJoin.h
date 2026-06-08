/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_TOOLS_JOIN_H
#define SAMPLE_HANDLER_TOOLS_JOIN_H

/// This module defines utility functions used for joining samples.



//protect
#include <SampleHandler/Global.h>

#include <string>

namespace SH
{
  /// effects: remove all samples matching the name pattern, and join
  ///   them into a single sample named sampleName
  /// guarantee: strong
  /// failures: out of memory II
  /// failures: i/o errors
  void mergeSamples (SampleHandler& sh, const std::string& sampleName,
		     const std::string& pattern);
}

#endif
