/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGTOOLS_ASGTOOL_H
#define ASGTOOLS_ASGTOOL_H

// Athena include(s).
#include "SGCore/sgkey_t.h"

// Local include(s):
#include "AsgTools/IAsgTool.h"
#include "AsgMessaging/MsgLevel.h"
#include "AsgMessaging/MessageCheck.h"

// Environment specific include(s):
#ifdef XAOD_STANDALONE
#   include "AsgMessaging/AsgMessaging.h"
#   include "AsgTools/AsgComponent.h"
#   include "AsgTools/SgTEvent.h"
   // Forward declaration(s):
#else // XAOD_STANDALONE
#   include "AthenaBaseComps/AthAlgTool.h"
#endif // XAOD_STANDALONE

// System include(s).
#include <string>

namespace asg {

   // Declare the type name of AsgTool's base class
#ifndef XAOD_STANDALONE
   typedef ::AthAlgTool AsgToolBase;
#else // not XAOD_STANDALONE
   typedef AsgComponent AsgToolBase;
#endif // not XAOD_STANDALONE

   /// Base class for the dual-use tool implementation classes
   ///
   /// This class can be used like AthAlgTool can be used for Athena-only
   /// tools.
   ///
   /// @author David Adams <dladams@bnl.gov>
   ///
   ///
   class AsgTool : public virtual IAsgTool,
                   public AsgToolBase {

   public:
      /// Constructor specifying the tool instance's name
      AsgTool( const std::string& name );
      /// Destructor
      ~AsgTool();

      AsgTool (const AsgTool&) = delete;
      AsgTool& operator= (const AsgTool&) = delete;


#ifdef XAOD_STANDALONE

      /// Stand-alone, StoreGate-like accessor to the event store
      SgTEvent* evtStore() const;

     // this is just so that my template functions can find this
     // method in the base class.
   public:
     using AsgToolBase::msg;

#endif // XAOD_STANDALONE

#ifndef XAOD_STANDALONE
   public:
      /// Pull in the usage of the base class's getProperty function
      using ::AthAlgTool::getProperty;

#endif // not XAOD_STANDALONE

      /// @name Additional helper functions, not directly mimicking Athena
      /// @{

      /// Get one of the tool's properties
      template< class T >
      const T* getProperty( const std::string& name ) const;

       /// A deprecated function for getting the message level's name
      const std::string& msg_level_name() const __attribute__ ((deprecated));

      /// Get the name of an object that is / should be in the event store
      ///
      /// This is a bit of a special one. @c StoreGateSvc and @c xAOD::TEvent
      /// both provide ways for getting the @c std::string name for an object
      /// that is in the store, based on a bare pointer. But they provide
      /// different interfaces for doing so.
      ///
      /// In order to allow tools to efficiently perform this operation, they
      /// can use this helper function.
      ///
      /// @see asg::AsgTool::getKey
      ///
      /// @param ptr The bare pointer to the object that the event store should
      ///            know about
      /// @return The string name of the object in the store. If not found, an
      ///         empty string.
      ///
      const std::string& getName( const void* ptr ) const;

      /// Get the (hashed) key of an object that is in the event store
      ///
      /// This is a bit of a special one. @c StoreGateSvc and @c xAOD::TEvent
      /// both provide ways for getting the @c SG::sgkey_t key for an object
      /// that is in the store, based on a bare pointer. But they provide
      /// different interfaces for doing so.
      ///
      /// In order to allow tools to efficiently perform this operation, they
      /// can use this helper function.
      ///
      /// @see asg::AsgTool::getName
      ///
      /// @param ptr The bare pointer to the object that the event store should
      ///            know about
      /// @return The hashed key of the object in the store. If not found, an
      ///         invalid (zero) key.
      ///
      SG::sgkey_t getKey( const void* ptr ) const;

      /// @}

      /// Dummy implementation of the initialisation function
      ///
      /// It's here to allow the dual-use tools to skip defining an
      /// initialisation function. Since many are doing so...
      ///
      virtual StatusCode initialize() { return StatusCode::SUCCESS; }

      /// Print the state of the tool
      virtual void print() const;

   private:
#ifdef XAOD_STANDALONE
      mutable SgTEvent m_event; ///< Wrapper around TEvent/TStore
#endif // XAOD_STANDALONE

   }; // class AsgTool

} // namespace asg

// Include the implementation of the template functions:
#include "AsgTools/AsgTool.icc"

// Include static methods for working with AsgTools
#include "AsgTools/SetProperty.h"

#endif // ASGTOOLS_ASGTOOL_H
