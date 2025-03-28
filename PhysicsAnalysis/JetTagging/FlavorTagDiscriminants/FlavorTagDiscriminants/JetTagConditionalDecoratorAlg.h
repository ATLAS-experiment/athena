/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_TAG_CONDITIONALDECORATOR_ALG_H
#define JET_TAG_CONDITIONALDECORATOR_ALG_H

#include "FlavorTagDiscriminants/DecoratorAlg.h"
#include "FlavorTagInference/IJetTagConditionalDecorator.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

// needed to parse set<string>
#include "Gaudi/Parsers/Factory.h"

namespace detail {
  using JetCondTag_t = FlavorTagDiscriminants::DecoratorAlg<
    xAOD::JetContainer,
    IJetTagConditionalDecorator,
    xAOD::TrackParticleContainer
    >;
}

namespace FlavorTagDiscriminants {
  class JetTagConditionalDecoratorAlg : public detail::JetCondTag_t
  {
  public:
    JetTagConditionalDecoratorAlg(const std::string& name,
                                  ISvcLocator* svcloc);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& cxt ) const override;
  private:
    Gaudi::Property<std::unordered_set<std::string>>  m_tagFlags {
      this, "tagFlags", {}, "Jet variable to flag a jet for tagging"
    };
    SG::ReadDecorHandleKeyArray<xAOD::JetContainer> m_tagFlagReadDecors;
  };
}

#endif
