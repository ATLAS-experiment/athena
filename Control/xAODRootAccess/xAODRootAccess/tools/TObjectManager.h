// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TOBJECTMANAGER_H
#define XAODROOTACCESS_TOOLS_TOBJECTMANAGER_H

// Local include(s):
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/IObjectManager.h"

// System include(s).
#include <memory>

// Forward declaration(s):
class TBranch;

namespace xAOD {

   /// @short Manager for EDM objects created by ROOT
   ///
   /// This class is used when an EDM object is meant to be created
   /// by ROOT's schema evolution system, behind the scenes.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TObjectManager : public Details::IObjectManager {

   public:
      /// Constructor, getting hold of the created objects
      TObjectManager( ::TBranch* br = 0,
                      std::unique_ptr<THolder> holder = nullptr,
                      ::Bool_t renewOnRead = kFALSE );
      /// Copy constructor
      TObjectManager( const TObjectManager& parent );
      /// Destructor
      ~TObjectManager();

      /// Assignment operator
      TObjectManager& operator=( const TObjectManager& parent );

      /// Accessor to the branch
      ::TBranch* branch();
      /// Pointer to the branch's pointer
      ::TBranch** branchPtr();

      /// Function for updating the object in memory if needed
      virtual ::Int_t getEntry( ::Int_t getall = 0 ) override;

      /// Function getting a const pointer to the object being handled
      virtual const void* object() const override;
      /// Function getting a pointer to the object being handled
      virtual void* object() override;
      /// Function replacing the object being handled
      virtual void setObject( void* obj ) override;

      /// Create the object for the current event
      virtual ::Bool_t create() override;
      /// Check if the object was set for the current event
      virtual ::Bool_t isSet() const override;
      /// Reset the object at the end of processing of an event
      virtual void reset() override;

   private:
      /// Pointer keeping track of the branch
      ::TBranch* m_branch;
      /// The last entry that was loaded for this branch
      ::Long64_t m_entry;
      /// Was the object set for the current event?
      ::Bool_t m_isSet;
      /// Should the object be recreated on each read?
      ::Bool_t m_renewOnRead;

   }; // class TObjectManager

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TOBJECTMANAGER_H
