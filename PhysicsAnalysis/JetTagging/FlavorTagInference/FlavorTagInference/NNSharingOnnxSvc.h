/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NN_SHARING_ONNX_SVC_H
#define NN_SHARING_ONNX_SVC_H

#include "FlavorTagInference/INNSharingSvc.h"
#include "AsgServices/AsgService.h"

namespace FlavorTagInference
{
  class NNSharingOnnxSvc: public extends<asg::AsgService, INNSharingSvc>
  {
  public:
    using extends::extends;  // base class constructor
    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) override;
  private:
    using val_t = std::shared_ptr<const GNN>;
    std::unordered_map<NNHashing::NNKey, val_t, NNHashing::NNHasher> m_gnns;
    std::unordered_map<std::string, val_t> m_base_gnns;
  };

}

#endif
