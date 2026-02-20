/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "AssociationUtils/TauLooseMuOverlapTool.h"

namespace
{
  /// Unit conversion constants
  const float GeV = 1e3;
}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  TauLooseMuOverlapTool::TauLooseMuOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("DR", m_maxDR = 0.2,
                    "Delta-R cone for flagging overlaps");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
    declareProperty("MinMuonPt", m_minMuPt = 2.*GeV,
                    "Minimum muon PT for rejecting taus");
    declareProperty("MinTauPtCombinedMuon", m_minTauPtMuComb = 50.*GeV,
                    "Tau PT threshold to compare to combined muons only");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode TauLooseMuOverlapTool::initializeDerived()
  {

    // Initialize the dR matcher
    m_dRMatcher = std::make_unique<DeltaRMatcher> (m_maxDR, m_useRapidity);
    ATH_CHECK (m_dRMatcher->setObjectTypes (xAODType::ObjectType::Tau, xAODType::ObjectType::Muon));
    addSubtool(*m_dRMatcher);

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauLooseMuOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::TauJetContainer>(cont1, "First container arg is not of type TauJetContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::MuonContainer>(cont2, "Second container arg is not of type MuonContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauLooseMuOverlapTool::
  internalFindOverlaps(columnar::Particle1Range taus,
                       columnar::Particle2Range muons) const
  {
    ATH_MSG_DEBUG("Removing taus from loose muons");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(taus);
    initializeDecorations(muons);

    // Loop over loose surviving muons
    for(auto muon : muons){

      // It's not obvious that I should be skipping objects that were
      // flagged as overlap. Perhaps this deserves more thought/study.
      if(isRejectedObject(muon)) continue;
      if(muon(acc.m_muPtAcc) < m_minMuPt) continue;
      bool isCombined = (muon(acc.m_muonTypeAcc) == xAOD::Muon::Combined);

      // Loop over surviving taus
      for(auto tau : taus){
        if(!isSurvivingObject(tau)) continue;

        // High-PT taus are only compared to combined muons
        if((tau(acc.m_tauPtAcc) > m_minTauPtMuComb) && !isCombined) continue;

        // Test for overlap
        if(m_dRMatcher->objectsMatch(muon, tau)){
          ATH_CHECK( handleOverlap(tau, muon) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
