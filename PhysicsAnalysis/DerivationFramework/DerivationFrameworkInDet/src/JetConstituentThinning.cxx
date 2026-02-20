/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// JetConstituentThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/JetConstituentThinning.h"
#include "StoreGate/ThinningHandle.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/AuxElement.h"
#include <cmath>

DerivationFramework::JetConstituentThinning::JetConstituentThinning(
    const std::string &t, const std::string &n, const IInterface *p)
    : base_class(t, n, p) {}

DerivationFramework::JetConstituentThinning::~JetConstituentThinning() = default;

StatusCode DerivationFramework::JetConstituentThinning::initialize() {
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

  // Build container names from prefixes
  std::string jetChargedName = m_jetConstituentName + "ChargedParticleFlowObjects";
  std::string jetNeutralName = m_jetConstituentName + "NeutralParticleFlowObjects";
  std::string globalChargedName = m_globalConstituentName + "ChargedParticleFlowObjects";
  std::string globalNeutralName = m_globalConstituentName + "NeutralParticleFlowObjects";

  // Initialize thinning handle keys
  m_jetChargedKey = SG::ThinningHandleKey<xAOD::FlowElementContainer>(jetChargedName);
  m_jetNeutralKey = SG::ThinningHandleKey<xAOD::FlowElementContainer>(jetNeutralName);
  m_globalChargedKey = SG::ThinningHandleKey<xAOD::FlowElementContainer>(globalChargedName);
  m_globalNeutralKey = SG::ThinningHandleKey<xAOD::FlowElementContainer>(globalNeutralName);

  ATH_CHECK(m_jetChargedKey.initialize(m_streamName));
  ATH_CHECK(m_jetNeutralKey.initialize(m_streamName));
  ATH_CHECK(m_globalChargedKey.initialize(m_streamName));
  ATH_CHECK(m_globalNeutralKey.initialize(m_streamName));

  ATH_MSG_INFO("Thinning containers:");
  ATH_MSG_INFO("  Jet constituents: " << jetChargedName << ", " << jetNeutralName);
  ATH_MSG_INFO("  Global constituents: " << globalChargedName << ", " << globalNeutralName);

  // Initialize otherObjects thinning if requested
  if (!m_otherObjectsName.empty()) {
    m_otherObjectsKey = SG::ThinningHandleKey<xAOD::IParticleContainer>(m_otherObjectsName);
    ATH_CHECK(m_otherObjectsKey.initialize(m_streamName));
    ATH_MSG_INFO("  OtherObjects: " << m_otherObjectsName);
  } else {
    ATH_MSG_INFO("  OtherObjects thinning: disabled");
  }

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::JetConstituentThinning::finalize() {
  ATH_MSG_VERBOSE("finalize() ...");
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::JetConstituentThinning::doThinning() const {
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

  // Get thinning handles for all containers
  SG::ThinningHandle<xAOD::FlowElementContainer> jetChargedContainer(m_jetChargedKey, ctx);
  SG::ThinningHandle<xAOD::FlowElementContainer> jetNeutralContainer(m_jetNeutralKey, ctx);
  SG::ThinningHandle<xAOD::FlowElementContainer> globalChargedContainer(m_globalChargedKey, ctx);
  SG::ThinningHandle<xAOD::FlowElementContainer> globalNeutralContainer(m_globalNeutralKey, ctx);

  // Initialize masks for all containers
  std::vector<bool> jetChargedMask(jetChargedContainer.isValid() ? jetChargedContainer->size() : 0, false);
  std::vector<bool> jetNeutralMask(jetNeutralContainer.isValid() ? jetNeutralContainer->size() : 0, false);
  std::vector<bool> globalChargedMask(globalChargedContainer.isValid() ? globalChargedContainer->size() : 0, false);
  std::vector<bool> globalNeutralMask(globalNeutralContainer.isValid() ? globalNeutralContainer->size() : 0, false);

  // Cache mask sizes as int for comparison (avoid repeated static_cast)
  const int jetChargedMaskSize = static_cast<int>(jetChargedMask.size());
  const int jetNeutralMaskSize = static_cast<int>(jetNeutralMask.size());
  const int globalChargedMaskSize = static_cast<int>(globalChargedMask.size());
  const int globalNeutralMaskSize = static_cast<int>(globalNeutralMask.size());

  // Accessor for originalObjectLink
  static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::IParticleContainer>> originalAcc("originalObjectLink");

  // Collect otherObjects indices if needed
  std::vector<const xAOD::IParticle*> otherObjects;

  // Loop over selected jets and mark constituents to keep
  for (const auto* jet : selectedJets) {
    if (!jet) continue;

    ATH_MSG_VERBOSE("Processing jet with " << jet->numConstituents() << " constituents");

    for (size_t i = 0; i < jet->numConstituents(); ++i) {
      const auto& link = jet->constituentLinks().at(i);
      if (!link.isValid()) {
        ATH_MSG_VERBOSE("  Invalid constituent link at index " << i);
        continue;
      }

      int index = link.index();
      if (index < 0) {
        ATH_MSG_VERBOSE("  Constituent link has negative index, skipping");
        continue;
      }

      // Cast to FlowElement (all PFlow constituents are FlowElements)
      const xAOD::FlowElement* flowElement = dynamic_cast<const xAOD::FlowElement*>(*link);
      if (!flowElement) {
        ATH_MSG_VERBOSE("  Constituent at index " << i << " is not a FlowElement, skipping");
        continue;
      }

      // Check if it's charged or neutral
      bool isCharged = flowElement->isCharged();

      // Mark in the jet constituent container mask
      if (isCharged) {
        if (index < jetChargedMaskSize) {
          jetChargedMask[index] = true;
          ATH_MSG_VERBOSE("  Charged constituent at index " << index << ": pt=" << flowElement->pt()
                         << ", eta=" << flowElement->eta());
        }
      } else {
        if (index < jetNeutralMaskSize) {
          jetNeutralMask[index] = true;
          ATH_MSG_VERBOSE("  Neutral constituent at index " << index << ": pt=" << flowElement->pt()
                         << ", eta=" << flowElement->eta());
        }
      }

      // Mark in the global container mask using originalObjectLink
      if (originalAcc.isAvailable(*flowElement)) {
        const ElementLink<xAOD::IParticleContainer>& origLink = originalAcc(*flowElement);
        if (origLink.isValid()) {
          int origIndex = origLink.index();
          if (origIndex >= 0) {
            if (isCharged && origIndex < globalChargedMaskSize) {
              globalChargedMask[origIndex] = true;
              ATH_MSG_VERBOSE("  Global charged constituent at index " << origIndex);
            } else if (!isCharged && origIndex < globalNeutralMaskSize) {
              globalNeutralMask[origIndex] = true;
              ATH_MSG_VERBOSE("  Global neutral constituent at index " << origIndex);
            }
          }
        }
      }

      // Collect otherObjects for thinning if requested
      if (!m_otherObjectsName.empty()) {
        std::vector<const xAOD::IParticle*> others = flowElement->otherObjects();
        otherObjects.insert(otherObjects.end(), others.begin(), others.end());
      }
    }
  }

  size_t nChargedKept = std::count(jetChargedMask.begin(), jetChargedMask.end(), true);
  size_t nNeutralKept = std::count(jetNeutralMask.begin(), jetNeutralMask.end(), true);
  ATH_MSG_DEBUG("Keeping " << nChargedKept << " charged constituents and "
               << nNeutralKept << " neutral constituents from selected jets");

  if (!m_otherObjectsName.empty()) {
    ATH_MSG_DEBUG("Found " << otherObjects.size() << " otherObjects from constituents");
  }

  
  if (jetChargedContainer.isValid()) {
    jetChargedContainer.keep(jetChargedMask);
    ATH_MSG_DEBUG("Container " << m_jetChargedKey.key() << ": keeping "
                 << nChargedKept << " out of " << jetChargedContainer->size() << " objects");
  } else {
    ATH_MSG_WARNING("Container " << m_jetChargedKey.key() << " not valid, skipping");
  }

  if (jetNeutralContainer.isValid()) {
    jetNeutralContainer.keep(jetNeutralMask);
    size_t nNeutralGlobalKept = std::count(jetNeutralMask.begin(), jetNeutralMask.end(), true);
    ATH_MSG_DEBUG("Container " << m_jetNeutralKey.key() << ": keeping "
                 << nNeutralGlobalKept << " out of " << jetNeutralContainer->size() << " objects");
  } else {
    ATH_MSG_WARNING("Container " << m_jetNeutralKey.key() << " not valid, skipping");
  }

  if (globalChargedContainer.isValid()) {
    globalChargedContainer.keep(globalChargedMask);
    size_t nGlobalChargedKept = std::count(globalChargedMask.begin(), globalChargedMask.end(), true);
    ATH_MSG_DEBUG("Container " << m_globalChargedKey.key() << ": keeping "
                 << nGlobalChargedKept << " out of " << globalChargedContainer->size() << " objects");
  } else {
    ATH_MSG_WARNING("Container " << m_globalChargedKey.key() << " not valid, skipping");
  }

  if (globalNeutralContainer.isValid()) {
    globalNeutralContainer.keep(globalNeutralMask);
    size_t nGlobalNeutralKept = std::count(globalNeutralMask.begin(), globalNeutralMask.end(), true);
    ATH_MSG_DEBUG("Container " << m_globalNeutralKey.key() << ": keeping "
                 << nGlobalNeutralKept << " out of " << globalNeutralContainer->size() << " objects");
  } else {
    ATH_MSG_WARNING("Container " << m_globalNeutralKey.key() << " not valid, skipping");
  }

  // Thin otherObjects container if requested
  if (!m_otherObjectsName.empty()) {
    SG::ThinningHandle<xAOD::IParticleContainer> otherObjectsContainer(m_otherObjectsKey, ctx);

    if (!otherObjectsContainer.isValid()) {
      ATH_MSG_WARNING("OtherObjects container " << m_otherObjectsKey.key() << " not valid, skipping");
    } else {
      size_t nObjects = otherObjectsContainer->size();
      std::vector<bool> mask(nObjects, false);
      const int maskSize = static_cast<int>(mask.size());

      // Mark objects using their indices
      for (const auto* other : otherObjects) {
        if (!other) continue;
        int index = other->index();
        if (index >= 0 && index < maskSize) {
          mask[index] = true;
        }
      }

      size_t nKept = std::count(mask.begin(), mask.end(), true);
      ATH_MSG_DEBUG("OtherObjects container " << m_otherObjectsKey.key() << ": keeping "
                   << nKept << " out of " << nObjects << " objects");

      otherObjectsContainer.keep(mask);
    }
  }

  return StatusCode::SUCCESS;
}
