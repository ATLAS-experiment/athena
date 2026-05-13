/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGTOOLS_SGEVENTMETA_H
#define ASGTOOLS_SGEVENTMETA_H

// Local include(s):
#include "AsgMessaging/StatusCode.h"
#include "xAODRootAccess/Event.h"

// Complain if we try using this header in Athena:
#if !defined(XAOD_STANDALONE) && !defined(__CPPCHECK__)
#   error "This header should not be used in Athena"
#endif // XAOD_STANDALONE

#include <memory>
#include <vector>

// Forward declaration(s):
namespace xAOD {
   class Event;
} // namespace xAOD

namespace asg {

   /// Wrapper class providing StoreGate-like access to metadata in ROOT
   ///
   /// In Athena we access the input and output metadata stores with two
   /// separate StoreGateSvc instances. In ROOT the access to the metadata
   /// content of the input/output files is provided using the same xAOD::Event
   /// object that we use to access the event content of these files as well.
   /// To emulate the same pattern for the dual-use tools that Athena
   /// provides, this wrapper is used.
   ///
   /// It behaves pretty much like SgEvent, but I didn't want to make that
   /// class even more complicated. Instead decided to implement the metadata
   /// access using this independent class.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 611829 $
   /// $Date: 2014-08-14 15:53:41 +0200 (Thu, 14 Aug 2014) $
   ///
   class SgEventMeta {

   public:
      /// Type of the store that this object implements
      enum StoreType {
         InputStore  = 0, ///< This store is used to access the input metadata
         OutputStore = 1  ///< This store is used to access the output metadata
      }; // enum StoreType

      /// Constructor with a type and an optional Event pointer
      SgEventMeta( StoreType type, xAOD::Event* event = 0 );

      /// Move constructor
      SgEventMeta(SgEventMeta&& other) noexcept;

      /// Move assignment operator
      SgEventMeta& operator=(SgEventMeta&& other) noexcept;

      /// Assignment operator to handle direct event pointer assignment
      SgEventMeta& operator=(xAOD::Event* event);

      // Prevent copying
      SgEventMeta(const SgEventMeta&) = delete;
      SgEventMeta& operator=(const SgEventMeta&) = delete;

      /// @name Functions providing access to the metadata payload
      /// @{

      /// Check if an object is available for constant access
      template< typename T >
      bool contains( const std::string& name );
      /// Check if an object is available for non-constant access
      template< typename T >
      bool transientContains( const std::string& name ) const;

      /// Retrieve a non-constant pointer to an object in the store
      template< typename T >
      StatusCode retrieve( T*& obj, const std::string& name );
      /// Retrieve a constant pointer to an object in the store
      template< typename T >
      StatusCode retrieve( const T*& obj, const std::string& name );

      /// Record an object/container using a smart pointer for ownership
      template< typename T >
      StatusCode record( std::unique_ptr<T> obj, const std::string& cname );

      /// Record an object into the store
      template< typename T >
      StatusCode record( T* obj, const std::string& name );

      /// provide list of all keys associated with a type.
      /// usage: store->keys< type >(vector_to_fill)
      /// @param vkeys will be filled with list of keys (may be empty)
      template< typename T >
      void keys( std::vector<std::string>& vkeys ) const;

      /// @}

   private:
      /// Function initialising the object
      StatusCode initialize() const;

      /// Type of this store
      StoreType m_type;
      /// Pointer to the xAOD::Event object in use
      mutable std::atomic<xAOD::Event*> m_event;

   }; // class SgEventMeta

} // namespace asg

// Include the template implementation:
#include "AsgTools/SgEventMeta.icc"

#endif // ASGTOOLS_SGEVENTMETA_H
