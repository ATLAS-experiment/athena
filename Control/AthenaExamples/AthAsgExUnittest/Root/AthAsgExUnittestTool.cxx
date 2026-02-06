
//
//  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// AthAsgExUnittest includes
#include "AthAsgExUnittestTool.h"

AthAsgExUnittestTool::AthAsgExUnittestTool( const std::string& name ) : asg::AsgTool( name ) {
}

StatusCode AthAsgExUnittestTool::initialize() {
  ATH_MSG_INFO( "Initializing " << name() << "..." );
  ATH_MSG_INFO( "Property = " << m_nProperty );
  //
  //Make use of the property values to configure the tool
  //Tools should be designed so that no method other than setProperty is called before initialize
  //
  return StatusCode::SUCCESS;
}

double AthAsgExUnittestTool::useTheProperty() {
  return m_nProperty;
}
