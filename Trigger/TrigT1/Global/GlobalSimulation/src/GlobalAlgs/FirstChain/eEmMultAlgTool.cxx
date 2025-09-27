/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmMultAlgTool.h"

namespace GlobalSim {

  eEmMultAlgTool::eEmMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmMultAlgTool::initialize() {

    return StatusCode::SUCCESS;
  }

  
  // Main functional block running for each event
  StatusCode eEmMultAlgTool::run(const EventContext& ) const {
    return StatusCode::SUCCESS;
  }
  

}
