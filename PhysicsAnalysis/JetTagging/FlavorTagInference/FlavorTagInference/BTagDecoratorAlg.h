/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAG_DECORATOR_ALG_H
#define BTAG_DECORATOR_ALG_H


#include "FlavorTagInference/DecoratorAlg.h"
#include "FlavorTagInference/IBTagDecorator.h"

#include "xAODBTagging/BTaggingContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace detail {
  using BTag_t = FlavorTagInference::DecoratorAlg<
    xAOD::BTaggingContainer,
    IBTagDecorator,
    xAOD::TrackParticleContainer
    >;
}

namespace FlavorTagInference {
  class BTagDecoratorAlg : public detail::BTag_t
  {
  public:
    BTagDecoratorAlg(const std::string& name, ISvcLocator* svcloc);
    StatusCode initialize() override;
  };
}

#endif
