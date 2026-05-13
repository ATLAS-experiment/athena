/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// xAOD include(s):
#ifdef XAOD_STANDALONE
#   include "xAODRootAccessInterfaces/TActiveEvent.h"
#   include "xAODRootAccess/Event.h"
#endif // XAOD_STANDALONE

// Local include(s):
#include "AsgTools/SgEventMeta.h"

namespace asg {

   SgEventMeta::SgEventMeta( StoreType type, xAOD::Event* event )
      : m_type( type ) {
      m_event.store(event);
   }

   SgEventMeta::SgEventMeta(SgEventMeta&& other) noexcept
      : m_type(other.m_type) {
      m_event.store(other.m_event.load());
   }

   SgEventMeta& SgEventMeta::operator=(SgEventMeta&& other) noexcept {
      if (this != &other) {
         m_type = other.m_type;
         m_event.store(other.m_event.load());
      }
      return *this;
   }

   SgEventMeta& SgEventMeta::operator=(xAOD::Event* event) {
      m_event.store(event);
      return *this;
   }

   /// This function is used by the template functions to try to retrieve
   /// a valid pointer to the active Event object, if one is not available
   /// yet.
   ///
   /// @returns <code>StatusCode::FAILURE</code> if Event can't be found,
   ///          <code>StatusCode::SUCCESS</code> otherwise
   ///
   StatusCode SgEventMeta::initialize() const {

      // Return right away if we already have a non-null pointer:
      if (m_event && m_event.load()) {
         return StatusCode::SUCCESS;
      }

      // Check if there's an active event:
      xAOD::TVirtualEvent* event = xAOD::TActiveEvent::event();
      if( ! event ) {
         std::cout << META_ERROR_SRC << "Couldn't find an active event in "
                   << "the job" << std::endl;
         return StatusCode::FAILURE;
      }

      // This should actually be a Event:
      m_event.store(dynamic_cast< xAOD::Event* >( event ));
      if( !m_event || !m_event.load() ) {
         std::cout << META_ERROR_SRC << "The active event is not of type "
                   << "xAOD::Event?!?" << std::endl;
         return StatusCode::FAILURE;
      }

      // We succeeded:
      return StatusCode::SUCCESS;
   }

} // namespace asg
