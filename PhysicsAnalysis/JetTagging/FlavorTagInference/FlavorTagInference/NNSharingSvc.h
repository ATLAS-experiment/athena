/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NN_SHARING_SVC_H
#define NN_SHARING_SVC_H

#include "FlavorTagInference/INNSharingSvc.h"
#include "AsgServices/AsgService.h"

#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/GNN.h"

namespace FlavorTagInference
{

  namespace detail {
    struct NNKey {
      std::string path;
      GNNOptions opts;
      bool operator==(const NNKey&) const;
      std::size_t hash() const;
    };
    struct NNHasher {
      std::size_t operator()(const NNKey& o) const {
        return o.hash();
      }
    };
  }

  class NNSharingSvc: public extends<asg::AsgService, INNSharingSvc>
  {
  public:
    using extends::extends;  // base class constructor
#ifndef XAOD_ANALYSIS
    virtual StatusCode initialize() override;
#endif
    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) override;
  private:
    using val_t = std::shared_ptr<const GNN>;
    std::unordered_map<detail::NNKey, val_t, detail::NNHasher> m_gnns;
    std::unordered_map<std::string, val_t> m_base_gnns;
#ifndef XAOD_ANALYSIS
    Gaudi::Property<bool> m_useTriton {this, "UseTriton", false
      , "Toggle running the inference through Triton"};
    Gaudi::Property<float> m_tritonTimeout {this, "TritonTimeout", 0.f
      , "Timeout value for Triton client"};
    Gaudi::Property<int> m_tritonPort {this, "TritonPort", 443
      , "Triton server port"};
    Gaudi::Property<std::string> m_tritonUrl {this, "TritonUrl", ""
      , "Triton server URL"};
    Gaudi::Property<bool> m_tritonUseSsl {this, "TritonUseSSL", true
      , "Connect to the Triton server over SSL"};

    // !!! ------ For testing purpose only! -------- !!!
    // In the long run we need to find some other mechanism
    // for mapping physical paths to model names
    std::map<std::string, std::string> m_tritonPathToName;
    void initTritonPathToName();
    // !!! ----------------------------------------- !!!
#endif
  };

}

#endif
