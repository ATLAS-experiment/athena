/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef POOLTESTBASE_H
#define POOLTESTBASE_H

#include <string>
#include <iostream>

#include "GaudiKernel/System.h"
#include "GaudiKernel/ISvcLocator.h"
#include "DataModelRoot/RootType.h"
#include "PoolSvc/ISessionTool.h"


/// Small helper base class for pool tests 
class PoolTestBase {
public:
  PoolTestBase() = default;
  PoolTestBase(const PoolTestBase&) = delete;
  PoolTestBase& operator=(const PoolTestBase&) = delete;
  virtual ~PoolTestBase() = default;

  /// load local dictionary libraries for the test
  static void loadLibraries( const std::vector<std::string>& libraries );
  /// create a session tool for the test with @sessionName, attach the catalog to it
  ToolHandle<ISessionTool> createSession( Gaudi::IFileCatalog& fileCatalog, const std::string& sessionName );
};


inline
void PoolTestBase::loadLibraries( const std::vector<std::string>& libraries )
{
   for( std::vector<std::string>::const_iterator iLibrary = libraries.begin();
      iLibrary != libraries.end(); ++iLibrary ) {
    const std::string& library = *iLibrary;
    std::cout << "Loading library " << library << std::endl;

    System::ImageHandle         libhandle;
    if( System::loadDynamicLib(library, &libhandle) != 1 ) {
       std::cout << "Error! library loading failed"  << std::endl;
    }
  }
}


// Sessions created here as Tools (and managed by the ToolSvc) need to be closed by hand with closeSession() to cleanup everything
inline ToolHandle<ISessionTool>
PoolTestBase::createSession( Gaudi::IFileCatalog& catalog, const std::string& sessionName )
{
  std::cout << "Creating the persistency service" << std::endl;
  auto dbsession = ToolHandle<ISessionTool>("SessionTool/" + sessionName, Gaudi::svcLocator()->service("ToolSvc"));
  if( !dbsession.retrieve().isSuccess() ) {
    throw std::runtime_error( "Could not create DB Session" );
  }
  dbsession->setupFC( catalog );
  return dbsession;
}

#endif