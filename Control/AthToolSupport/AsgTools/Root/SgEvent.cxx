/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "AsgTools/SgEvent.h"

// RootCore include(s):
#ifdef XAOD_STANDALONE
#   include "xAODRootAccessInterfaces/TActiveEvent.h"
#   include "xAODRootAccess/TActiveStore.h"
#endif // XAOD_STANDALONE

namespace asg {

   SgEvent::SgEvent( xAOD::Event* pevm, xAOD::TStore* ptds )
      : m_pevm( pevm ), m_ptds( ptds ) {

   }

   xAOD::Event* SgEvent::event() const {

      if( ! m_pevm ) {
         initialize().ignore();
      }

      return m_pevm;
   }

   xAOD::TStore* SgEvent::tds() const {

      // I'm checking the value of m_pevm on purpose. Since m_ptds may be
      // missing under normal circumstances as well.
      if( ! m_pevm ) {
         initialize().ignore();
      }

      return m_ptds;
   }

   StatusCode SgEvent::initialize() const {

      // Return right away if we already have a pointer to both stores:
      if( m_pevm && m_ptds ) {
         return StatusCode::SUCCESS;
      }

      // Look for a pointer to the active event if necessary:
      if( ! m_pevm ) {
         // Check if there's an active event:
         xAOD::TVirtualEvent* event = xAOD::TActiveEvent::event();
         if( ! event ) {
            std::cout << ERROR_SRC << "Couldn't find an active event in "
                      << "the job" << std::endl;
            return StatusCode::FAILURE;
         }

         // This should actually be a Event:
         m_pevm = dynamic_cast< xAOD::Event* >( event );
         if( ! m_pevm ) {
            std::cout << ERROR_SRC << "The active event is not of type "
                      << "xAOD::Event?!?" << std::endl;
            return StatusCode::FAILURE;
         }
      }

      // Look for a pointer to the active store if necessary:
      if( ! m_ptds ) {
         m_ptds = xAOD::TActiveStore::store();
         if( ! m_ptds ) {
            std::cout << "asg::SgEvent              WARNING "
                      << "No xAOD::TStore object is available" << std::endl;
         }
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

} // namespace asg
