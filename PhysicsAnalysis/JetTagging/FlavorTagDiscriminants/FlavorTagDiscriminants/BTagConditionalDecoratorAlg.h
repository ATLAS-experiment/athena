/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef B_TAG_CONDITIONALDECORATOR_ALG_H
#define B_TAG_CONDITIONALDECORATOR_ALG_H

#include "FlavorTagDiscriminants/DecoratorAlg.h"
#include "FlavorTagInference/IBTagConditionalDecorator.h"

#include "xAODBTagging/BTaggingContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace detail {
  using BCondTag_t = FlavorTagDiscriminants::DecoratorAlg<
    xAOD::BTaggingContainer,
    IBTagConditionalDecorator,
    xAOD::TrackParticleContainer
    >;
}

namespace FlavorTagDiscriminants {
  class BTagConditionalDecoratorAlg : public detail::BCondTag_t
  {
  public:
    BTagConditionalDecoratorAlg(const std::string& name,
                                  ISvcLocator* svcloc);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& cxt ) const override;
  private:
    Gaudi::Property<std::string>  m_tagFlag {
      this, "tagFlag", "", "BTag variable to flag a jet for tagging"
    };
    SG::ReadDecorHandleKey<xAOD::BTaggingContainer> m_tagFlagReadDecor{
      this, "fullTagFlag", "", "filled internally"};
  };
}

#endif
