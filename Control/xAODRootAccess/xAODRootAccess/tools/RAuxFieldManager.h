// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_RAUXFIELDMANAGER_H
#define XAODROOTACCESS_TOOLS_RAUXFIELDMANAGER_H

// EDM include(s):
#include "AthContainersInterfaces/AuxTypes.h"

// Local include(s):
#include "xAODRootAccess/tools/IObjectManager.h"

// Forward declaration(s):
namespace SG {
   class IAuxTypeVector;
}

namespace xAOD::Experimental {

/// @short Manager for auxiliary branches created dynamically
///
/// This manager class is meant to deal with "simple"
/// auxiliary branches in the xAOD files.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
/// @author RD Schaffer <R.D.Schaffer@cern.ch>
///
///
class RAuxFieldManager : public Details::IObjectManager {

public:
   /// Constructor getting hold of a possible branch
   RAuxFieldManager( std::unique_ptr<THolder> holder, bool isPrimitive = false );

   /// Destructor
   ~RAuxFieldManager();

   /// Accessor to whether the field is a primitive type, e.g. int, float, bool
   ::Bool_t isPrimitive() const;

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

   /// Was the object set for the current event?
   ::Bool_t m_isSet;
   /// Is field of primitive type, e.g. int, float, bool
   ::Bool_t m_isPrimitive;

}; // class RAuxFieldManager
}  // namespace xAOD::Experimental 

#endif // XAODROOTACCESS_TOOLS_RAUXFIELDMANAGER_H
