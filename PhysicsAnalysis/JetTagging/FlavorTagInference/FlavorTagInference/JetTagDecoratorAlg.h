/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_TAG_DECORATOR_ALG_H
#define JET_TAG_DECORATOR_ALG_H

#include "FlavorTagInference/DecoratorAlg.h"
#include "FlavorTagInference/IJetTagDecorator.h"

#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace detail {
  using JetTag_t = FlavorTagInference::DecoratorAlg<
    xAOD::JetContainer,
    IJetTagDecorator,
    xAOD::TrackParticleContainer
    >;
}

namespace FlavorTagInference {
  class JetTagDecoratorAlg : public detail::JetTag_t
  {
  public:
    JetTagDecoratorAlg(const std::string& name, ISvcLocator* svcloc);
    StatusCode initialize() override;
  };
}

#endif
