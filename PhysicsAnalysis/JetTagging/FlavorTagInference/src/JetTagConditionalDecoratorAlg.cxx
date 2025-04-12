/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/JetTagConditionalDecoratorAlg.h"
#include "CxxUtils/checker_macros.h"

namespace FlavorTagInference {
  JetTagConditionalDecoratorAlg::JetTagConditionalDecoratorAlg(
    const std::string& name, ISvcLocator* svcloc):
    detail::JetCondTag_t(name, svcloc)
  {
  }

  StatusCode JetTagConditionalDecoratorAlg::initialize() {
    // this reserve might prevent a segfault
    m_tagFlagReadDecors.reserve(m_tagFlags.size());
    for (const auto& k: m_tagFlags) {
      std::string fullname = m_containerKey.key() + "." + k;
      m_tagFlagReadDecors.emplace_back(
        this, k, fullname, "condition for decorating");
    }
    ATH_CHECK(m_tagFlagReadDecors.initialize());
    ATH_CHECK(detail::JetCondTag_t::initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetTagConditionalDecoratorAlg::execute(
    const EventContext& cxt ) const {
    SG::ReadHandle container(m_containerKey, cxt);
    if (!container.isValid()) {
      ATH_MSG_ERROR("no container " << container.key());
      return StatusCode::FAILURE;
    }
    std::vector<SG::ReadDecorHandle<xAOD::JetContainer, char>> checks;
    for (const auto& k: m_tagFlagReadDecors) {
      checks.emplace_back(k, cxt);
    }
    ATH_MSG_DEBUG(
      "Decorating " + std::to_string(container->size()) + " elements");
    for (const auto* element: *container) {
      // check that every condition is nonzero in the element
      auto f = [&e=*element](const auto& c){ return c(e) != 0; };
      if (std::all_of(checks.begin(), checks.end(), f)) {
        m_decorator->decorate(*element);
      } else {
        m_decorator->decorateWithDefaults(*element);
      }
    }

    // Lock the decorations
    //
    // Ok --- we just made these decorations; no one else should be accessing
    // them yet.
    auto* container_nc ATLAS_THREAD_SAFE = const_cast<xAOD::JetContainer*>(container.get());
    for (SG::auxid_t id: m_auxids) {
      ATH_MSG_DEBUG("locking auxid " << id << " in " << m_containerKey.key());
      container_nc->lockDecoration(id);
    }

    return StatusCode::SUCCESS;
  }

}
