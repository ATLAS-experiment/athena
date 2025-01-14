/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LINK_COUNTER_ALG_H
#define LINK_COUNTER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODBase/IParticleContainer.h"

namespace FlavorTagDiscriminants {

  template <typename T, typename C>
  class LinkCounterAlg : public AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& cxt) const override;
  protected:
    SG::ReadDecorHandleKey<C> m_links {
      this, "links", "", "name of linked container"
    };
    SG::WriteDecorHandleKey<C> m_flag {
      this, "flag", "applyGNN", "flag to confirm GNN application"
    };
    Gaudi::Property<size_t> m_minimumLinks {
      this, "minimumLinks", 0, "minimum number of links required to tag a jet"
    };
  };

}

#include "LinkCounterAlg.icc"

#endif
