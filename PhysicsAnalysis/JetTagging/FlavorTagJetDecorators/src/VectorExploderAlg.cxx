/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
 * @file VectorExploderAlg.cxx
 * @brief Algorithm to "explode" a vector<float> jet decoration into
 *        multiple scalar float decorations, one per vector index.
 *
 * Ported from TDD FTagDumper/src/VectorExploderAlg.cxx.
 * Namespace FlavorTagJetDecorators; hardcoded xAOD::JetContainer.
 ****************************************************************/

#include "VectorExploderAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace FlavorTagJetDecorators {

VectorExploderAlg::VectorExploderAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode VectorExploderAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_collectionKey.initialize());
  ATH_CHECK(m_inputVectorKey.initialize());

  if (m_outputNamesMap.value().empty()) {
    ATH_MSG_ERROR("OutputNamesMap property must contain at least one entry.");
    return StatusCode::FAILURE;
  }

  // Create and initialize the output decoration keys.
  // We MUST reserve first: declare(key) saves a pointer to the key object,
  // so if the vector resizes the pointer is invalidated → segfault.
  m_maxIndex = -1;
  m_outputKeys.reserve(m_outputNamesMap.value().size());
  for (const auto& [index, outName] : m_outputNamesMap.value()) {
    if (index > m_maxIndex) m_maxIndex = index;
    m_outputKeys.emplace_back(index,
      SG::WriteDecorHandleKey<xAOD::JetContainer>{
        m_collectionKey.key() + "." + outName});
    auto& key = m_outputKeys.back().second;
    ATH_CHECK(key.initialize());
    declare(key);
  }

  return StatusCode::SUCCESS;
}

StatusCode VectorExploderAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << "...");

  SG::ReadHandle<xAOD::JetContainer> collection(m_collectionKey, ctx);
  if (!collection.isValid()) {
    ATH_MSG_ERROR("Could not retrieve jet collection with key "
                  << m_collectionKey.key());
    return StatusCode::FAILURE;
  }

  // Input read-decor handle for the vector<float> decoration.
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<float>>
    vectorReader(m_inputVectorKey, ctx);

  // Build write handles once per event (not per jet).
  std::vector<std::pair<int, SG::WriteDecorHandle<xAOD::JetContainer, float>>>
    outHandles;
  outHandles.reserve(m_outputKeys.size());
  for (const auto& [index, key] : m_outputKeys) {
    outHandles.emplace_back(index,
      SG::WriteDecorHandle<xAOD::JetContainer, float>(key, ctx));
  }

  for (const xAOD::Jet* jet : *collection) {
    if (!vectorReader.isAvailable()) {
      ATH_MSG_ERROR("Input vector decoration '"
                    << m_inputVectorKey.key()
                    << "' not found for the specified jet collection.");
      return StatusCode::FAILURE;
    }

    const auto& inVec = vectorReader(*jet);

    if (inVec.size() < static_cast<size_t>(m_maxIndex + 1)) {
      ATH_MSG_ERROR("Input vector size " << inVec.size()
                    << " is smaller than the maximum index used in "
                    << "OutputNamesMap (" << m_maxIndex << ")");
      return StatusCode::FAILURE;
    }

    for (auto& [index, handle] : outHandles) {
      handle(*jet) = inVec.at(index);
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace FlavorTagJetDecorators
