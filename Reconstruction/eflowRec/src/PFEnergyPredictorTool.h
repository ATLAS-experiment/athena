/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFENERFYPREDICTORTOOL_H
#define EFLOWREC_PFENERFYPREDICTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include <fstream>      // std::fstream

static const InterfaceID IID_PFEnergyPredictorTool("PFEnergyPredictorTool", 1, 0);
class eflowRecTrack;

class PFEnergyPredictorTool :  public AthAlgTool
{
public:
  PFEnergyPredictorTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize() override;
  virtual StatusCode finalize()   override;

  float runOnnxInference(std::vector<float> &tensor) const;
  static const InterfaceID& interfaceID();

  float nnEnergyPrediction(const eflowRecTrack *ptr) const;
  void NormalizeTensor(std::vector<float> &tensor, size_t limit) const;

private:
  //mark as thread safe because we need to call the run function of Session, which is not const
  //the onnx documentation states that this is thread safe
  std::unique_ptr<Ort::Session> m_session ATLAS_THREAD_SAFE;

  std::vector<const char *> m_input_node_names;

  std::vector<const char *> m_output_node_names;

  std::vector<int64_t> m_input_node_dims;
  ServiceHandle<AthOnnx::IOnnxRuntimeSvc> m_svc{this, "ONNXRuntimeSvc", "AthOnnx::OnnxRuntimeSvc", "CaloMuonScoreTool ONNXRuntimeSvc"};
  Gaudi::Property<std::string> m_model_filepath{this, "ModelPath", "////"};

  /** Normalization constants for the inputs to the onnx model */
  Gaudi::Property<float> m_cellE_mean{this,"cellE_mean",-2.2852574689444385};
  Gaudi::Property<float> m_cellE_std{this,"cellE_std",2.0100506557174946};
  Gaudi::Property<float> m_cellPhi_std{this,"cellPhi_std",0.6916977411859621};
  
};

inline const InterfaceID& PFEnergyPredictorTool::interfaceID() { return IID_PFEnergyPredictorTool; }


#endif

