/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METRecoAlg.cxx

#include "METRecoAlg.h"
#include "METRecoInterface/IMETRecoTool.h"

using std::string;

namespace met {

  //**********************************************************************

  StatusCode METRecoAlg::initialize() {
    ATH_MSG_VERBOSE("Initializing " << name() << "...");

    ATH_CHECK( m_recotools.retrieve() );

    return StatusCode::SUCCESS;
  }

  //**********************************************************************

  StatusCode METRecoAlg::finalize() {
    ATH_MSG_VERBOSE ("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
  }

  //**********************************************************************

  StatusCode METRecoAlg::execute(const EventContext& ctx) const{
    ATH_MSG_VERBOSE("Executing " << name() << "...");
    // Loop over tools.

    // Run the top-level MET tools in sequence
    for(auto tool : m_recotools) {
      ATH_MSG_VERBOSE("Running tool: " << tool->name() );
      if( tool->execute(ctx).isFailure() ) {
        ATH_MSG_ERROR("Failed to execute tool: " << tool->name());
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

  //**********************************************************************

}
