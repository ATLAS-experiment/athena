/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
      virtual const char* what() const throw() {
         return "Problem accessing file";
      }
   };  
   class readError
   { 
      virtual const char* what() const throw() {
         return "Problem during DataReader getData";
      }
   }; 
   class badFragment
   {
      virtual const char* what() const throw() {
         return "Unable to build RawEvent, fragment does not match known formats.";
      }
   }; 
   class badFragmentData
   {
      virtual const char* what() const throw() {
         return "RawEvent does not pass validation";
      }
   }; 
} 
#endif
