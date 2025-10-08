/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LAR_RDO_ANALYSIS_H
#define LAR_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "LArRawEvent/LArTTL1Container.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"


class LArRDOAnalysis : public AthHistogramAlgorithm {

public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~LArRDOAnalysis() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;
 
private:

  SG::ReadHandleKey<LArRawChannelContainer> m_inputRawChannelKey{this, "InputRawChannelKey", "LArRawChannels"};
  SG::ReadHandleKey<LArTTL1Container> m_inputTTL1HADKey{this, "InputTTL1HADKey", "LArTTL1HAD"};
  SG::ReadHandleKey<LArTTL1Container> m_inputTTL1EMKey{this, "InputTTL1EMKey", "LArTTL1EM"};
  SG::ReadHandleKey<LArDigitContainer> m_inputDigitKey{this, "InputDigitKey", "LArDigitContainer_MC_Thinned"};
  BooleanProperty m_presampling{this, "PreSampling", false};

  Gaudi::Property<bool> m_doNtuple{this , "doNtuple" , true};
  Gaudi::Property<std::string> m_ntupleFileName{this , "NtupleFileName" , "/ntuples/file1"};
  Gaudi::Property<std::string> m_ntupleDirName{this , "NtupleDirectoryName" , "/LArRDOAnalysis/"};
  Gaudi::Property<std::string> m_ntupleTreeName{this , "NtupleTreeName" , "LArRDOAna"};
  Gaudi::Property<std::string> m_path{this , "HistPath" , "/LArRDOAnalysis/"};


  // LAR RAW CHANNELS
  std::vector<unsigned long long>* m_larID{nullptr};
  std::vector<int>* m_energy{nullptr};
  std::vector<int>* m_time{nullptr};
  std::vector<uint16_t>* m_qual{nullptr};
  std::vector<uint16_t>* m_prov{nullptr};
  std::vector<int>* m_gain{nullptr};
  // LAR TTL1
  std::vector<unsigned long long>* m_hadOnID{nullptr};
  std::vector<unsigned long long>* m_hadOffID{nullptr};
  std::vector<float>* m_hadSamples{nullptr};
  std::vector<unsigned long long>* m_emOnID{nullptr};
  std::vector<unsigned long long>* m_emOffID{nullptr};
  std::vector<float>* m_emSamples{nullptr};
  // LAR DIGITS
  std::vector<unsigned long long>* m_digiID{nullptr};
  std::vector<int>* m_digiGain{nullptr};
  std::vector<short>* m_digiSamples{nullptr};

  // HISTOGRAMS
  TH1* m_h_larID{nullptr};
  TH1* m_h_energy{nullptr};
  TH1* m_h_time{nullptr};
  TH1* m_h_qual{nullptr};
  TH1* m_h_prov{nullptr};
  TH1* m_h_gain{nullptr};
  TH1* m_h_hadOnID{nullptr};
  TH1* m_h_hadOffID{nullptr};
  TH1* m_h_hadSamples{nullptr};
  TH1* m_h_emOnID{nullptr};
  TH1* m_h_emOffID{nullptr};
  TH1* m_h_emSamples{nullptr};
  TH1* m_h_digiID{nullptr};
  TH1* m_h_digiGain{nullptr};
  TH1* m_h_digiSamples{nullptr};

  TTree *m_tree{nullptr};

};

#endif // LAR_RDO_ANALYSIS_H
