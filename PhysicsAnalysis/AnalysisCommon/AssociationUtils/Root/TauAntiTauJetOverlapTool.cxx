/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "AssociationUtils/TauAntiTauJetOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"
#include "AsgTools/CurrentContext.h"
#include "AsgDataHandles/ReadHandle.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  TauAntiTauJetOverlapTool::TauAntiTauJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name),
      m_bJetLabel(""),
      m_tauLabel(""),
      m_antiTauLabel(""),
      m_dR(0.2),
      m_useRapidity(true),
      m_dRMatcher(nullptr),
      m_antiTauDecHelper(nullptr)
  {
    declareProperty("BJetLabel", m_bJetLabel,
                    "Input b-jet flag. Disabled by default.");
    declareProperty("TauLabel", m_tauLabel,
                    "Decoration which labels ID-ed taus");
    declareProperty("AntiTauLabel", m_antiTauLabel,
                    "Decoration which labels anti-taus");
    declareProperty("antiTauEventCategory", m_antiTauEventCategoryDecorName="antiTauEventCategory",
                    "Decoration which labels event type");
    declareProperty("DR", m_dR, "Maximum dR for overlap match");
    declareProperty("UseRapidity", m_useRapidity,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode TauAntiTauJetOverlapTool::initializeDerived()
  {
    // Initialize the b-jet helper
    if(!m_bJetLabel.empty()) {
      ATH_MSG_DEBUG("Configuring btag-aware OR with btag label: " << m_bJetLabel);
      resetAccessor (m_accessors->m_bJetAcc, *m_accessors, m_bJetLabel);
    }

    // Initialize the dR matcher
    m_dRMatcher = std::make_unique<DeltaRMatcher>(m_dR, m_useRapidity);
    ATH_CHECK (m_dRMatcher->setObjectTypes (xAODType::ObjectType::Jet, xAODType::ObjectType::Tau));
    addSubtool(*m_dRMatcher);

    // Initialize the IDed-tau decoration helper
    if(!m_tauLabel.empty()) {
      ATH_MSG_DEBUG("Configuring tau OR with label: " << m_tauLabel);
      m_tauDecHelper =
        std::make_unique<OverlapDecorationHelper<columnar::ContainerId::particle2>>
          (m_tauLabel, m_outputLabel, m_outputPassValue);
      addSubtool(*m_tauDecHelper);
    }

    // Initialize the anti-tau decoration helper
    if(!m_antiTauLabel.empty()) {
      ATH_MSG_DEBUG("Configuring anti-tau OR with label: " << m_antiTauLabel);
      m_antiTauDecHelper =
        std::make_unique<OverlapDecorationHelper<columnar::ContainerId::particle2>>
          (m_antiTauLabel, m_outputLabel, m_outputPassValue);
      addSubtool(*m_antiTauDecHelper);
    }

    resetAccessor (m_accessors->m_evtAcc, *m_accessors, m_evtKeyName, {.addMTDependency = true});
    resetAccessor (m_accessors->m_categoryAcc, *m_accessors, m_antiTauEventCategoryDecorName);
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauAntiTauJetOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId eventContext) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::JetContainer>(cont1, "First container arg is not of type JetContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::TauJetContainer>(cont2, "Second container arg is not of type TauJetContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2, eventContext) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauAntiTauJetOverlapTool::
  internalFindOverlaps(columnar::Particle1Range jets,
                       columnar::Particle2Range taus,
                       columnar::EventContextId eventContext) const
  {
    ATH_MSG_DEBUG("Removing overlapping taus and jets");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(taus);
    initializeDecorations(jets);

    // Start by discarding all taus which are not ID or anti-ID
    for(const auto tau : taus) {
      if(!isSurvivingObject(tau)) continue;
      if(isSurvivingTau(tau)) continue;
      if(isSurvivingAntiTau(tau)) continue;
      // remove it with trivial overlap with itself
      ATH_CHECK( handleOverlap(tau, tau) );
    }

    // Remove bjets overlapping with ID taus
    int ntaus = 0;
    for(const auto tau : taus) {
      if(!isSurvivingObject(tau)) continue;
      // Only consider ID taus
      if(!isSurvivingTau(tau)) continue;
      ntaus++;
      for(const auto jet : jets) {
        if(!isSurvivingObject(jet)) continue;
        if(m_dRMatcher->objectsMatch(tau, jet)){
          ATH_CHECK( handleOverlap(jet, tau) );
        }
      }
    }

    // Remove anti-taus from remaining bjets
    for(const auto jet : jets) {
      if(!isSurvivingObject(jet)) continue;
      if(!isBJet(jet)) continue;
      for(const auto tau : taus) {
        if(!isSurvivingObject(tau)) continue;
        if(!isSurvivingAntiTau(tau)) continue;
        if(m_dRMatcher->objectsMatch(tau, jet)) {
          ATH_CHECK( handleOverlap(tau, jet) );
        }
      }
    }

    int nantitaus = 0;
    int antiTauCategory = 0;
    for(const auto tau : taus) {
      if(!isSurvivingObject(tau)) continue;
      if(!isSurvivingAntiTau(tau)) continue;
      nantitaus++;
      antiTauCategory = acc.m_categoryAcc(tau);
    }

    int nAntiTauMax = int(ntaus<antiTauCategory);

    // AntiTauCategory = 1 for lephad event, 2 for hadhad events
    // nAntiTauMax = 1 if we didn't get enough ID taus, 0 otherwise 
    auto eventInfo = acc.m_evtAcc(eventContext);
    auto eventIndex = eventInfo(acc.m_eventNumberAcc);    // pseudo-random selection of anti-taus    // pseudo-random selection of anti-taus
    if (nantitaus > 0) {
      int selIndex = eventIndex%nantitaus;

      int nSelectedAntitaus = 0;
      int idx = 0;
      for(const auto tau : taus) {
        if(!isSurvivingObject(tau)) continue;
        if(!isSurvivingAntiTau(tau) ) continue;
        if (idx == selIndex  && nSelectedAntitaus < nAntiTauMax) nSelectedAntitaus++;
        else {
          // remove excess anti-taus by applying OR fail (it trivially overlaps with itself)
          ATH_CHECK( handleOverlap(tau, tau) );
        }
        idx++;
      }
    }

    // Remove light jets from remaining anti-taus.
    for(const auto tau : taus) {
      if(!isSurvivingObject(tau)) continue;
      if(!isSurvivingAntiTau(tau)) continue;
      // if isSurviving
      for(const auto jet : jets) {
        if(!isSurvivingObject(jet)) continue;
        // We don't need to check the bjet label, but it might save CPU.
        if(isBJet(jet)) continue;
        if(m_dRMatcher->objectsMatch(tau, jet)){
          ATH_CHECK( handleOverlap(jet, tau) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify a user-labeled b-jet
  //---------------------------------------------------------------------------
  bool TauAntiTauJetOverlapTool::isBJet(columnar::Particle1Id jet) const
  {
    if(!m_bJetLabel.empty() && m_accessors->m_bJetAcc(jet)) return true;
    return false;
  }

  //---------------------------------------------------------------------------
  // Identify a user-labeled IDed-tau
  //---------------------------------------------------------------------------
  bool TauAntiTauJetOverlapTool::isSurvivingTau(columnar::Particle2Id tau) const
  {
    if(m_tauDecHelper && m_tauDecHelper->isSurvivingObject(tau)) return true;
    return false;
  }

  //---------------------------------------------------------------------------
  // Identify a user-labeled anti-tau
  //---------------------------------------------------------------------------
  bool TauAntiTauJetOverlapTool::isSurvivingAntiTau(columnar::Particle2Id tau) const
  {
    if(m_antiTauDecHelper && m_antiTauDecHelper->isSurvivingObject(tau))
      return true;
    return false;
  }
  

} // namespace ORUtils
