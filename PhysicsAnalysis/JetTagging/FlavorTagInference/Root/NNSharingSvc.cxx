/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/NNSharingSvc.h"
#include "PathResolver/PathResolver.h"
#ifndef XAOD_ANALYSIS
#include "FlavorTagInference/SaltModelTriton.h"
#endif
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
    std::shared_ptr<const GNN> nn;
#ifndef XAOD_ANALYSIS
    auto it = m_tritonPathToName.find(nn_name);
    if(m_useTriton && it!=m_tritonPathToName.end()) {
      ATH_MSG_INFO("building " << nn_name << " from onnx file to run with Triton");
      //using namespace FlavorTagInference;
      std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_name);
      auto saltSharedTriton = std::make_shared<const SaltModelTriton>(fullPathToOnnxFile
								    , it->second
								    , m_tritonTimeout
								    , m_tritonPort
								    , m_tritonUrl
								    , m_tritonUseSsl);
      ISaltModelPtr saltShared = saltSharedTriton;
      nn = std::make_shared<const GNN>(saltShared, opts);
    }
    else {
      ATH_MSG_INFO("building " << nn_name << " from onnx file");
      nn = std::make_shared<const GNN>(nn_name, opts);
    }
#else
    ATH_MSG_INFO("building " << nn_name << " from onnx file");
    nn = std::make_shared<const GNN>(nn_name, opts);
#endif
    m_base_gnns[nn_name] = nn;
    m_gnns[key] = nn;
    return nn;
  }

#ifndef XAOD_ANALYSIS
    StatusCode NNSharingSvc::initialize() {
    if(m_useTriton) initTritonPathToName();
    return StatusCode::SUCCESS;
  }

  // This is a quick solution for the initial stage of testing.
  // In the long run we need to find some other mechanism for mapping
  // model paths to model names
  void NNSharingSvc::initTritonPathToName() {
    m_tritonPathToName = {
      {"BTagging/20250527/GN3V01/antikt4empflow/network.onnx"
       , "BTagging_network_93a858f5c730"},
      {"BTagging/20231205/GN2v01/antikt4empflow/network_fold0.onnx"
       , "BTagging_network_fold0_4812578c733e"},
      {"BTagging/20231205/GN2v01/antikt4empflow/network_fold1.onnx"
       , "BTagging_network_fold1_9280d77c131c"},
      {"BTagging/20231205/GN2v01/antikt4empflow/network_fold2.onnx"
       , "BTagging_network_fold2_25c6ad03db10"},
      {"BTagging/20231205/GN2v01/antikt4empflow/network_fold3.onnx"
       , "BTagging_network_fold3_0558b4924c49"},
      {"BTagging/20250213/GN3V00/antikt4empflow/network.onnx"
       , "BTagging_network_cce6be90efd1"},
      {"BTagging/20250213/GN3PflowMuonsV00/antikt4empflow/network.onnx"
       , "BTagging_network_d2138c4252e6"},
//      {"BTagging/20230705/gn2xv01/antikt10ufo/network.onnx" << This model is commented out because at the time of submitting
//       , "BTagging_network_9f8aadb82b76"},                  << it did not work on Triton. The code falls back to direct ONNX reading
      {"BTagging/20240925/GN2Xv02/antikt10ufo/network.onnx"
       , "BTagging_network_09c2dddf15bf"},
      {"BTagging/20250310/GN2XTauV00/antikt10ufo/network.onnx"
       , "BTagging_network_e8d5e9a3059b"},
      {"BTagging/20250912/GN3XPV01/antikt10ufo/network.onnx"
       , "BTagging_network_08105bb8c1d6"},
      {"BTagging/20250912/GN3EPCLV01/antikt4empflow/network.onnx"
       , "BTagging_network_8085e6c5717c"},
      {"JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20_CSSKUFO_bJR10v00Ext_20250212.onnx"
       , "JetCalibTools_bbJESJMS_calibFactor_80138d800ac5"},
      {"JetCalibTools/CalibArea-00-04-83/CalibrationFactors/bbJESJMS_calibFactors_R22_MC20MC23_CSSKUFO_bJR10v01_20250212.onnx"
       , "JetCalibTools_bbJESJMS_calibFactor_fefb85f452f9"}
    };
  }
#endif
}
