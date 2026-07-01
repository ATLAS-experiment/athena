/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef LUNDNETTAGGER_H
#define LUNDNETTAGGER_H

#include "BoostedJetTaggers/JSSTaggerBase.h"

// xAOD
#include "xAODJet/Jet.h"

// ONNX Runtime
#include <onnxruntime_cxx_api.h>

// STL
#include <string>
#include <vector>
#include <memory>

class LundNetTagger :
  public JSSTaggerBase {
    ASG_TOOL_CLASS0(LundNetTagger)

    public:

      /// Constructor
      LundNetTagger(const std::string& name);

      /// Run once at the start of the job to setup everything
      virtual StatusCode initialize() override;

      /// Decorate single jet with tagging info
      virtual StatusCode tag(const xAOD::Jet& jet) const override;
      virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

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
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_classNDecorKey{
    this, "ClassNDecor", "", "LundNet output nodeN inference"
  };

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreOneDecorKey{
    this, "ScoreOneDecorKey", "", "Score one decoration key"
  };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreTwoDecorKey{
    this, "ScoreTwoDecorKey", "", "Score two decoration key"
  };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreThreeDecorKey{
    this, "ScoreThreeDecorKey", "", "Score three decoration key"
  };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreFourDecorKey{
    this, "ScoreFourDecorKey", "", "Score four decoration key"
  };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_scoreFiveDecorKey{
    this, "ScoreFiveDecorKey", "", "Score five decoration key"
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

  unsigned int m_classN = 0;
  // Check the classN from the output shape in initialize()

  /// Helper to build ONNX inputs (batch_size=1)
  bool buildOnnxInputs(const xAOD::Jet& jet,
                       std::vector<float>& out_x_float,            // flattened [num_nodes * feat_dim]
                       std::vector<int64_t>& out_edge_index_int64, // flattened [2 * num_edges]
                       std::vector<int64_t>& out_batch_int64,      // [num_nodes]
                       std::vector<int64_t>& out_counts_int64,     // [1]
                       std::vector<float>& out_Ntrk_float) const;  // [1]
};

#endif // LUNDNETTAGGER_H
