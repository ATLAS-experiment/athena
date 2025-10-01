// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODTruthCnv/TruthVertexFixerAlg.h"

#include "removePrefix.h"

// Framework include(s).
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s).
#include "xAODCore/AuxContainerBase.h"
#include "xAODTruth/TruthParticleContainer.h"

// System include(s).
#include <cassert>

namespace xAODMaker {

StatusCode TruthVertexFixerAlg::initialize() {

  // initialize handles
  ANA_CHECK(m_inputContainerKey.initialize());
  ANA_CHECK(m_outputContainerKey.initialize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TruthVertexFixerAlg::execute(const EventContext& ctx) const {

  // Access the input container.
  auto inputContainer = SG::makeHandle(m_inputContainerKey, ctx);

  // Create the output container and its auxiliary store.
  auto outputContainer = std::make_unique<xAOD::TruthVertexContainer>();
  auto outputAuxContainer = std::make_unique<xAOD::AuxContainerBase>();
  outputContainer->setStore(outputAuxContainer.get());

  // Variable(s) specific to the old schema.
  static const SG::ConstAccessor<int> barcodeAcc("barcode");
  static const SG::ConstAccessor<int> idAcc("id");

  // Deep-copy the objects in a relatively slow/inefficient way.
  bool warningPrinted = false;
  for (const xAOD::TruthVertex* input : *inputContainer) {
    // Create the output vertex as a (deep) copy of the input one.
    xAOD::TruthVertex* output = new xAOD::TruthVertex();
    outputContainer->push_back(output);
    *output = *input;
    // Fix up the output vertex if needed.
    if (barcodeAcc.isAvailable(*output) && idAcc.isAvailable(*output)) {
      // Access the variables copied from the in-file container.
      const int id = idAcc(*output);
      const int barcode = barcodeAcc(*output);
      // convert "old" id + barcode to "new" status values
      output->setStatus(HepMC::new_vertex_status_from_old(id, barcode));
      // The old barcode is still a unique identifier
      output->setUid(barcode);
    } else if (warningPrinted == false) {
      // Print a warning if not, but don't fail.
      ANA_MSG_WARNING(
          "One or both of xAOD::TruthVertex barcode and id are not available. "
          "This algorithm should not be run with these inputs.");
      warningPrinted = true;
    }

    // Fix the truth particle links on it if needed.
    for (const std::string& linkName : m_particleLinks.value()) {
      // Access the link in question.
      SG::Accessor<std::vector<ElementLink<xAOD::TruthParticleContainer>>>
          linkAcc(linkName);
      if (!linkAcc.isAvailable(*output)) {
        ANA_MSG_ERROR("TruthParticle link \"" << linkName
                                              << "\" is not available on "
                                                 "container \""
                                              << m_inputContainerKey.key()
                                              << "\"");
        return StatusCode::FAILURE;
      }
      auto& links = linkAcc(*output);
      // Update the name of the container that it's pointing to.
      for (auto& link : links) {
        link.resetWithKeyAndIndex(
            Details::removePrefix(link.dataID(), m_linkPrefixToRemove.value()),
            link.index());
      }
    }

    // Fix the truth vertex links on it if needed.
    for (const std::string& linkName : m_vertexLinks.value()) {
      // Access the link in question.
      SG::Accessor<std::vector<ElementLink<xAOD::TruthVertexContainer>>>
          linkAcc(linkName);
      if (!linkAcc.isAvailable(*output)) {
        ANA_MSG_ERROR("TruthParticle link \"" << linkName
                                              << "\" is not available on "
                                                 "container \""
                                              << m_inputContainerKey.key()
                                              << "\"");
        return StatusCode::FAILURE;
      }
      auto& links = linkAcc(*output);
      // Update the name of the container that it's pointing to.
      for (auto& link : links) {
        link.resetWithKeyAndIndex(
            Details::removePrefix(link.dataID(), m_linkPrefixToRemove.value()),
            link.index());
      }
    }
  }

  // Record the modified truth vertices.
  ANA_CHECK(
      SG::makeHandle(m_outputContainerKey, ctx)
          .record(std::move(outputContainer), std::move(outputAuxContainer)));
  ANA_MSG_DEBUG(
      "Recorded TruthVertexContainer with key: " << m_outputContainerKey.key());

  // Return gracefully.
  return StatusCode::SUCCESS;
}
}  // namespace xAODMaker
