/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef I_NN_SHARING_SVC_H
#define I_NN_SHARING_SVC_H

#include "AsgServices/IAsgService.h"

#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/GNN.h"
#include "src/hash.h"

#include <memory>

namespace FlavorTagInference
{
  class INNSharingSvc: virtual public asg::IAsgService
  {
  public:
    DeclareInterfaceID(FlavorTagInference::INNSharingSvc, 1, 0);
    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) = 0;
  };
}

#endif
