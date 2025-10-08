// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// ROOT include(s):
#include <TClass.h>
#include <TError.h>
#include <TPython.h>

// Local include(s):
#include "xAODRootAccess/TPyEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "CxxUtils/checker_macros.h"

namespace xAOD {

   PyObject* TPyEvent::retrieve (const std::string& key)
   {
     // Try to find the event format corresponding to this key.
      const EventFormat* ef = this->inputEventFormat();
      const EventFormatElement* efe = ef->get (key);
      if (!efe) {
         ef = this->outputEventFormat();
         efe = ef->get (key);
         if (!efe) {
            // Fail if we can't find the type.
            Py_INCREF (Py_None);
            return Py_None;
         }
      }

      // Convert the type to a std::type_info.
      TClass* cl = TClass::GetClass (efe->className().c_str());
      if (!cl) {
        Py_INCREF (Py_None);
        return Py_None;
      }
      const std::type_info& ti = *cl->GetTypeInfo();

      // Get the object from the store as a void*.
      void* obj ATLAS_THREAD_SAFE = const_cast<void*>(this->getInputObject (efe->hash(), ti, true));
      if (!obj) {
         obj = this->getOutputObject (efe->hash(), ti);
      }
      if (!obj) {
         Py_INCREF (Py_None);
         return Py_None;
      }

      // Convert to a PyObject.
      return TPython::CPPInstance_FromVoidPtr (obj, efe->className().c_str());
   }


   ::Bool_t TPyEvent::contains( const std::string& key,
                                const std::string& type ) {

      // Try to access the dictionary of this type:
      ::TClass* cl = ::TClass::GetClass( type.c_str() );
      if( ! cl ) {
         ::Warning( "xAOD::TPyEvent::contains",
                    "Type name \"%s\" not known", type.c_str() );
         return kFALSE;
      }

      // Check if the dictionary can return a type_info:
      const std::type_info* ti = cl->GetTypeInfo();
      if( ! ti ) {
         ::Warning( "xAOD::TPyEvent::contains",
                    "Type \"%s\" doesn't have a proper dictionary",
                    type.c_str() );
         return kFALSE;
      }

      // Let the base class do the work:
      return TEvent::contains( key, *ti, kFALSE );
   }

   ::Bool_t TPyEvent::transientContains( const std::string& key,
                                         const std::string& type ) const {

      // Try to access the dictionary of this type:
      ::TClass* cl = ::TClass::GetClass( type.c_str() );
      if( ! cl ) {
         ::Warning( "xAOD::TPyEvent::transientContains",
                    "Type name \"%s\" not known", type.c_str() );
         return kFALSE;
      }

      // Check if the dictionary can return a type_info:
      const std::type_info* ti = cl->GetTypeInfo();
      if( ! ti ) {
         ::Warning( "xAOD::TPyEvent::transientContains",
                    "Type \"%s\" doesn't have a proper dictionary",
                    type.c_str() );
         return kFALSE;
      }

      // Let the base class do the work:
      return TEvent::transientContains( key, *ti, kFALSE );
   }

   /// This function is designed to be callable from PyROOT in order to record
   /// hand-made containers into an output file.
   ///
   /// Notice that unline TEvent::record(...), this function doesn't take
   /// ownership of the object that's given to it. That's because all objects
   /// created by the Python interpreter are garbage collected by the Python
   /// interpreter. And we don't want to run into double-deletes.
   ///
   /// @param dummy Necessary to make sure that this function's signature is
   ///              distinct from @c xAOD::TEvent::record. So that PyROOT would
   ///              not mistake them for each other.
   /// @param obj  Typeless pointer to the object that is to be recorded
   /// @param key  The key with which the object is to be recorded
   /// @param type The type name of the object being recorded
   /// @param basketSize The size of the baskets used to write the payload
   /// @param splitLevel The split level of the branch created
   /// @returns The usual StatusCode types
   ///
   StatusCode TPyEvent::record( int /*dummy*/, void* obj,
                                 const std::string& key,
                                 const std::string& type,
                                 ::Int_t basketSize, ::Int_t splitLevel ) {

      // Simply forward the call to the base class:
      RETURN_CHECK( "xAOD::TPyEvent::record",
                    TEvent::record( obj, type, key, basketSize, splitLevel,
                                    kFALSE, kFALSE, kFALSE ) );
      // Return gracefully:
      return StatusCode::SUCCESS;
   }

} // namespace xAOD
