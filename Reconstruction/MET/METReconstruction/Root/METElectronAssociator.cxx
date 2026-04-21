///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// METElectronAssociator.cxx
// Implementation file for class METElectronAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
///////////////////////////////////////////////////////////////////

// METReconstruction includes
#include "METReconstruction/METElectronAssociator.h"

// Egamma EDM
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Photon.h"

namespace met {

  using namespace xAOD;

  // Constructors
  ////////////////
  METElectronAssociator::METElectronAssociator(const std::string& name) :
    AsgTool(name),
    METAssociator(name),
    METEgammaAssociator(name)
  { 
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METElectronAssociator::initialize()
  {
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");
    ATH_CHECK( m_elContKey.initialize());
    ATH_CHECK( METEgammaAssociator::initialize() );

    ATH_CHECK(m_electronNeutralPFOReadDecorKey.initialize(m_usePFOLinks));
    ATH_CHECK(m_electronChargedPFOReadDecorKey.initialize(m_usePFOLinks));
    ATH_CHECK(m_electronNeutralFEReadDecorKey.initialize(m_useFELinks));
    ATH_CHECK(m_electronChargedFEReadDecorKey.initialize(m_useFELinks));
    ATH_CHECK(m_photonNeutralPFOReadDecorKey.initialize(false));
    ATH_CHECK(m_photonChargedPFOReadDecorKey.initialize(false));
    ATH_CHECK(m_photonNeutralFEReadDecorKey.initialize(false));
    ATH_CHECK(m_photonChargedFEReadDecorKey.initialize(false));

    return StatusCode::SUCCESS;
  }


  // executeTool
  ////////////////
  StatusCode METElectronAssociator::executeTool(xAOD::MissingETContainer* /*metCont*/, xAOD::MissingETAssociationMap* metMap, const EventContext& ctx) const
  {
    ATH_MSG_VERBOSE ("In execute: " << name() << "...");

    SG::ReadHandle<xAOD::ElectronContainer> elCont(m_elContKey, ctx);
    if (!elCont.isValid()) {
      ATH_MSG_WARNING("Unable to retrieve input electron container " << m_elContKey.key());
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully retrieved electron collection");
    if (fillAssocMap(metMap,elCont.cptr(), ctx).isFailure()) {
      ATH_MSG_WARNING("Unable to fill map with electron container " << m_elContKey.key());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

}
