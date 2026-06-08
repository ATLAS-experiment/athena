/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER__TOOLS_DUPLICATES_H
#define SAMPLE_HANDLER__TOOLS_DUPLICATES_H

#include <SampleHandler/Global.h>

namespace SH
{
  /// effects: check the given sample for duplicate events and then
  ///   print them out
  /// guarantee: basic, may print partially
  /// failures: out of memory III
  /// failures: i/o errors
  void printDuplicateEvents (const Sample& sample);


  /// effects: check each sample for duplicate events and then
  ///   print them out
  /// guarantee: basic, may print partially
  /// failures: out of memory III
  /// failures: i/o errors
  void printDuplicateEventsSplit (const SampleHandler& sh);


  /// effects: check for duplicate events between all the samples and
  ///   then print them out
  /// guarantee: basic, may print partially
  /// failures: out of memory III
  /// failures: i/o errors
  void printDuplicateEventsJoint (const SampleHandler& sh);
}

#endif
