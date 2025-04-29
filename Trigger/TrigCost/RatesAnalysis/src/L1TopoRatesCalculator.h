/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_L1TOPORATESCALCULATOR_H
#define RATESANALYSIS_L1TOPORATESCALCULATOR_H 1

#include "RatesAnalysis/RatesAnalysisAlg.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

/* L1TopoSim results access
 */
#include "xAODTrigger/TrigDecision.h"
#include "xAODTrigger/L1TopoSimResultsContainer.h"
#include "xAODTrigger/L1TopoSimResults.h"

//for hw data access 
#include "xAODTrigL1Calo/L1TopoRawDataContainer.h"

/* Including L1Menu connectors
 */
#include "TrigConfData/L1Connector.h"
#include "TrigConfData/L1Menu.h"
#include "AthenaBaseComps/AthCommonDataStore.h"
#include <vector>
#include <cstdint>
#include <unordered_map>

class L1TopoRatesCalculator: public ::RatesAnalysisAlg { 
 public:
  L1TopoRatesCalculator( const std::string& name, ISvcLocator* pSvcLocator );
  
  SG::ReadHandleKey<xAOD::TrigDecision> m_trigDecisionKey{this, "TrigDecisionKey", "xTrigDecision"};
  SG::ReadHandleKey<xAOD::L1TopoSimResultsContainer> m_l1topoKey {this, "L1_TopoKey", "L1_TopoSimResults", "l1topo EDM"};
  
  virtual StatusCode initialize() override;
  virtual StatusCode  ratesInitialize() override;
  virtual StatusCode  ratesExecute() override;
  virtual StatusCode  ratesFinalize() override;
  
 private:  
  TH2D* m_ratesMatrixHist{}; // Rates Matrix
  struct ResultDefinition {
      unsigned int conID{};
      unsigned int flatindex{};
      unsigned int nBit{};
      unsigned int clock{};
      bool fromSim{};
      bool overflow{};
  };
  
  Gaudi::Property<std::vector<std::string>> m_L1_items_json{this, "m_L1_items_json", {}, "L1 trigger items"};
  Gaudi::Property<std::vector<std::string>> m_userDefinedNames{this, "m_userDefinedNames", {}, "user Defined Names"};
  Gaudi::Property<std::vector<std::string>> m_userDefinedDefinitions{this, "m_userDefinedDefinitions", {}, "user Defined definitions"};
  std::unordered_map<std::string, std::string> m_userDefinedMap; 
  
  std::vector<std::string> m_L1_items;
  std::vector<std::string> m_L1_item_definitions;
  std::vector<std::string> m_beforeCTP_triggers;
  std::vector<std::string> m_beforeCTP_triggers_mult;
  std::vector<ResultDefinition> m_definitions;
  std::vector<std::vector<double>> m_rates_matrix;
  std::vector<std::vector<double>> m_rates_matrix2;
  std::vector<double> m_denominator;
  double m_weighted_sum{};
  struct TriggerInfo {
  	std::vector<std::string> triggers;
    	std::vector<std::string> operations;
  };
 
  std::map<std::string, TriggerInfo> m_triggerMap; 
  uint32_t extractResult(const std::vector<uint32_t>& connectorContents, const L1TopoRatesCalculator::ResultDefinition& definition, unsigned int startOffset);
  uint32_t getHWvalue(const L1TopoRatesCalculator::ResultDefinition& definition); 
  bool getTOBblockFromProcessor(const std::vector<uint32_t>& allDataBlock, std::vector<uint32_t>& tobBlock, uint32_t fpgaPattern);
  uint32_t L1TopoSimResultsContainer_decoder(const L1TopoRatesCalculator::ResultDefinition& definition, SG::ReadHandle<xAOD::L1TopoSimResultsContainer>& cont);
  Gaudi::Property<float> m_lumi{this, "TargetLuminosity", 2e34, "Targer inst. luminosity, assuming full ring."};
}; 

#endif //> !RATESANALYSIS_RATESEMULATIONEXAMPLE_H
