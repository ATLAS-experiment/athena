/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RATESANALYSIS_JZCOMBINEWEIGHTNTUPLER_H
#define RATESANALYSIS_JZCOMBINEWEIGHTNTUPLER_H 1

#include "RatesAnalysis/RatesAnalysisAlg.h"
#include "RatesAnalysis/IEmulatedTrigger.h"
#include "TTree.h"

#include <unordered_map>

/**
 * This class is used to produce the ntuple for deriving the weights
 * to combine JZ-sliced multi-jet MC samples using the BLUE method
 */

class JZCombineWeightNtupler : public RatesAnalysisAlg { 
 public: 
  JZCombineWeightNtupler(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~JZCombineWeightNtupler(); 

  virtual StatusCode ratesInitialize() override;
  virtual StatusCode ratesExecute() override;
  virtual StatusCode ratesFinalize() override;
  
 private:
  StatusCode addEmulatedThresholds();
  StatusCode setEmulatedThresholds();
  StatusCode resetValues();

 private:
  Gaudi::Property<std::string> m_jetCollectionHS {
    this, "JetCollectionHS", "AntiKt4TruthJets", "Name of the hard-scatter jet collection"};
  Gaudi::Property<std::string> m_jetCollectionPU {
    this, "JetCollectionPU", "InTimeAntiKt4TruthJets", "Name of the pile-up jet collection"};
  Gaudi::Property<uint32_t> m_dsid_JZ0 {
    this, "JZ0ID", 801165u, "DSID of the JZ0 sample"};

  /**
   * User can extend the output tree with customised emulated triggers
   */
  ToolHandleArray<IEmulatedTrigger> m_triggers {this, "Triggers", {}, ""};
  
  TTree* m_tree {nullptr};
  uint64_t m_event_number {0};
  double m_weight_eb {1.0};
  double m_mu_actual {0.0};
  double m_pt_j0_AK4HS {5.0};
  uint32_t m_index_JZ {0};
  uint32_t m_n_pileup_records {0};
  std::vector<double> m_pt_j0_AK4PU;
  std::vector<uint32_t> m_pileup_event_number;

  std::unordered_map<std::string, double> m_thresholds;
}; 

#endif //> !RATESANALYSIS_JZCOMBINEWEIGHTNTUPLER_H
