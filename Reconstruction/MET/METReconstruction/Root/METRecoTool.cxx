///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METRecoTool.cxx 
// Implementation file for class METRecoTool
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METRecoTool.h"

// MET EDM
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETComponentMap.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAuxComponentMap.h"

#include <iomanip>

namespace met {

  using xAOD::MissingET;
  using xAOD::MissingETContainer;
  using xAOD::MissingETComposition;
  using xAOD::MissingETComponentMap;
  using xAOD::MissingETAuxContainer;
  using xAOD::MissingETAuxComponentMap;
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
  METRecoTool::METRecoTool(const std::string& name) : 
    AsgTool(name)
  {
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METRecoTool::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    if( m_contname.key().empty() ) {
      ATH_MSG_FATAL("Output MissingETContainer name must be provided.");
      return StatusCode::FAILURE;
    }

    if( m_mapname.key().empty() ) {
      ATH_MSG_FATAL("Output MissingETComponentMap name must be provided.");
      return StatusCode::FAILURE;
    }
    ATH_CHECK( m_contname.initialize() );
    ATH_CHECK( m_mapname.initialize() );


    ATH_MSG_INFO ("Reconstructing MET container: " << m_contname
		  << " with composition map: " << m_mapname     );

    // Do we need a flag to toggle the summation?
    // Or will we require the summation to be made 
    if( !m_metfinalname.empty() ) {
      m_doMetSum = true;
      ATH_MSG_INFO ("Will produce final MET sum \"" << m_metfinalname << "\"");
    } else {
      ATH_MSG_INFO ("Will not sum MET in this container.");
    }

    ATH_CHECK( m_metbuilders.retrieve() );
    ATH_CHECK( m_metrefiners.retrieve() );
    return StatusCode::SUCCESS;
  }

  StatusCode METRecoTool::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG ("In execute: " << name() << "...");

    // Create a MissingETContainer with its aux store
    auto metHandle= SG::makeHandle (m_contname, ctx);
    //note that the method below automatically creates the MET container and its corresponding aux store (which will be named "m_contname+Aux.")
    ATH_CHECK( metHandle.record (std::make_unique<xAOD::MissingETContainer>(),                      std::make_unique<xAOD::MissingETAuxContainer>()) );
    xAOD::MissingETContainer* metCont=metHandle.ptr();


    // Create a MissingETComponentMap with its aux store

    auto metMapHandle= SG::makeHandle (m_mapname, ctx);
    //note that the method below automatically creates the MET container and its corresponding aux store (which will be named "m_contname+Aux.")
    ATH_CHECK( metMapHandle.record (std::make_unique<xAOD::MissingETComponentMap>(),                      std::make_unique<xAOD::MissingETAuxComponentMap>()) );
    xAOD::MissingETComponentMap* metMap=metMapHandle.ptr();


    if( buildMET(metCont, metMap).isFailure() ) {
      ATH_MSG_WARNING("Failed in MissingET reconstruction");
      return StatusCode::SUCCESS;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode METRecoTool::finalize()
  {
    ATH_MSG_INFO ("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
  }


  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  StatusCode METRecoTool::buildMET(xAOD::MissingETContainer* metCont, xAOD::MissingETComponentMap* metMap) const
  {


    MissingET* metFinal = nullptr;
    if( m_doMetSum ) {
      ATH_MSG_DEBUG("Building final MET sum: " << m_metfinalname);
      metFinal = new MissingET(0.,0.,0., m_metfinalname, MissingETBase::Source::total());
    }

    // Run the MET reconstruction tools in sequence
    for(auto tool : m_metbuilders) {
      ATH_MSG_VERBOSE("Building new MET term with: " << tool->name() );
      MissingET* metTerm = new MissingET(0.,0.,0.);
      ATH_MSG_VERBOSE("Insert MET object into container");
      metCont->push_back(metTerm);
      ATH_MSG_VERBOSE("Insert MET object into ComponentMap");
      MissingETComposition::add(metMap,metTerm);
      ATH_MSG_VERBOSE("Execute tool");
      if( tool->execute(metTerm, metMap).isFailure() ) {
        ATH_MSG_WARNING("Failed to execute tool: " << tool->name());
        // return StatusCode::SUCCESS;
      }
      ///////////////////////////////////////////////////
      // FIXME: Make a genuine decision about whether
      //        to include terms in the sum here.
      ///////////////////////////////////////////////////
      if( m_doMetSum && MissingETBase::Source::hasCategory(metTerm->source(),MissingETBase::Source::Category::Refined) ) {
        ATH_MSG_DEBUG("Adding constructed term: " << metTerm->name() << " to sum" );
        (*metFinal) += (*metTerm);
      }
    }

    // Run the MET reconstruction tools in sequence
    for(auto tool : m_metrefiners) {
      ATH_MSG_VERBOSE("Refining MET with: " << tool->name() );
      MissingET* metTerm = new MissingET(0.,0.,0.);
      ATH_MSG_VERBOSE("Insert MET object into container");
      metCont->push_back(metTerm);
      ATH_MSG_VERBOSE("Insert MET object into ComponentMap");
      MissingETComposition::add(metMap,metTerm);
      ATH_MSG_VERBOSE("Execute tool");
      if( tool->execute(metTerm, metMap).isFailure() ) {
        ATH_MSG_WARNING("Failed to execute tool: " << tool->name());
        // return StatusCode::SUCCESS;
      }
    }

    if( m_doMetSum ) {
      MissingETBase::Types::bitmask_t source = MissingETBase::Source::total();
      metFinal->setSource(source);
      metCont->push_back(metFinal);
    }
    ++m_nevt;
    return StatusCode::SUCCESS;
  }

}
