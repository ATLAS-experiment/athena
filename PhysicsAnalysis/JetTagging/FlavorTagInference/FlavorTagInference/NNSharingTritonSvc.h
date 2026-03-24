/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NN_SHARING_TRITON_SVC_H
#define NN_SHARING_TRITON_SVC_H

#include "FlavorTagInference/INNSharingSvc.h"
#include "AsgServices/AsgService.h"


namespace FlavorTagInference
{
  class NNSharingTritonSvc: public extends<asg::AsgService, INNSharingSvc>
  {
  public:
    using extends::extends;  // base class constructor
    virtual StatusCode initialize() override;
    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) override;
  private:
    using val_t = std::shared_ptr<const GNN>;
    std::unordered_map<NNHashing::NNKey, val_t, NNHashing::NNHasher> m_gnns;
    std::unordered_map<std::string, val_t> m_base_gnns;
    Gaudi::Property<float> m_tritonTimeout {this, "TritonTimeout", 0.f
      , "Timeout value for Triton client"};
    Gaudi::Property<int> m_tritonPort {this, "TritonPort", 443
      , "Triton server port"};
    Gaudi::Property<std::string> m_tritonUrl {this, "TritonUrl", ""
      , "Triton server URL"};
    Gaudi::Property<bool> m_tritonUseSsl {this, "TritonUseSSL", true
      , "Connect to the Triton server over SSL"};
    Gaudi::Property<std::map<std::string, std::string>> m_tritonPathToName {this, "TritonPathsMap", {}
      , "Mapping of ONNX file paths to Triton model names"};
  
  };

}

#endif
