/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAOD_ANALYSIS
#ifndef LUNDJETONNXALG_H
#define LUNDJETONNXALG_H

// Framework
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// xAOD
#include "xAODJet/Jet.h"

// ONNX Runtime
#include <onnxruntime_cxx_api.h>

#include "StoreGate/WriteDecorHandleKey.h"

// STL
#include <string>
#include <vector>
#include <memory>

class LundJetOnnxAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  // Job properties
  Gaudi::Property<std::string> m_inputJetContainer{
    this, "InputJetContainer", "AntiKt10UFO",
    "Name of input jet container"
  };
  Gaudi::Property<std::string> m_prefix{
    this, "Prefix", "",
    "Prefix used by LundVariablesTool decorations"
  };
  Gaudi::Property<float> m_kTSelection{
    this, "kTSelection", -1000.f,
    "kT cut to apply (same meaning as kT_Cut in python loader)"
  };

  Gaudi::Property<std::string> m_scoreName{
    this, "ScoreDecoration", "LundNetScore",
    "Name of the decoration (without container prefix). Final decoration will be <container>.<prefix><ScoreDecoration>"
  };
  
  Gaudi::Property<int> m_expectedBatchSize{
    this,
    "ExpectedBatchSize",
    1,
    "Batch size expected by the ONNX model (from training/export)"
  };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_validDecorKey{
    this, "ValidDecor", "", "Valid LundNet inference"
  };
  //SG::WriteDecorHandleKey<float> m_scoreDecorKey{ this, "ScoreDecorKey", "", "WriteDecorHandleKey for the LundNet score" };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreDecorKey{
    this, "ScoreDecorKey", "", "Score decoration key"
  };
  // Means and stds used for normalization (defaults from your python snippet)
  Gaudi::Property<float> m_mean_z{ this, "MeanZ", 2.0568479032747313f, "mean z (for normalization)" };
  Gaudi::Property<float> m_std_z{ this, "StdZ", 1.4450598054504056f, "std z (for normalization)" };
  Gaudi::Property<float> m_mean_dr{ this, "MeanDR", 3.8597358364389427f, "mean dr (for normalization)" };
  Gaudi::Property<float> m_std_dr{ this, "StdDR", 2.2748462855901073f, "std dr (for normalization)" };
  Gaudi::Property<float> m_mean_kt{ this, "MeanKT", -2.379904791478249f, "mean kt (for normalization)" };
  Gaudi::Property<float> m_std_kt{ this, "StdKT", 2.940813577366582f, "std kt (for normalization)" };
  Gaudi::Property<float> m_mean_ntrk{ this, "MeanNtrk", 57.588158609500134f, "mean Ntrk" };
  Gaudi::Property<float> m_std_ntrk{ this, "StdNtrk", 23.900100132781983f, "std Ntrk" };
  std::string m_resolvedModelPath;
  Gaudi::Property<std::string> m_modelPath{
      this,
      "ModelPath",
      "",
      "Path to the ONNX model"
    };
  std::unique_ptr<Ort::Env>     m_env;
  std::unique_ptr<Ort::Session> m_session;
  // The IAthInferenceTool that wraps ONNX runtime (provided via CA)

  /// Helper to build ONNX inputs (batch_size=1)
  bool buildOnnxInputs(const xAOD::Jet& jet,
                       std::vector<float>& out_x_float,            // flattened [num_nodes * feat_dim]
                       std::vector<int64_t>& out_edge_index_int64, // flattened [2 * num_edges]
                       std::vector<int64_t>& out_batch_int64,      // [num_nodes]
                       std::vector<int64_t>& out_counts_int64,     // [1]
                       std::vector<float>& out_Ntrk_float) const;  // [1]
};

#endif // LUNDJETONNXALG_H
#endif // ATHENA-ONLY
