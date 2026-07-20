/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_TOOLS_SPLIT_H
#define SAMPLE_HANDLER_TOOLS_SPLIT_H

/// This module defines utility functions used for splitting samples.



//protect
#include <SampleHandler/Global.h>

#include <Rtypes.h>

namespace SH
{
  /// effects: scan each sample in the sample handler and store the
  ///   number of entries per file in the meta-data
  /// guarantee: basic, may only scan some
  /// failures: out of memory
  /// failures: read errors
  /// failures: invalid sample type
  void scanNEvents (SampleHandler& sh);


  /// effects: scan the given sample and store the number of entries
  ///   per file in the meta-data
  /// guarantee: strong
  /// failures: out of memory
  /// failures: read errors
  /// failures: invalid sample type
  void scanNEvents (Sample& sample);


  /// effects: split the given sample into a set of samples, with each
  ///   sample containing either exactly one file or at most nevt
  ///   events
  /// side effects: if scanNEvents hasn't been run on this sample, run
  ///   it.
  /// guarantee: strong
  /// failures: out of memory
  /// failures: scanning errors
  SampleHandler splitSample (Sample& sample, Long64_t nevt);
}

#endif
