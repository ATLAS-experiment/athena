/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/BTagConditionalDecoratorAlg.h"
#include "CxxUtils/checker_macros.h"

namespace FlavorTagInference {
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

    // Lock the decorations
    //
    // Ok --- we just made these decorations; no one else should be accessing
    // them yet.
    auto* container_nc ATLAS_THREAD_SAFE = const_cast<xAOD::BTaggingContainer*>(container.get());
    for (SG::auxid_t id: m_auxids) {
      ATH_MSG_DEBUG("locking auxid " << id << " in " << m_containerKey.key());
      container_nc->lockDecoration(id);
    }

    return StatusCode::SUCCESS;
  }

}
