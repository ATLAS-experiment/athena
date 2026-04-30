///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METAssociationTool.cxx 
// Implementation file for class METAssociationTool
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METAssociationTool.h"

// MET EDM
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAuxComponentMap.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterChangeSignalState.h"

#include <iomanip>

namespace met {

  using namespace xAOD;
  //
  using std::string;
  using std::setw;
  using std::setprecision;
  using std::fixed;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////
  METAssociationTool::METAssociationTool(const std::string& name) : 
    AsgTool(name)
  {
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METAssociationTool::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    if( m_metSuffix.size()==0 ) {
      ATH_MSG_ERROR("Output suffix for MET names must be provided.");
      return StatusCode::FAILURE;
    } else {
      std::string coreName = "MET_Core_" + m_metSuffix;
      std::string mapName = "METAssoc_" + m_metSuffix;
      ATH_MSG_INFO("Tool configured to build MET with names:");
      ATH_MSG_INFO("   Core container  ==> " << coreName);
      ATH_MSG_INFO("   Association map ==> " << mapName);
      ATH_CHECK( m_coreKey.assign(coreName) ); // FIXME properties should be properly assigned during the configuration
      ATH_CHECK( m_coreKey.initialize() );
      ATH_CHECK( m_mapKey.assign(mapName) ); // FIXME properties should be properly assigned during the configuration
      ATH_CHECK( m_mapKey.initialize() );
    }

    // retrieve associators
    ATH_CHECK(m_metAssociators.retrieve());    
    return StatusCode::SUCCESS;
  }

  StatusCode METAssociationTool::execute(const EventContext& ctx) const
  {
    //this section has had a very big re-write, after discussions with TJK...
    ATH_MSG_DEBUG ("In execute: " << name() << "...");

    //Create map and core containers

    auto metHandle = SG::makeHandle (m_coreKey, ctx);
    ATH_CHECK( metHandle.record (std::make_unique<xAOD::MissingETContainer>(), std::make_unique<xAOD::MissingETAuxContainer>()) );
    xAOD::MissingETContainer* metCont=metHandle.ptr();

    auto metMapHandle = SG::makeHandle (m_mapKey, ctx);
    ATH_CHECK( metMapHandle.record (std::make_unique<xAOD::MissingETAssociationMap>(), std::make_unique<xAOD::MissingETAuxAssociationMap>()) );
    xAOD::MissingETAssociationMap* metMap=metMapHandle.ptr();


    if( buildMET(metCont, metMap, ctx).isFailure() ) {
      ATH_MSG_DEBUG("Failed in MissingET reconstruction");
      return StatusCode::SUCCESS;
    }

    // Lock the containers in SG
    //ATH_CHECK( evtStore()->setConst(metMap) );
    //ATH_CHECK( evtStore()->setConst(metCont) );

    return StatusCode::SUCCESS;
  }

  StatusCode METAssociationTool::finalize()
  {
    ATH_MSG_INFO ("Finalizing " << name() << "...");

    return StatusCode::SUCCESS;
  }

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  StatusCode METAssociationTool::buildMET(xAOD::MissingETContainer* metCont, xAOD::MissingETAssociationMap* metMap, const EventContext& ctx) const
  {


    // Run the MET reconstruction tools in sequence
    for(auto tool : m_metAssociators) {
      if (tool->execute(metCont,metMap,ctx).isFailure()){
        ATH_MSG_WARNING("Failed to execute tool: " << tool->name());
        return StatusCode::FAILURE;
      }
    }
    bool foundOverlaps = metMap->identifyOverlaps();
    ATH_MSG_DEBUG( (foundOverlaps ? "Overlaps" : "No overlaps") << " identified!");
    ++m_nevt;
    return StatusCode::SUCCESS;
  }
}
