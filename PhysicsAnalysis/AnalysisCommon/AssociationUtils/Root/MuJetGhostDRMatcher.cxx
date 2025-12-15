/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Infrastructure
#include "AthContainers/AuxElement.h"
#include "AthLinks/ElementLink.h"

// EDM includes
#include "xAODBase/IParticleContainer.h"
#include "xAODMuon/Muon.h"

// Local includes
#include "AssociationUtils/MuJetGhostDRMatcher.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  MuJetGhostDRMatcher::MuJetGhostDRMatcher(double dR, bool useRapidity)
    : asg::AsgMessaging("MuJetGhostDRMatcher"),
      m_drMatcher (std::make_unique<DeltaRMatcher>(dR, useRapidity))
  {
    addSubtool (*m_drMatcher.get());
  }



  StatusCode MuJetGhostDRMatcher::setObjectTypes (xAODType::ObjectType type1,
                                            xAODType::ObjectType type2)
  {
    if (type1 != xAOD::Type::Muon) {
      ATH_MSG_ERROR("First particle arg to setObjectTypes is not a muon!");
      return StatusCode::FAILURE;
    }
    if (type2 != xAOD::Type::Jet) {
      ATH_MSG_ERROR("Second particle arg to setObjectTypes is not a jet!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK (m_drMatcher->setObjectTypes(type1, type2));
    addSubtool (*m_drMatcher.get());
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Check for a match via ghost association or delta-R
  //---------------------------------------------------------------------------
  bool MuJetGhostDRMatcher::objectsMatch(columnar::Particle1Id mu,
                                         columnar::Particle2Id jet,
                                         bool swapArgs) const
  {
    if (swapArgs) {
      ATH_MSG_WARNING("MuJetGhostDRMatcher does not support swapped args");
      return false;
    }

    // Check the particle types. First particle should be the muon,
    // and the second particle should be the jet.
    if constexpr (columnar::ColumnarModeDefault::isXAOD)
    {
      if(mu.getXAODObject().type() != xAOD::Type::Muon) {
        ATH_MSG_WARNING("First particle arg to objectsMatch is not a muon!");
        return false;
      }
      if(jet.getXAODObject().type() != xAOD::Type::Jet) {
        ATH_MSG_WARNING("Second particle arg to objectsMatch is not a jet!");
        return false;
      }
    }

    // Try the delta-R match first.
    if(m_drMatcher->objectsMatch(mu, jet)) {
      ATH_MSG_DEBUG("  Found a dR association");
      return true;
    }

    // Retrieve the muon's ID track, or bail if none available.
    auto muTrk = mu(m_muonTrkAcc).opt_value();
    if(!muTrk) return false;

    // Search for the muon ID track in the list of ghosts.
    for(const auto ghostLink : m_ghostAcc(jet)) {
      if(ghostLink.has_value() && muTrk == ghostLink) {
        ATH_MSG_DEBUG("  Found a ghost association!");
        return true;
      }
    }

    return false;
  }

} // namespace ORUtils
