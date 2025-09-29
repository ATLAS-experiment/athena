/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef BCM_RDO_ANALYSIS_H
#define BCM_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "InDetBCM_RawData/BCM_RDO_Container.h"
#include "InDetBCM_RawData/BCM_RDO_Collection.h"
#include "InDetSimData/InDetSimDataCollection.h"

#include <string>
#include <vector>
#include "TH1.h"


class BCM_RDOAnalysis : public AthHistogramAlgorithm {

public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~BCM_RDOAnalysis() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;

private:

  SG::ReadHandleKey<BCM_RDO_Container> m_inputKey{this, "InputKey", "BCM_RDOs"};
  SG::ReadHandleKey<InDetSimDataCollection> m_inputTruthKey{this, "InputTruthKey", "BCM_SDO_Map"};


  Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName", "/ntuples/file"};
  Gaudi::Property<std::string> m_ntupleDirName{this, "NtupleDirectoryName", "/BCM_RDOAnalysis/"};
  Gaudi::Property<std::string> m_ntupleTreeName{this, "NtupleTreeName", "BCM_RDOAna"};
  Gaudi::Property<std::string> m_path{this, "HistPath", "/BCM_RDOAnalysis/"};

  // RDO
  std::vector<int>* m_word1{nullptr};
  std::vector<int>* m_word2{nullptr};
  std::vector<int>* m_chan{nullptr};
  std::vector<int>* m_pulse1Pos{nullptr};
  std::vector<int>* m_pulse1Width{nullptr};
  std::vector<int>* m_pulse2Pos{nullptr};
  std::vector<int>* m_pulse2Width{nullptr};
  std::vector<int>* m_LVL1A{nullptr};
  std::vector<int>* m_BCID{nullptr};
  std::vector<int>* m_LVL1ID{nullptr};
  std::vector<int>* m_err{nullptr};
  // SDO
  std::vector<unsigned long long>* m_sdoID{nullptr};
  std::vector<int>* m_sdoWord{nullptr};
  std::vector<int>* m_barcode{nullptr};
  std::vector<int>* m_eventIndex{nullptr};
  std::vector<float>* m_charge{nullptr};
  std::vector< std::vector<int> >* m_barcode_vec{nullptr};
  std::vector< std::vector<int> >* m_eventIndex_vec{nullptr};
  std::vector< std::vector<float> >* m_charge_vec{nullptr};

  // HISTOGRAMS
  TH1* m_h_word1{nullptr};
  TH1* m_h_word2{nullptr};
  TH1* m_h_chan{nullptr};
  TH1* m_h_pulse1Pos{nullptr};
  TH1* m_h_pulse1Width{nullptr};
  TH1* m_h_pulse2Pos{nullptr};
  TH1* m_h_pulse2Width{nullptr};

  TH1* m_h_sdoID{nullptr};
  TH1* m_h_sdoWord{nullptr};
  TH1* m_h_barcode{nullptr};
  TH1* m_h_eventIndex{nullptr};
  TH1* m_h_charge{nullptr};

  TTree* m_tree{nullptr};
};

#endif // BCM_RDO_ANALYSIS_H
