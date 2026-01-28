// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_ROUTOBJMANAGER_H
#define XAODROOTACCESS_TOOLS_ROUTOBJMANAGER_H

// Local include(s):
#include "xAODRootAccess/tools/IObjectManager.h"

// Forward declaration(s):

namespace xAOD::Experimental {


///
/// @short Manager for EDM objects to be written to RNTuple
///
/// This class is used when an EDM object is meant to be created
/// by ROOT's schema evolution system, behind the scenes.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
/// @author RD Schaffer <R.D.Schaffer@cern.ch>
///
class ROutObjManager : public Details::IObjectManager {

public:
   /// Constructor, getting hold of the created objects
   ROutObjManager( std::string_view key, std::unique_ptr<THolder> holder );

   /// Destructor
   ~ROutObjManager();

   // provide field name
   const std::string& key();

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
   /// The name of the (SG) key
   std::string  m_key;
   /// Was the object set for the current event?
   ::Bool_t     m_isSet;
   /// Should the object be recreated on each read?
   ::Bool_t     m_renewOnRead;

}; // class ROutObjManager

} // namespace xAOD::Experimental

#endif // XAODROOTACCESS_TOOLS_ROUTOBJMANAGER_H
