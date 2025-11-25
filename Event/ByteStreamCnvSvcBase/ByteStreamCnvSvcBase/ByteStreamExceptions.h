/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_BYTESTREAMEXCEPTIONS_H
#define BYTESTREAMCNVSVC_BYTESTREAMEXCEPTIONS_H

/** @file ByteExceptions.h
 **/
 
// EXCEPTIONS 
namespace ByteStreamExceptions 
{
   class fileAccessError
   {
      virtual const char* what() const noexcept {
         return "Problem accessing file";
      }
   };
   class readError
   {
      virtual const char* what() const noexcept {
         return "Problem during DataReader getData";
      }
   };
   class badFragment
   {
      virtual const char* what() const noexcept {
         return "Unable to build RawEvent, fragment does not match known formats.";
      }
   };
   class badFragmentData
   {
      virtual const char* what() const noexcept {
         return "RawEvent does not pass validation";
      }
   }; 
} 
#endif
