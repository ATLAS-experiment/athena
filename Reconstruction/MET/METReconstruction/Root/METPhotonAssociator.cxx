///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// METPhotonAssociator.cxx
// Implementation file for class METPhotonAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
///////////////////////////////////////////////////////////////////

// METReconstruction includes
#include "METReconstruction/METPhotonAssociator.h"

// Egamma EDM
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

namespace met {

  using namespace xAOD;

  // Constructors
  ////////////////
  METPhotonAssociator::METPhotonAssociator(const std::string& name) :
    AsgTool(name),
    METAssociator(name),
    METEgammaAssociator(name)
  {
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METPhotonAssociator::initialize()
  {
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");
    ATH_CHECK( m_phContKey.initialize());
    ATH_CHECK( METEgammaAssociator::initialize() );

    ATH_CHECK(m_photonNeutralPFOReadDecorKey.initialize(m_usePFOLinks));
    ATH_CHECK(m_photonChargedPFOReadDecorKey.initialize(m_usePFOLinks));
    ATH_CHECK(m_photonNeutralFEReadDecorKey.initialize(m_useFELinks));
    ATH_CHECK(m_photonChargedFEReadDecorKey.initialize(m_useFELinks));
    ATH_CHECK(m_electronNeutralPFOReadDecorKey.initialize(false));
    ATH_CHECK(m_electronChargedPFOReadDecorKey.initialize(false));
    ATH_CHECK(m_electronNeutralFEReadDecorKey.initialize(false));
    ATH_CHECK(m_electronChargedFEReadDecorKey.initialize(false));

    return StatusCode::SUCCESS;
  }


  // executeTool
  ////////////////
  StatusCode METPhotonAssociator::executeTool(xAOD::MissingETContainer* /*metCont*/, xAOD::MissingETAssociationMap* metMap, const EventContext& ctx) const
  {
    ATH_MSG_VERBOSE ("In execute: " << name() << "...");

    SG::ReadHandle<xAOD::PhotonContainer> phCont(m_phContKey, ctx);
    if (!phCont.isValid()) {
      ATH_MSG_WARNING("Unable to retrieve input photon container " << m_phContKey.key());
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully retrieved photon collection");

    if (fillAssocMap(metMap,phCont.cptr(), ctx).isFailure()) {
      ATH_MSG_WARNING("Unable to fill map with photon container " << m_phContKey.key());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

}
