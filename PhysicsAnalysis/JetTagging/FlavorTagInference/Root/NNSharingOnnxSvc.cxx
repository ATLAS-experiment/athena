/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/NNSharingOnnxSvc.h"
#include "FlavorTagInference/SaltModel.h"
#include "PathResolver/PathResolver.h"

namespace FlavorTagInference {

  std::shared_ptr<const GNN> NNSharingOnnxSvc::get(
    const std::string& nn_name,
    const GNNOptions& opts) {
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
    ATH_MSG_INFO("building " << nn_name << " from onnx file on the "
                 << m_executionProvider.value() << " execution provider");
    SaltModelOptions salt_opts {
      m_executionProvider.value(), m_deviceId.value(), m_useTF32.value()};
    ISaltModelPtr salt = std::make_shared<const SaltModel>(
      PathResolverFindCalibFile(nn_name), salt_opts);
    nn = std::make_shared<const GNN>(salt, opts);
    m_base_gnns[nn_name] = nn;
    m_gnns[key] = nn;
    return nn;
  }
}
