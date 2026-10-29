/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GaudiKernel/System.h"
#include "GaudiKernel/ISvcLocator.h"
#include "AthenaKernel/getMessageSvc.h"

#include "DataModelRoot/RootType.h"
#include "../src/SessionTool.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "PersistentDataModel/Token.h"

#include "PersTestBase.h"

#include <iostream>
#include <filesystem>


PersTestBase::PersTestBase( const std::string& testname, const std::string& catname ):
  AthMessaging( testname ),
  m_fileCatalogMgr( Gaudi::svcLocator()->service<Gaudi::IFileCatalogMgr>( "Gaudi::MultiFileCatalog" ) ),
  m_fileCatalog( m_fileCatalogMgr )
{
  // Suppress Athena MessageSvc warnings about not finding Gaudi MessageSvc
  Athena::getMessageSvcQuiet = true;

  ATH_MSG_INFO( "Creating FileCatalog" );
  if ( ! m_fileCatalog.isValid() ) {
    throw std::runtime_error( "Could not create a file catalog" );
  }
  std::filesystem::remove( {catname} );
  FileCatalogUtils::addCatalog( *m_fileCatalogMgr, catname, true );
}


void PersTestBase::loadLibraries( const std::vector<std::string>& libraries )
{
   ATH_MSG_INFO( "Loading shared libraries:" );
   for( const auto& libname : libraries ) {
      ATH_MSG_INFO( " - loading library " << libname );
      System::ImageHandle  libhandle;
      if( System::loadDynamicLib(libname, &libhandle) != 1 ) {
         ATH_MSG_ERROR( "Error! library loading failed" );
      }
   }
}


// Sessions created here as Tools (and managed by the ToolSvc) need to be closed by hand with closeSession() to cleanup everything
ToolHandle<SessionTool>
PersTestBase::createSession( Gaudi::IFileCatalog& catalog, const std::string& sessionName )
{
  ATH_MSG_INFO( "Creating the persistency service" );
  auto dbsession = ToolHandle<SessionTool>("SessionTool/" + sessionName, Gaudi::svcLocator()->service("ToolSvc"));
  if( !dbsession.retrieve().isSuccess() ) {
    throw std::runtime_error( "Could not create DB Session" );
  }
  dbsession->setupFC( catalog );
  return dbsession;
}

PersTestBase::~PersTestBase()
{
   ATH_MSG_INFO( "Test finished. Number of floating tokens : " << Token::numInstances() );
}

