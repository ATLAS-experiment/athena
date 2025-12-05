/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "AssociationUtils/EleJetOverlapTool.h"

namespace
{
  /// Unit conversion constants
  const double GeV = 1e3;
}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  EleJetOverlapTool::EleJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("BJetLabel", m_bJetLabel = "",
                    "Input b-jet flag. Disabled by default.");
    declareProperty("MaxElePtForBJetAwareOR", m_maxElePtForBJetAwareOR = 100.*GeV,
                    "Max electron PT for b-tag aware OR. 100 GeV by default.");
    declareProperty("ApplyPtRatio", m_applyPtRatio = false,
                    "Toggle ele/jet PT ratio requirement");
    declareProperty("EleJetPtRatio", m_eleJetPtRatio = 0.8,
                    "Ele/jet PT ratio threshold to remove a jet");
    declareProperty("InnerDR", m_innerDR = 0.2,
                    "Inner cone for removing jets");
    declareProperty("OuterDR", m_outerDR = 0.4,
                    "Outer cone for removing electrons");
    declareProperty("UseSlidingDR", m_useSlidingDR = false,
                    "Use sliding dR cone to reject electrons");
    declareProperty("SlidingDRC1", m_slidingDRC1 = 0.04,
                    "The constant offset for sliding dR");
    declareProperty("SlidingDRC2", m_slidingDRC2 = 10.*GeV,
                    "The inverse muon pt factor for sliding dR");
    declareProperty("SlidingDRMaxCone", m_slidingDRMaxCone = 0.4,
                    "Maximum size of sliding dR cone");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode EleJetOverlapTool::initializeDerived()
  {
    // Initialize the b-jet helper
    if(!m_bJetLabel.empty()) {

      if (m_maxElePtForBJetAwareOR < 0){
        m_maxElePtForBJetAwareOR = std::numeric_limits<double>::max();
        ATH_MSG_DEBUG("Configuring btag-aware OR with btag label " <<
                      m_bJetLabel << " for all electrons");
      }
      else{
        ATH_MSG_DEBUG("Configuring btag-aware OR with btag label " <<
                      m_bJetLabel << " for electrons below "
                      << m_maxElePtForBJetAwareOR/GeV << " GeV");
      }
      resetAccessor (m_accessors->m_bJetAcc, *m_accessors, m_bJetLabel);
    }

    // Initialize the dR matchers
    ATH_MSG_DEBUG("Configuring ele-jet inner cone size " << m_innerDR);
    m_dRMatchCone1 = std::make_unique<DeltaRMatcher>(m_innerDR, m_useRapidity);
    ATH_CHECK (m_dRMatchCone1->setObjectTypes (xAODType::ObjectType::Electron, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone1);
    if(m_useSlidingDR) {
      ATH_MSG_DEBUG("Configuring sliding outer cone for ele-jet OR with " <<
                    "constants C1 = " << m_slidingDRC1 << ", C2 = " <<
                    m_slidingDRC2 << ", MaxCone = " << m_slidingDRMaxCone);
      m_dRMatchCone2 =
        std::make_unique<SlidingDeltaRMatcher>
          (m_slidingDRC1, m_slidingDRC2, m_slidingDRMaxCone, m_useRapidity);
    }
    else {
      ATH_MSG_DEBUG("Configuring ele-jet outer cone size " << m_outerDR);
      m_dRMatchCone2 = std::make_unique<DeltaRMatcher>(m_outerDR, m_useRapidity);
    }
    ATH_CHECK (m_dRMatchCone2->setObjectTypes (xAODType::ObjectType::Electron, xAODType::ObjectType::Jet));
    addSubtool(*m_dRMatchCone2);

    // Additional debug printouts
    if(m_applyPtRatio) {
      ATH_MSG_DEBUG("Apply ele/jet PT ratio requirement at " << m_eleJetPtRatio);
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode EleJetOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    // Check the container types
    ATH_CHECK (checkForXAODContainer<xAOD::ElectronContainer>(cont1, "First container arg is not an ElectronContainer!"));
    ATH_CHECK (checkForXAODContainer<xAOD::JetContainer>(cont2, "Second container arg is not of type JetContainer!"));

    ATH_CHECK( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode EleJetOverlapTool::
  internalFindOverlaps(columnar::Particle1Range electrons,
                       columnar::Particle2Range jets) const
  {
    ATH_MSG_DEBUG("Removing overlapping electrons and jets");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(electrons);
    initializeDecorations(jets);

    // First flag overlapping jets
    for(const auto electron : electrons){
      if(!isSurvivingObject(electron)) continue;

      for(const auto jet : jets){
        if(!isSurvivingObject(jet)) continue;
        // Don't reject user-defined b-tagged jets below an electron pT threshold
        if(!m_bJetLabel.empty() && acc.m_bJetAcc(jet) &&
           electron(acc.m_elePtAcc) < m_maxElePtForBJetAwareOR) continue;
        // Don't reject jets with high relative PT
        if(m_applyPtRatio && (electron(acc.m_elePtAcc)/jet(acc.m_jetPtAcc) < m_eleJetPtRatio)) continue;

        if(m_dRMatchCone1->objectsMatch(jet, electron)){
          ATH_CHECK( handleOverlap(jet, electron) );
        }
      }
    }

    // Now flag overlapping electrons
    for(const auto jet: jets){
      if(!isSurvivingObject(jet)) continue;

      for(const auto electron : electrons){
        if(!isSurvivingObject(electron)) continue;

        if(m_dRMatchCone2->objectsMatch(electron, jet)){
          ATH_CHECK( handleOverlap(electron, jet) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
