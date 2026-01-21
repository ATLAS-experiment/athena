/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <stdexcept>

#include "PersistencySvc/SimpleUtilityBase.h"

#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DatabaseConnection.h"
#include "StorageSvc/pool.h"

#include "TError.h"

using namespace pool;


SimpleUtilityBase::SimpleUtilityBase( int argc, char* argv[] ):
      technologyName( pool::ROOT_StorageType.storageName() )
{
   if( argc > 0 && argv[0] )
      executableName = argv[0];
   for ( int i = 1; i < argc; ++i )
      args.push_back( std::string( argv[i] ) );
}


SimpleUtilityBase::~SimpleUtilityBase()
{
   if( storageSvc ) {
      if( session ) storageSvc->endSession( session ).ignore();
      storageSvc->release();
   }
}


bool SimpleUtilityBase::parseArguments()
{
   if( fileNames.empty() )
      return false;
   pool::DbType theType = pool::DbType::getType( technologyName );
   if ( theType.match( pool::TEST_StorageType ) ) {
      throw std::runtime_error( "Unsupported file type : " + technologyName );
   }
   return true;
}


void SimpleUtilityBase::startSession ATLAS_NOT_THREAD_SAFE ()
{
   storageSvc = pool::createStorageSvc("StorageSvc");
   if( !storageSvc ) {
      throw std::runtime_error( "Could not create a StorageSvc object" );
   }
   long technologyId = pool::DbType::getType( technologyName ).majorType();
   if( ! storageSvc->startSession( pool::READ, technologyId, session ).isSuccess() ) {
      throw std::runtime_error( "Could not start a new session" );
   }
   if( technologyId == pool::ROOT_StorageType.majorType() ) {
      // Disable warnings about unknown classes when opening the file
      gErrorIgnoreLevel = kError;
   }
}


std::string SimpleUtilityBase::readFileGUID( const std::string& pfn )
{
   std::string fid;
   pool::FileDescriptor fd( fid, pfn );
   if( ! storageSvc->connect( session, pool::READ, fd ).isSuccess() ) {
      throw std::runtime_error( "Could not open file \"" + pfn + "\"" );
   }
   pool::DatabaseConnection* connection = fd.dbc();
   DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
   StatusCode sc = dbH.param( "FID", fid );
   if( !storageSvc->disconnect( fd ).isSuccess() or !sc.isSuccess() ) {
      throw std::runtime_error( "Could not retrieve the FID from file \"" + pfn + "\"" );
   }
   return fid;
}


void SimpleUtilityBase::readFileGUIDs()
{
   for( const auto& pfn : fileNames ) {
      std::string fid = readFileGUID( pfn );
      fidAndPfn.push_back( std::make_pair( fid, pfn ) );
   }
}


int SimpleUtilityBase::run()
{
   try {
      if( parseArguments() ) {
         execute();
      }
      else printSyntax();
   }
   catch ( std::exception& error ) {
      std::cerr << executableName << ": " << error.what() << std::endl;
      return 1;
   }
   catch ( ... ) {
      std::cerr << executableName << ": Unrecognized exception!" << std::endl;
      return 1;
   }
   return 0;
}


//  ------------------   Utils -----------------------
std::string Utils::readFileGUID ATLAS_NOT_THREAD_SAFE ( const std::string& pfn )
{
   SimpleUtilityBase util;
   util.startSession();
   return util.readFileGUID( pfn );
}
