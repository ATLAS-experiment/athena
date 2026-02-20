/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// JetGhostThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/JetGhostThinning.h"
#include "StoreGate/ThinningHandle.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/AuxElement.h"

// Constructor
DerivationFramework::JetGhostThinning::JetGhostThinning(
    const std::string &t, const std::string &n, const IInterface *p)
    : base_class(t, n, p) {}

// Destructor
DerivationFramework::JetGhostThinning::~JetGhostThinning() = default;

// Athena initialize
StatusCode DerivationFramework::JetGhostThinning::initialize() {
  ATH_MSG_VERBOSE("initialize() ...");

  // Check that we have a jet container
  if (m_jetSGKey.empty()) {
    ATH_MSG_FATAL("No jet collection provided!");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_jetSGKey.initialize());
  ATH_MSG_INFO("Using " << m_jetSGKey.key() << " as the jet collection");

  // Initialize the selection string parser if provided
  if (!m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
    ATH_MSG_INFO("Jet selection string: " << m_selectionString);
  }

  // Check that we have a ghost name
  if (m_ghostName.empty()) {
    ATH_MSG_FATAL("No ghost name provided!");
    return StatusCode::FAILURE;
  }

  // Check that we have a ghost container name
  if (m_ghostContainerName.empty()) {
    ATH_MSG_FATAL("No ghost container name provided!");
    return StatusCode::FAILURE;
  }

  // Initialize thinning handle key
  m_ghostContainerKey = SG::ThinningHandleKey<xAOD::IParticleContainer>(m_ghostContainerName);
  ATH_CHECK(m_ghostContainerKey.initialize(m_streamName));

  ATH_MSG_INFO("Thinning ghost objects:");
  ATH_MSG_INFO("  Ghost name: " << m_ghostName);
  ATH_MSG_INFO("  Ghost container: " << m_ghostContainerName);

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::JetGhostThinning::finalize() {
  ATH_MSG_VERBOSE("finalize() ...");
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::JetGhostThinning::doThinning() const {
  const EventContext &ctx = Gaudi::Hive::currentContext();

  // Retrieve jet collection
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetSGKey, ctx);
  if (!jets.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve jet collection " << m_jetSGKey.key());
    return StatusCode::FAILURE;
  }

  // Get the jets that pass the selection
  std::vector<const xAOD::Jet*> selectedJets;

  if (!m_selectionString.empty()) {
    // Use the expression parser
    std::vector<int> entries = m_parser->evaluateAsVector();
    if (entries.size() != jets->size()) {
      ATH_MSG_ERROR("Selection string evaluation size mismatch!");
      return StatusCode::FAILURE;
    }

    for (size_t i = 0; i < jets->size(); ++i) {
      if (entries[i] == 1) {
        selectedJets.push_back((*jets)[i]);
      }
    }
  } else {
    // Keep all jets
    for (const auto* jet : *jets) {
      selectedJets.push_back(jet);
    }
  }

  ATH_MSG_DEBUG("Number of selected jets: " << selectedJets.size()
                << " out of " << jets->size());

  // Get thinning handle for the ghost container
  SG::ThinningHandle<xAOD::IParticleContainer> ghostContainer(m_ghostContainerKey, ctx);

  if (!ghostContainer.isValid()) {
    ATH_MSG_WARNING("Ghost container " << m_ghostContainerKey.key() << " not valid, skipping");
    return StatusCode::SUCCESS;
  }

  size_t nObjects = ghostContainer->size();
  std::vector<bool> mask(nObjects, false);
  const int maskSize = static_cast<int>(mask.size());

  // Create accessor for the ghost association
  SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> ghostAcc(m_ghostName);

  size_t nGhostLinks = 0;

  // Loop over selected jets and mark ghost objects to keep using their indices
  for (const auto* jet : selectedJets) {
    if (!jet) continue;

    // Check if this jet has the ghost association
    if (!ghostAcc.isAvailable(*jet)) {
      ATH_MSG_VERBOSE("Jet does not have ghost association: " << m_ghostName);
      continue;
    }

    // Get the ghost links
    const std::vector<ElementLink<xAOD::IParticleContainer>>& ghostLinks = ghostAcc(*jet);

    ATH_MSG_VERBOSE("Jet has " << ghostLinks.size() << " ghost objects");

    // Mark all valid ghost objects using their indices
    for (const auto& link : ghostLinks) {
      if (!link.isValid()) continue;

      int index = link.index();
      if (index < 0) {
        ATH_MSG_VERBOSE("  Ghost link has negative index, skipping");
        continue;
      }

      if (index < maskSize) {
        mask[index] = true;
        nGhostLinks++;
        const xAOD::IParticle* ghostObj = *link;
        if (ghostObj) {
          ATH_MSG_VERBOSE("  Ghost object at index " << index << ": pt=" << ghostObj->pt()
                         << ", eta=" << ghostObj->eta());
        }
      }
    }
  }

  size_t nKept = std::count(mask.begin(), mask.end(), true);
  ATH_MSG_DEBUG("Found " << nGhostLinks << " ghost links from selected jets");
  ATH_MSG_DEBUG("Ghost container " << m_ghostContainerKey.key() << ": keeping "
               << nKept << " out of " << nObjects << " objects");

  ghostContainer.keep(mask);

  return StatusCode::SUCCESS;
}
