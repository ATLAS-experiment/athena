/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/BTagConditionalDecoratorAlg.h"

namespace FlavorTagDiscriminants {
  BTagConditionalDecoratorAlg::BTagConditionalDecoratorAlg(
    const std::string& name, ISvcLocator* svcloc):
    detail::BCondTag_t(name, svcloc)
  {
  }

  StatusCode BTagConditionalDecoratorAlg::initialize() {
    m_tagFlagReadDecor = m_containerKey.key() + "." + m_tagFlag;
    ATH_CHECK(m_tagFlagReadDecor.initialize());
    ATH_CHECK(detail::BCondTag_t::initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode BTagConditionalDecoratorAlg::execute(
    const EventContext& cxt ) const {
    SG::ReadDecorHandle<xAOD::BTaggingContainer, char> container(
      m_tagFlagReadDecor, cxt);
    if (!container.isValid()) {
      ATH_MSG_ERROR("no container " << container.key());
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG(
      "Decorating " + std::to_string(container->size()) + " elements");
    for (const auto* element: *container) {
      if (container(*element)) {
        m_decorator->decorate(*element);
      } else {
        m_decorator->decorateWithDefaults(*element);
      }
    }
    return StatusCode::SUCCESS;
  }

}
