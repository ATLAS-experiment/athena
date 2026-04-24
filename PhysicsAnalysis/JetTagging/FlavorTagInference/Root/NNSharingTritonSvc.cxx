/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/NNSharingTritonSvc.h"
#include "FlavorTagInference/SaltModelTriton.h"
#include "PathResolver/PathResolver.h"

namespace FlavorTagInference {

  std::shared_ptr<const GNN> NNSharingTritonSvc::get(
    const std::string& nn_name,
    const GNNOptions& opts
  ) {
    NNHashing::NNKey key{nn_name, opts};
    if (m_gnns.count(key)) {
      ATH_MSG_INFO("getting " << nn_name << " from cached NNs");
      return m_gnns.at(key);
    } else if (m_base_gnns.count(nn_name) ) {
      ATH_MSG_INFO("adapting " << nn_name << " from cached NNs, new opts");
      auto nn = std::make_shared<const GNN>(*m_base_gnns.at(nn_name), opts);
      m_gnns[key] = nn;
      return nn;
    }
    std::shared_ptr<const GNN> nn;
    if(auto it = m_tritonPathToName.find(nn_name); it!=m_tritonPathToName.end()) {
      ATH_MSG_INFO("building " << nn_name << " from onnx file to run with Triton");
      //using namespace FlavorTagInference;
      std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_name);
      auto saltSharedTriton = std::make_shared<const SaltModelTriton>(fullPathToOnnxFile
								    , it->second
								    , m_tritonTimeout
								    , m_tritonPort
								    , m_tritonUrl
								    , m_tritonUseSsl
								    , m_tritonBearer);
      ISaltModelPtr saltShared = saltSharedTriton;
      nn = std::make_shared<const GNN>(saltShared, opts);
    }
    m_base_gnns[nn_name] = nn;
    m_gnns[key] = nn;
    return nn;
  }
  StatusCode NNSharingTritonSvc::initialize() {
    return StatusCode::SUCCESS;
  }
}
