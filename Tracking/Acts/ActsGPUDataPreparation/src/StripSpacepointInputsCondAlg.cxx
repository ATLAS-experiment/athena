/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StripSpacepointInputsCondAlg.h"

#include <exception>
#include <vecmem/memory/host_memory_resource.hpp>

#include "InDetIdentifier/SCT_ID.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "StripSpacepointAdapter.h"

namespace ActsTrk {
StatusCode StripSpacepointInputsCondAlg::initialize() {
  ATH_CHECK(m_elements.initialize());
  ATH_CHECK(m_properties.initialize());
  ATH_CHECK(m_output.initialize());
  ATH_CHECK(m_provider.retrieve());
  ATH_CHECK(detStore()->retrieve(m_mapping, m_mappingName.value()));
  ATH_CHECK(detStore()->retrieve(m_stripId, "SCT_ID"));
  return StatusCode::SUCCESS;
}

StatusCode StripSpacepointInputsCondAlg::execute(
    const EventContext& ctx) const {
  SG::WriteCondHandle<StripSpacepointConditions> output(m_output, ctx);
  // The scheduler may revisit an already available IOV for out-of-order events.
  if (output.isValid())
    return StatusCode::SUCCESS;
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> elements(m_elements,
                                                                    ctx);
  SG::ReadCondHandle<InDet::SiElementPropertiesTable> properties(m_properties,
                                                                 ctx);
  ATH_CHECK(elements.isValid());
  ATH_CHECK(properties.isValid());
  output.addDependency(elements);
  output.addDependency(properties);
  try {
    // Host staging is needed only until upload completes. Use the existing
    // static mapping directly, without building another Identifier map.
    vecmem::host_memory_resource hostMR;
    const auto inputs = makeStripSpacepointInputs(
        **elements, *m_stripId, **properties, *m_mapping, hostMR);
    auto deviceInputs = m_provider->uploadInputs(inputs, ctx);
    // uploadInputs waits for completion before the CDO is published, allowing
    // subsequent events to read it from their own CUDA streams.
    ATH_CHECK(output.record(std::move(deviceInputs)));
    ATH_MSG_DEBUG("Recorded Strip SP inputs for " << output.getRange());
  } catch (const std::exception& error) {
    ATH_MSG_ERROR("Strip SP conditions preparation failed: " << error.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
