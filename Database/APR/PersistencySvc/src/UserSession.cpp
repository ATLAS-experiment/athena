/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "UserSession.h"
#include "PersistencySvc/DatabaseConnectionPolicy.h"
#include "GlobalTransaction.h"
#include "DatabaseRegistry.h"
#include "UserDatabase.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"

pool::PersistencySvc::UserSession::UserSession( pool::IFileCatalog& fileCatalog ):
  m_policy( 0 ),
  m_catalog( &fileCatalog ),
  m_registry( 0 ),
  m_transaction( 0 )
{
  m_policy = new pool::DatabaseConnectionPolicy;
  m_registry = new pool::PersistencySvc::DatabaseRegistry();
  m_transaction = new pool::PersistencySvc::GlobalTransaction( *m_registry );
}

pool::PersistencySvc::UserSession::~UserSession()
{
  // order is important
  m_technologies.clear();
  delete m_transaction;
  delete m_registry;
  delete m_policy;
}


void*
pool::PersistencySvc::UserSession::readObject( const Token& token, void* object )
{
  void* result( (void*)0 );
  if ( m_transaction->isActive() ) {
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
  if( !m_transaction->isActive() || m_transaction->type() != pool::ITransaction::UPDATE ) {
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
      
pool::ITransaction&
pool::PersistencySvc::UserSession::transaction()
{
  return static_cast<pool::ITransaction&>( *m_transaction );
}

const pool::ITransaction&
pool::PersistencySvc::UserSession::transaction() const
{
  return static_cast<const pool::ITransaction&>( *m_transaction );
}

std::unique_ptr<pool::IDatabase>
pool::PersistencySvc::UserSession::databaseHandle( const std::string& dbName,
                                                   DatabaseSpecification::NameType dbNameType )
{
  if ( m_transaction->isActive() ) {
     return std::make_unique<UserDatabase>( *this, dbName, dbNameType );
  }
  return nullptr;
}

pool::ITransaction&
pool::PersistencySvc::UserSession::globalTransaction()
{
  return static_cast< pool::ITransaction& >( *m_transaction );
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
  mgr.connect( *m_transaction );
  return mgr;
}
