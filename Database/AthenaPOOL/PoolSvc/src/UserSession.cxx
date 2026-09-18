/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "UserSession.h"
#include "DatabaseRegistry.h"
#include "UserDatabase.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"


std::unique_ptr< pool::ISession >
pool::createSession( IFileCatalog& catalog , int ageLimit )
{
   return std::unique_ptr<ISession>(  new pool::UserSession(catalog, ageLimit) );
}


pool::UserSession::UserSession( pool::IFileCatalog& fileCatalog, int ageLimit ):
  APRMessaging( "APR Session" ),
  m_catalog( &fileCatalog ),
  m_ageLimit( ageLimit ),
  m_registry( 0 ),
  m_transactionType( Io::INVALID )
{
  m_registry = new pool::DatabaseRegistry();
}

pool::UserSession::~UserSession()
{
  // order is important
  m_technologies.clear();
  delete m_registry;
}


void*
pool::UserSession::readObject( const Token& token, void* object )
{
  void* result {};
  if( isActive() ) {
    UserDatabase db( *this, token.dbID().toString(), pool::DatabaseSpecification::FID );
    if ( db.openMode() == Io::INVALID ) {
      db.setTechnology( token.technology() );
      db.connectForRead();
    }
    result = db.readObject( token, object );
  }
  return result;
}

Token*
pool::UserSession::registerForWrite( const Placement& place,
                                                     const void* object,
                                                     const RootType& type )
{
  if( m_transactionType != Io::WRITE && m_transactionType != Io::APPEND ) {
    return 0;
  }
  UserDatabase db( *this, place.fileName(), pool::DatabaseSpecification::PFN );
  if ( db.openMode() == Io::INVALID ) {
    db.setTechnology( place.technology() );
    db.connectForWrite();
  }
  return db.writeObject( place.containerName(), place.technology(), object, type );
}


pool::DatabaseRegistry&
pool::UserSession::registry()
{
  return *m_registry;
}


bool
pool::UserSession::disconnectAll()
{
  bool ret = true;
  for( auto& iManager : m_technologies ) {
    if( !iManager.second->disconnectAll() ) ret = false;
  }
  return ret;
}
      

bool
pool::UserSession::start( Io::IoFlag type )
{
  if( isActive() ) return false;
  m_transactionType = type;
  return true;
}


bool
pool::UserSession::commit()
{
  if( isActive() ) {
    bool OK = true;
    for( auto db : *m_registry ) {
      bool bCommit = db->commitTransaction(); // This has to be replaced with a two phase commit
      if ( ! bCommit ) {
        ATH_MSG_ERROR("Could not commit the transaction for the database with:" << endmsg
                      << "FID = " << db->fid() << endmsg   << "PFN = " << db->pfn() );
      }
      OK = OK && bCommit;
    }
    m_transactionType = Io::INVALID;
    return OK;
  }
  return false;
}


bool
pool::UserSession::commitAndHold()
{
  if( isActive() ) {
    bool OK = true;
    for( auto db : *m_registry ) {
      bool bCommit = db->commitAndHoldTransaction(); // This has to be replaced with a two phase commit
      if ( ! bCommit ) {
        ATH_MSG_ERROR("Could not commit and hold the transaction for the database with:" << endmsg
            << "FID = " << db->fid() << endmsg  << "PFN = " << db->pfn() );
      }
      OK = OK && bCommit;
    }
    return OK;
  }
  return false;
}



std::unique_ptr<pool::IDatabase>
pool::UserSession::databaseHandle( const std::string& dbName,
                                                   DatabaseSpecification::NameType dbNameType )
{
  if( isActive() ) {
     return std::make_unique<UserDatabase>( *this, dbName, dbNameType );
  }
  return nullptr;
}

pool::IFileCatalog&
pool::UserSession::fileCatalog()
{
  return *m_catalog;
}

void
pool::UserSession::setFileCatalog(pool::IFileCatalog& catalog)
{
  m_catalog = &catalog;
}


pool::MicroSessionManager&
pool::UserSession::microSessionManager( long technology )
{
  pool::DbType dbType( technology );
  long majorType = dbType.majorType();
  auto iManager = m_technologies.find( majorType );
  if ( iManager != m_technologies.end() ) {
    return *(iManager->second);
  }
  // Technology does not exist. Create the new session.
  auto mgr = new pool::MicroSessionManager( *m_registry, majorType );
  m_technologies.insert( std::make_pair( majorType, mgr ) );
  mgr->connect( m_transactionType, m_ageLimit );
  return *mgr;
}

pool::ITechnologySpecificAttributes&
pool::UserSession::technologySpecificAttributes( long technology )
{
  pool::MicroSessionManager& mgr = microSessionManager( technology );
  mgr.connect( m_transactionType, m_ageLimit );
  return mgr;
}
