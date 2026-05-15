/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_IADDITIONALWEIGHT_H
#define RATESANALYSIS_IADDITIONALWEIGHT_H 1

#include "GaudiKernel/IAlgTool.h"

/**
 * Interface for an additional weight to be multiplied to the event weight
 * e.g. for histogramming
 */

class IAdditionalWeight : virtual public IAlgTool {
 public:
  DeclareInterfaceID( IAdditionalWeight, 1, 0 );

  virtual StatusCode getValue(double& value) const = 0;

};

#endif
