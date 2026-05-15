/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_IEMULATEDTRIGGER_H
#define RATESANALYSIS_IEMULATEDTRIGGER_H 1

#include "GaudiKernel/IAlgTool.h"

/**
 * Interface for an emulated trigger. 
 * The trigger can be added to a ntupler to store the threshold as a tree branch.
 */

class IEmulatedTrigger : virtual public IAlgTool {
 public:
  DeclareInterfaceID( IEmulatedTrigger, 1, 0 );

  virtual std::string branchName() const = 0;
  virtual double thresholdValue() const = 0;
  virtual StatusCode updateThresholdValue() = 0;

};

#endif
