/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_JZCOMBINEWEIGHT_H
#define RATESANALYSIS_JZCOMBINEWEIGHT_H 1

#include "RatesAnalysis/IAdditionalWeight.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/PropertyHolder.h"

#include <nlohmann/json.hpp>

/**
 * This class provides the weight used to combine JZ sliced samples by querying JSON.
 */

class JZCombineWeight : public extends<AthAlgTool, IAdditionalWeight> {
 public:
  using extends::extends;
  
  virtual StatusCode initialize() override;
  virtual StatusCode getValue(double& value) const override;

 private:
  std::size_t getIndex(double value) const;

 private: 
  Gaudi::Property<std::string> m_jetCollectionHS {
    this, "JetCollectionHS", "AntiKt4TruthJets", "Name of the hard-scatter jet collection"};
  Gaudi::Property<std::string> m_jetCollectionPU {
    this, "JetCollectionPU", "InTimeAntiKt4TruthJets", "Name of the pile-up jet collection"};
  Gaudi::Property<std::string> m_weightsFile { 
    this, "WeightsFile", "", "Path to the weights file"};
  Gaudi::Property<std::vector<double>> m_binning {
    this, "Binning", {}, "Pt binning used to define categories"};
  Gaudi::Property<std::vector<std::string>> m_weightsName {
    this, "WeightsName", {}, "Weights to be calculated - their product is used"};

  nlohmann::json m_weightsMap;
};

#endif
