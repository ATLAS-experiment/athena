// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H
#define XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H

// Local include(s):
#include "TVirtualManager.h"

// ROOT include(s):
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>

// Make the RNTuple types available in the ROOT namespace
// with all versions of ROOT.
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 1)
namespace ROOT {
using Experimental::RNTupleReader;
using Experimental::RNTupleView;
}  // namespace ROOT
#endif  // ROOT_VERSION_CODE < ROOT_VERSION(6, 36, 0)

namespace xAOD {

   // Forward declaration(s):
   class THolder;

   /// @brief  temporary namespace during development phase
   namespace Experimental {

   ///
   /// @short Manager for EDM objects created by ROOT
   ///
   /// This class is used when an EDM object is meant to be created
   /// by ROOT's schema evolution system, behind the scenes.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   /// @author RD Schaffer <R.D.Schaffer@cern.ch>
   ///
   class RObjectManager : public xAOD::TVirtualManager {

   public:
      /// Constructor 
      RObjectManager( ROOT::RNTupleView<void> field, ::Long64_t& entry, THolder* holder = 0, ::Bool_t renewOnRead = kFALSE );

      /// Do not allow copy-constructing this object:
      RObjectManager( const RObjectManager& parent ) = delete;
      /// Destructor
      ~RObjectManager();

      /// Do not allow copying this object
      RObjectManager& operator=( const RObjectManager& parent ) = delete;

      /// provide the field name
      const std::string& fieldName() const;

      /// Accessor to the Holder object
      const THolder* holder() const;
      /// Accessor to the Holder object
      THolder* holder();

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
      /// The typeless object taking care of reading the field
      ROOT::RNTupleView<void>  m_field;
      /// Holder object for the EDM object
      std::unique_ptr<THolder> m_holder;
      /// Entry number to load next
      std::reference_wrapper<::Long64_t> m_entryToLoad;
      /// The last entry that was loaded for this field
      ::Long64_t m_entry;
      /// Was the object set for the current event?
      ::Bool_t   m_isSet;
      /// Should the object be recreated on each read?
      ::Bool_t   m_renewOnRead;

   }; // class RObjectManager

   } // namespace Experimental

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H
