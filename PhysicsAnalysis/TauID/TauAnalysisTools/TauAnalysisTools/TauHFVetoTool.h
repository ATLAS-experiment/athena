/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAUHFVETOTOOL_H
#define TAUANALYSISTOOLS_TAUHFVETOTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"

// local include(s)
#include "TauAnalysisTools/ITauHFVetoTool.h"

#include <string>
#include <vector>

namespace TauAnalysisTools
{

class TauHFVetoTool : public asg::AsgTool, public virtual ITauHFVetoTool {

  ASG_TOOL_CLASS2( TauHFVetoTool, asg::IAsgTool, TauAnalysisTools::ITauHFVetoTool )

 public:
  TauHFVetoTool(const std::string& name);

  virtual ~TauHFVetoTool() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode applyHFvetoBDTs(const xAOD::TauJetContainer* Taus, const xAOD::JetContainer* PFlowJets) const override;
  virtual const xAOD::Jet* findClosestPFlowJet(const xAOD::TauJet* xTau, const xAOD::JetContainer* vPFlowJets) const override;
  virtual std::vector<float> assembleInputValues(const xAOD::TauJet* xTau, const xAOD::Jet* xAuxJet) const override;

 private:

  virtual StatusCode inference(const ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> m_onnxTool, const std::vector<float>& inputValues, float& output) const;
  virtual StatusCode bVetoScore(const int& prongness, const std::vector<float>& input, float& output) const;
  virtual StatusCode cVetoScore(const int& prongness, const std::vector<float>& input, float& output) const;
  ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool >  m_onnxTool_bveto1p{this, "bveto1p", "AthOnnx::OnnxRuntimeInferenceTool"};
  ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool >  m_onnxTool_bveto3p{this, "bveto3p", "AthOnnx::OnnxRuntimeInferenceTool"};
  ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool >  m_onnxTool_cveto1p{this, "cveto1p", "AthOnnx::OnnxRuntimeInferenceTool"};
  ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool >  m_onnxTool_cveto3p{this, "cveto3p", "AthOnnx::OnnxRuntimeInferenceTool"};

};

}

#endif