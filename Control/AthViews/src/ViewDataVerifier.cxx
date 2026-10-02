/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ViewDataVerifier.cxx 
// Implementation file for class ViewDataVerifier
// Author: B. Wynne <bwynne@cern.ch>
/////////////////////////////////////////////////////////////////// 

#include "ViewDataVerifier.h"

// STL includes

// FrameWork includes
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthViews/View.h"
#include <print>

namespace AthViews {

ViewDataVerifier::ViewDataVerifier( const std::string& name,
                      ISvcLocator* pSvcLocator ) : 
  ::AthReentrantAlgorithm( name, pSvcLocator )
{
}

ViewDataVerifier::~ViewDataVerifier()
{}

StatusCode ViewDataVerifier::initialize()
{
  StatusCode sc = StatusCode::SUCCESS;

  // Debug message indicating what containers will be loaded
  // Code copied from SGInputLoader
  if ( m_load.value().size() > 0 ) {
    std::ostringstream str;
    std::print (str, "Will explicitly preload the following DataObjects:");;
    for ( auto &obj : m_load.value() ) {
      str << "\n    + " << obj;
      if ( obj.key() .empty() ) {
        sc = StatusCode::FAILURE;
        std::print (str, "   ERROR: empty key is not allowed!");
      }
    }
    ATH_MSG_INFO( str.str() );
  }

  // Inform the scheduler that these containers will be available
  if ( !setProperty( "ExtraOutputs", m_load.toString() ).isSuccess() ) {
    ATH_MSG_WARNING( "Failed setting property ExtraOutputs" );
  }

  return sc;
}

StatusCode ViewDataVerifier::execute(const EventContext& ctx) const
{  
  // Don't actually test for data presence in normal running
  // The VDV will just tell the scheduler that the data is available
  // Downstream algs can then fail if this is incorrect
  if ( !this->msgLvl( MSG::DEBUG ) ) [[likely]] {
    return StatusCode::SUCCESS;
  }

  // Retrieve the current view from the EventContext
  auto viewProxy = Atlas::getExtendedEventContext(ctx).proxy();

  ATH_MSG_DEBUG( "Executing {} running with store {}",
                 name(), viewProxy->name() );
  StatusCode sc = StatusCode::SUCCESS;

  // Test each container
  for ( auto &obj : m_load.value() ) {

    // Create a VarHandleKey (code copied from SGInputLoader)
    SG::VarHandleKey vhk( obj.clid(), obj.key(), Gaudi::DataHandle::Writer );

    // Create a test proxy 
    SG::DataProxy* dp = viewProxy->proxy( obj.clid(), vhk.key() );

    // Test if the proxy is valid
    if ( dp )
    {
      ATH_MSG_DEBUG( "Found {} in {}", obj.key(), viewProxy->name() );
    }
    else if ( obj.key().rfind( "DetectorStore", 0 ) != std::string::npos )
    {
      ATH_MSG_DEBUG( "Ignoring DetectorStore data {}", obj.key() );
    }
    else if ( obj.className() == "SG::AuxElement" )
    {
      // For decorations, look for the parent container
      const auto split = obj.key().rfind('.');
      if (split == std::string::npos) {
        ATH_MSG_ERROR( "Aux data specified, but key does not contain '.': {}", obj.key() );
        sc = StatusCode::FAILURE;
      }

      // Is parent container present?
      const auto parentKey = obj.key().substr( 0, split );
      SG::VarHandleKey parentVhk( obj.clid(), parentKey, Gaudi::DataHandle::Writer );
      dp = viewProxy->proxy( obj.clid(), parentVhk.key() );
      if ( dp ) {
        ATH_MSG_DEBUG( "Found {} in {} (need aux data {})",
                       parentKey, viewProxy->name(), obj.key() );
      } else {
        ATH_MSG_ERROR( "Did not find {} in {} (need aux data {})",
                       parentKey, viewProxy->name(), obj.key() );
        sc = StatusCode::FAILURE;
      }
    }
    else
    {
      // Examine the current view
      ATH_MSG_ERROR( "Did not find {} in {}", obj.key(), viewProxy->name() );
      const SG::View* view = dynamic_cast<const SG::View*>( viewProxy );
      if ( view != 0 ) {
        ATH_MSG_DEBUG( "Available content is: {}", view->dump() );
      } else {
        ATH_MSG_DEBUG( "Not a View" );
      }
      sc = StatusCode::FAILURE;
    }
  }

  return sc;
}

} //> end namespace AthViews
