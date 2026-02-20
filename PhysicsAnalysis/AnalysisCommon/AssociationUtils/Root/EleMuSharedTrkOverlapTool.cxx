/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// EDM includes
#include "xAODEgamma/ElectronxAODHelpers.h"

// Local includes
#include "AssociationUtils/EleMuSharedTrkOverlapTool.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  EleMuSharedTrkOverlapTool::EleMuSharedTrkOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("RemoveCaloMuons", m_removeCaloMuons = true,
                    "Turn on removal of overlapping calo muons");
    declareProperty("UseDRMatching", m_useDRMatching = false,
                    "Remove electrons in DR cone of muons");
    declareProperty("DR", m_maxDR = 0.01,
                    "Delta-R cone for flagging overlaps");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize the tool
  //---------------------------------------------------------------------------
  StatusCode EleMuSharedTrkOverlapTool::initializeDerived()
  {

    if(m_removeCaloMuons) {
      ATH_MSG_DEBUG("Configuring removal of overlapping calo muons");
    }

    if(m_useDRMatching){
      ATH_MSG_DEBUG("Configuring removal of electrons in delta R cone of " << m_maxDR);
      m_dRMatcher = std::make_unique<DeltaRMatcher>(m_maxDR, m_useRapidity);
      ATH_CHECK (m_dRMatcher->setObjectTypes (xAODType::ObjectType::Electron, xAODType::ObjectType::Muon));
      addSubtool(*m_dRMatcher);
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode EleMuSharedTrkOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::ElectronContainer>(cont1, "First container arg is not of type ElectronContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::MuonContainer>(cont2, "Second container arg is not of type MuonContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps between electrons and muons
  //---------------------------------------------------------------------------
  StatusCode EleMuSharedTrkOverlapTool::
  internalFindOverlaps(columnar::Particle1Range electrons,
                       columnar::Particle2Range muons) const
  {
    ATH_MSG_DEBUG("Removing overlapping electrons and muons");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(electrons);
    initializeDecorations(muons);

    // If removing calo-muons that overlap with electrons,
    // then we need to do it in a separate loop first.
    if(m_removeCaloMuons) {

      // Loop over electrons
      for(const auto electron : electrons){
        if(!isSurvivingObject(electron)) continue;

        // Get the original ID track
        auto elTrk = getOriginalTrackParticle(electron);

        // Loop over input calo muons
        for(const auto muon : muons) {
          if(!isSurvivingObject(muon)) continue;
          if(muon(acc.m_muonTypeAcc) != xAOD::Muon::CaloTagged) continue;

          // Get the muon ID track
          auto muTrk = muon(acc.m_muonTrkAcc);
          // Flag the calo muon as overlapping if they share the track
          if(elTrk == muTrk) {
            ATH_CHECK( handleOverlap(muon, electron) );
          }
        }
      }
    }

    // Loop over muons
    for(const auto muon : muons){
      if(!isSurvivingObject(muon)) continue;

      // Get the muon ID track
      auto muTrk = muon(acc.m_muonTrkAcc);

      // Loop over electrons
      for(const auto electron : electrons) {
        if(!isSurvivingObject(electron)) continue;

        // Get the original ID track
        auto elTrk = getOriginalTrackParticle(electron);

        // Flag the electron as overlapping if they share the track
        // or if they are DR matched
        bool removeEle = (elTrk == muTrk);
        if( (m_useDRMatching)
            && (m_dRMatcher->objectsMatch(electron, muon)) ){
          removeEle = true;
        }
        if(removeEle){
          ATH_CHECK( handleOverlap(electron, muon) );
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  [[nodiscard]] columnar::ObjectLink<EleMuSharedTrkOverlapTool::MyTrackDef> EleMuSharedTrkOverlapTool::
  getOriginalTrackParticle(columnar::Particle1Id electron) const
  {
    auto& acc = *m_accessors;
    auto elGsfTrk = electron(acc.m_eleTrackAcc);
    if (elGsfTrk.size() == 0 || !elGsfTrk[0].has_value())
      throw std::runtime_error("Electron has no associated track");
    return elGsfTrk[0].value()(acc.m_gsfOriginalTrackAcc);
  }

} // namespace ORUtils
