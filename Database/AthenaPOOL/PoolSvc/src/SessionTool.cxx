/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SessionTool.h"
#include "DatabaseRegistry.h"
#include "UserDatabase.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"


SessionTool::SessionTool(const std::string& type,
                         const std::string& name,
                         const IInterface* parent)
  : AthAlgTool(type, name, parent),
    m_catalog(nullptr),
    m_ageLimit(-1),
    m_registry( std::make_unique<pool::DatabaseRegistry>() ),
    m_transactionType(Io::INVALID)
{
}

void
SessionTool::setupFC( Gaudi::IFileCatalog& fileCatalog, int ageLimit )
{
  m_catalog = &fileCatalog;
  m_ageLimit = ageLimit;
}

SessionTool::~SessionTool()
{
  closeSession();
}

void SessionTool::closeSession() 
{
  // order is important
  disconnectAll();
  m_transactionType = Io::INVALID;
  m_technologies.clear();
}

StatusCode SessionTool::finalize()
{
  closeSession();
  return AthAlgTool::finalize();
}


void*
SessionTool::readObject( const Token& token, void* object )
{
  void* result {};
  if( m_transactionType != Io::INVALID ) {
    pool::UserDatabase db( *this, token.dbID().toString(), pool::DatabaseSpecification::FID );
    if ( db.openMode() == Io::INVALID ) {
      db.setTechnology( token.technology() );
      db.connectForRead();
    }
    result = db.readObject( token, object );
  }
  return result;
}

Token*
SessionTool::registerForWrite( const Placement& place,
                                                     const void* object,
                                                     const RootType& type )
{
  if( m_transactionType != Io::WRITE && m_transactionType != Io::APPEND ) {
    return 0;
  }
  pool::UserDatabase db( *this, place.fileName(), pool::DatabaseSpecification::PFN );
  if ( db.openMode() == Io::INVALID ) {
    db.setTechnology( place.technology() );
    db.connectForWrite();
  }
  return db.writeObject( place.containerName(), place.technology(), object, type );
}


bool
SessionTool::disconnectAll()
{
  bool ret = true;
  for( auto& iManager : m_technologies ) {
    if( !iManager.second->disconnectAll() ) ret = false;
  }
  return ret;
}
      
// MN: rename?
bool
SessionTool::start( Io::IoFlag type )
{
  if( m_transactionType != Io::INVALID ) return false;
  m_transactionType = type;
  return true;
}


bool
SessionTool::commit()
{
  if( m_transactionType != Io::INVALID ) {
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
SessionTool::commitAndHold()
{
  if( m_transactionType != Io::INVALID ) {
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
SessionTool::databaseHandle( const std::string& dbName,
                             pool::DatabaseSpecification::NameType dbNameType )
{
  if( m_transactionType != Io::INVALID ) {
     return std::make_unique<pool::UserDatabase>( *this, dbName, dbNameType );
  }
  return nullptr;
}

Gaudi::IFileCatalog&
SessionTool::fileCatalog()
{
  return *m_catalog;
}


pool::IStorageSvc&
SessionTool::getStorageSvc( long technology ) 
{ 
  return microSessionManager( technology ).getStorageSvc(); 
}


pool::MicroSessionManager&
SessionTool::microSessionManager( long technology )
{
  pool::DbType dbType( technology );
  long majorType = dbType.majorType();
  auto iManager = m_technologies.find( majorType );
  if ( iManager != m_technologies.end() ) {
    iManager->second->connect( m_transactionType, m_ageLimit );
    return *(iManager->second);
  }
  // Technology does not exist. Create the new session.
  auto mgr = new pool::MicroSessionManager( *m_registry, majorType );
  m_technologies.insert( std::make_pair( majorType, mgr ) );
  mgr->connect( m_transactionType, m_ageLimit );
  return *mgr;
}
