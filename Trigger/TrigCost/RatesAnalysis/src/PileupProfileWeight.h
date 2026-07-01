/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_PILEUPPROFILEWEIGHT_H
#define RATESANALYSIS_PILEUPPROFILEWEIGHT_H 1

#include "RatesAnalysis/IAdditionalWeight.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/PropertyHolder.h"

#include <nlohmann/json.hpp>

/**
 * This class provides the weight used to reweight pileup (mu) distribution by querying JSON.
 */

class PileupProfileWeight : public extends<AthAlgTool, IAdditionalWeight> {
 public:
  using extends::extends;
  
  virtual StatusCode initialize() override;
  virtual StatusCode getValue(double& value) const override;

 private: 
  Gaudi::Property<std::string> m_weightsFile {this, "WeightsFile", "", "Path to the weights file"};
  nlohmann::json m_weightsMap;
};

#endif
