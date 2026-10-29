/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_TESTBASE_H
#define STORAGESVC_TESTBASE_H

#include "AthenaBaseComps/AthMessaging.h"

#include <string>
#include <vector>


/// Small helper base class for pool tests 
class TestBase : public AthMessaging {
public:
  TestBase( const std::string& testname);
  TestBase(const TestBase&) = delete;
  TestBase& operator=(const TestBase&) = delete;
  virtual ~TestBase();

  /// load local dictionary libraries for the test
  void loadLibraries( const std::vector<std::string>& libraries );
};



#include "GaudiKernel/System.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "AthenaKernel/getMessageSvc.h"

#include "PersistentDataModel/Token.h"

#include <stdexcept>


TestBase::TestBase(const std::string& testname)
  : AthMessaging(testname)
{
   SmartIF<IMessageSvc> msgSvc(Athena::getMessageSvc(Athena::Options::Eager, true));
   SmartIF<IProperty> prop(msgSvc);
   if( prop ) {
      // set the width of the component name field to 40
      prop->setProperty("Format", "% F%40W%S%7W%R%T %0W%M").ignore();
   }
   msgSvc->setOutputLevel(MSG::INFO);
   ATH_MSG_INFO("Created the test driver");
   loadLibraries( {"test_StorageTestDictionary"} );
}

TestBase::~TestBase() 
{
   ATH_MSG_INFO("Number of floating tokens : " << Token::numInstances() );
}


inline
void TestBase::loadLibraries( const std::vector<std::string>& libraries )
{
   ATH_MSG_INFO( "Loading shared libraries:" );
   for( const auto& libname : libraries ) {
      ATH_MSG_INFO( " - loading library " << libname );
      System::ImageHandle  libhandle;
      if( System::loadDynamicLib(libname, &libhandle) != 1 ) {
         throw std::runtime_error( "Error! library loading failed" );
      }
   }
}


#endif