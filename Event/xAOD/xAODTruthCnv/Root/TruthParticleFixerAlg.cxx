// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODTruthCnv/TruthParticleFixerAlg.h"

// Framework include(s).
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s).
#include "xAODCore/AuxContainerBase.h"

// System include(s).
#include <cassert>

namespace xAODMaker {

StatusCode TruthParticleFixerAlg::initialize() {

  // initialize handles
  ANA_CHECK(m_inputContainerKey.initialize());
  ANA_CHECK(m_outputContainerKey.initialize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TruthParticleFixerAlg::execute(const EventContext& ctx) const {

  // Access the input container.
  auto inputContainer = SG::makeHandle(m_inputContainerKey, ctx);

  // Create the output container and its auxiliary store.
  auto outputContainer = std::make_unique<xAOD::TruthParticleContainer>();
  auto outputAuxContainer = std::make_unique<xAOD::AuxContainerBase>();
  outputContainer->setStore(outputAuxContainer.get());

  // Variable specific to the old schema.
  static const SG::ConstAccessor<int> barcodeAcc("barcode");

  // Deep-copy the objects in a relatively slow/inefficient way.
  bool warningPrinted = false;
  for (const xAOD::TruthParticle* input : *inputContainer) {
    // Create the output particle as a (deep) copy of the input one.
    xAOD::TruthParticle* output = new xAOD::TruthParticle();
    outputContainer->push_back(output);
    *output = *input;
    // Fix up the output particle if needed.
    if (barcodeAcc.isAvailable(*output)) {
      // Access the barcode copied from the in-file container.
      const int barcode = barcodeAcc(*output);
      // convert "old" status + barcode to "new" status values
      output->setStatus(
          HepMC::new_particle_status_from_old(output->status(), barcode));
      // The old barcode is still a unique identifier
      output->setUid(barcode);
    } else if (warningPrinted == false) {
      // Print a warning if not, but don't fail.
      ANA_MSG_WARNING(
          "xAOD::TruthParticle barcode is not available. This algorithm should "
          "not be run with the current input.");
      warningPrinted = true;
    }
  }

  // Record the modified truth particles.
  ANA_CHECK(
      SG::makeHandle(m_outputContainerKey, ctx)
          .record(std::move(outputContainer), std::move(outputAuxContainer)));
  ANA_MSG_DEBUG("Recorded TruthParticleContainer with key: "
                << m_outputContainerKey.key());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace xAODMaker
