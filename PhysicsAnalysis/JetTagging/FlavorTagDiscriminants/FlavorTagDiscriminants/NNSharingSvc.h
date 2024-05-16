/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NN_SHARING_SVC_H
#define NN_SHARING_SVC_H

#include "FlavorTagDiscriminants/INNSharingSvc.h"
#include "AsgServices/AsgService.h"

#include "FlavorTagDiscriminants/GNNOptions.h"
#include "FlavorTagDiscriminants/GNN.h"

namespace FlavorTagDiscriminants
{

  namespace detail {
    struct NNKey {
      std::string path;
      FlavorTagDiscriminants::GNNOptions opts;
      bool operator==(const NNKey&) const;
      std::size_t hash() const;
    };
    struct NNHasher {
      std::size_t operator()(const NNKey& o) const {
        return o.hash();
      }
    };
  }

  class NNSharingSvc: public asg::AsgService, public INNSharingSvc
  {
  public:
    NNSharingSvc(const std::string& name, ISvcLocator* svc);
    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) override;
  private:
    using val_t = std::shared_ptr<const GNN>;
    std::unordered_map<detail::NNKey, val_t, detail::NNHasher> m_gnns;
    std::unordered_map<std::string, val_t> m_base_gnns;
  };

}

#endif
