/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "UserSession.h"
#include "PersistencySvc/DatabaseConnectionPolicy.h"
#include "DatabaseRegistry.h"
#include "UserDatabase.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"


std::unique_ptr< pool::PersistencySvc::ISession >
pool::PersistencySvc::createSession( IFileCatalog& catalog )
{
   return std::unique_ptr<ISession>(  new pool::PersistencySvc::UserSession(catalog) );
}


pool::PersistencySvc::UserSession::UserSession( pool::IFileCatalog& fileCatalog ):
  APRMessaging( "APR Session" ),
  m_policy( 0 ),
  m_catalog( &fileCatalog ),
  m_registry( 0 ),
  m_transactionType( pool::ITransaction::INACTIVE )
{
  m_policy = new pool::DatabaseConnectionPolicy;
  m_registry = new pool::PersistencySvc::DatabaseRegistry();
}

pool::PersistencySvc::UserSession::~UserSession()
{
  // order is important
  m_technologies.clear();
  delete m_registry;
  delete m_policy;
}


void*
pool::PersistencySvc::UserSession::readObject( const Token& token, void* object )
{
  void* result {};
  if( isActive() ) {
    UserDatabase db( *this, token.dbID().toString(), pool::DatabaseSpecification::FID );
    if ( db.openMode() == pool::IDatabase::CLOSED ) {
      db.setTechnology( token.technology() );
      db.connectForRead();
    }
    result = db.databaseHandler().readObject( token, object );
  }
  return result;
}

Token*
pool::PersistencySvc::UserSession::registerForWrite( const Placement& place,
                                                     const void* object,
                                                     const RootType& type )
{
  if( m_transactionType != pool::ITransaction::UPDATE ) {
    return 0;
  }
  UserDatabase db( *this, place.fileName(), pool::DatabaseSpecification::PFN );
  if ( db.openMode() == pool::IDatabase::CLOSED ) {
    db.setTechnology( place.technology() );
    db.connectForWrite();
  }
  return db.databaseHandler().writeObject( place.containerName(),
                                           place.technology(),
                                           object,
                                           type );
}


pool::PersistencySvc::DatabaseRegistry&
pool::PersistencySvc::UserSession::registry()
{
  return *m_registry;
}

void
pool::PersistencySvc::UserSession::setDefaultConnectionPolicy( const pool::DatabaseConnectionPolicy& policy )
{
  *m_policy = policy;
}

const pool::DatabaseConnectionPolicy&
pool::PersistencySvc::UserSession::defaultConnectionPolicy() const
{
  return *m_policy;
}

bool
pool::PersistencySvc::UserSession::disconnectAll()
{
  bool ret = true;
  for( auto& iManager : m_technologies ) {
    if( !iManager.second->disconnectAll() ) ret = false;
  }
  return ret;
}
      

bool
pool::PersistencySvc::UserSession::start( pool::ITransaction::Type type )
{
  if( isActive() || type == pool::ITransaction::INACTIVE ) return false;
  m_transactionType = type;
  return true;
}


bool
pool::PersistencySvc::UserSession::commit()
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
    m_transactionType = INACTIVE;
    return OK;
  }
  return false;
}


bool
pool::PersistencySvc::UserSession::commitAndHold()
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
pool::PersistencySvc::UserSession::databaseHandle( const std::string& dbName,
                                                   DatabaseSpecification::NameType dbNameType )
{
  if( isActive() ) {
     return std::make_unique<UserDatabase>( *this, dbName, dbNameType );
  }
  return nullptr;
}

pool::IFileCatalog&
pool::PersistencySvc::UserSession::fileCatalog()
{
  return *m_catalog;
}

void
pool::PersistencySvc::UserSession::setFileCatalog(pool::IFileCatalog& catalog)
{
  m_catalog = &catalog;
}


pool::PersistencySvc::MicroSessionManager&
pool::PersistencySvc::UserSession::microSessionManager( long technology )
{
  pool::DbType dbType( technology );
  long majorType = dbType.majorType();
  auto iManager = m_technologies.find( majorType );
  if ( iManager != m_technologies.end() ) {
    return *(iManager->second);
  }
  // Technology does not exist. Create the new session.
  auto mgr = new pool::PersistencySvc::MicroSessionManager( *m_registry, majorType );
  m_technologies.insert( std::make_pair( majorType, mgr ) );
  return *mgr;
}


const pool::ITechnologySpecificAttributes&
pool::PersistencySvc::UserSession::technologySpecificAttributes( long technology ) const
{
  auto iManager = m_technologies.find( pool::DbType( technology ).majorType() );
  if( iManager == m_technologies.end() ) {
    throw std::runtime_error( "Technology not found, reading APR attributes " );
  }
  return *(iManager->second);
}

pool::ITechnologySpecificAttributes&
pool::PersistencySvc::UserSession::technologySpecificAttributes( long technology )
{
  pool::PersistencySvc::MicroSessionManager& mgr = microSessionManager( technology );
  mgr.connect( m_transactionType );
  return mgr;
}
