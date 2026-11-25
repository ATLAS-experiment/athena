/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/System.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "AthenaKernel/getMessageSvc.h"

#include "PersistentDataModel/Token.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/IStorageTool.h"
#include "TestBase.h"

#include <stdexcept>

TestBase::TestBase(const std::string& testname)
  : AthMessaging(testname),
    m_msgSvc(Athena::getMessageSvc(Athena::Options::Eager, true))
{
   SmartIF<IProperty> prop(m_msgSvc);
   if( prop ) {
      // set the width of the component name field to 40
      prop->setProperty("Format", "% F%40W%S%7W%R%T %0W%M").ignore();
   }
   m_msgSvc->setOutputLevel(MSG::INFO);
   ATH_MSG_INFO("Created the test driver");
   loadLibraries( {"test_StorageTestDictionary"} );
}

TestBase::~TestBase() 
{
   ATH_MSG_INFO("Number of floating tokens : " << Token::numInstances() );
}


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

ToolHandle<IStorageTool> TestBase::createStorageTool()
{
   ATH_MSG_INFO("Creating StorageTool");
   ToolHandle<IStorageTool> storageTool("DbStorageTool/StorageTool", Gaudi::svcLocator()->service("ToolSvc"));
   if( !storageTool.retrieve().isSuccess() ) {
      throw std::runtime_error( "Could not create a StorageTool" );
   }
   return storageTool;
}

void TestBase::startSession( ToolHandle<IStorageTool> storageTool, Io::IoFlag mode, DbType tech )
{
   ATH_MSG_INFO("Starting " << tech.storageName() <<  " " << accessMode(mode) << " Session");
   if( !storageTool->startSession( mode, tech.type() ).isSuccess() ) {
      throw std::runtime_error( "Could not start a session." );
   }
}

void TestBase::endSession( ToolHandle<IStorageTool> storageTool, FileDescriptor& fd )
{
   ATH_MSG_INFO("Disconnecting StorageTool");
   if( !storageTool->disconnect( fd ).isSuccess() ) {
      throw std::runtime_error( "Could not disconnect." );
   }
   endSession(storageTool);
}

void TestBase::endSession( ToolHandle<IStorageTool> storageTool )
{
   ATH_MSG_INFO("Ending Session");
   if( !storageTool->endSession().isSuccess() ) {
      throw std::runtime_error( "Could not end the session." );
   }
}