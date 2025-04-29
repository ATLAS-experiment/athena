/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/NNSharingSvc.h"

#include "src/hash.h"

namespace FlavorTagInference {

  namespace detail {
    std::size_t NNKey::hash() const {
      return combine(getHash(path), getHash(opts));
    }

    // allow NNKey to be used as unordered map key
    bool NNKey::operator==(const NNKey& key) const {
      return path == key.path && opts == key.opts;
    }
  }

  std::shared_ptr<const GNN> NNSharingSvc::get(
    const std::string& nn_name,
    const GNNOptions& opts) {
    detail::NNKey key{nn_name, opts};
    if (m_gnns.count(key)) {
      ATH_MSG_INFO("getting " << nn_name << " from cached NNs");
      return m_gnns.at(key);
    } else if (m_base_gnns.count(nn_name) ) {
      ATH_MSG_INFO("adapting " << nn_name << " from cached NNs, new opts");
      auto nn = std::make_shared<const GNN>(*m_base_gnns.at(nn_name), opts);
      m_gnns[key] = nn;
      return nn;
    }
    ATH_MSG_INFO("building " << nn_name << " from onnx file");
    auto nn = std::make_shared<const GNN>(nn_name, opts);
    m_base_gnns[nn_name] = nn;
    m_gnns[key] = nn;
    return nn;
  }


}
