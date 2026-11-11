/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SessionTool.h"
#include "Database.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"

#include "StorageSvc/DbType.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"


SessionTool::SessionTool(const std::string& type,
                         const std::string& name,
                         const IInterface* parent)
  : AthAlgTool(type, name, parent),
        m_catalog(nullptr),
        m_ageLimit(-1),
        m_transactionType(Io::INVALID) {
}

void SessionTool::setupFC( Gaudi::IFileCatalog& fileCatalog, int ageLimit ) {
  m_catalog = &fileCatalog;
  m_ageLimit = ageLimit;
}

SessionTool::~SessionTool() {
  closeSession();
}

void SessionTool::closeSession() {
  // order is important
  disconnectAll();
  m_transactionType = Io::INVALID;
  m_technologies.clear();
}

StatusCode SessionTool::finalize() {
  closeSession();
  return AthAlgTool::finalize();
}

void* SessionTool::readObject( const Token& token, void* object ) {
  void* result {};
  if( m_transactionType != Io::INVALID ) {
    Database db( *this, token.dbID().toString(), Database::Specification::FID );
    if ( db.openMode() == Io::INVALID ) {
      db.setTechnology( token.technology() );
      db.connectForRead();
    }
    result = db.readObject( token, object );
  }
  return result;
}

Token* SessionTool::registerForWrite( const Placement& place,
                                      const void* object,
                                      const RootType& type ) {
  if( m_transactionType != Io::WRITE && m_transactionType != Io::APPEND ) {
    return 0;
  }
  Database db( *this, place.fileName(), Database::Specification::PFN );
  if ( db.openMode() == Io::INVALID ) {
    db.setTechnology( place.technology() );
    db.connectForWrite();
  }
  return db.writeObject( place.containerName(), place.technology(), object, type );
}

void SessionTool::registerDatabaseHandler( DatabaseHandler* dbHandler, const std::string& lfn ) {
  std::set< DatabaseHandler* >::iterator iDatabase = m_databases.find( dbHandler );
  if ( iDatabase == m_databases.end() ) {
    iDatabase = m_databases.insert( dbHandler ).first;
    m_fidToDb.insert( std::make_pair( dbHandler->fid(), dbHandler ) );
    m_pfnToDb.insert( std::make_pair( dbHandler->pfn(), dbHandler ) );
    m_fidToLfns.insert( std::make_pair( dbHandler->fid(), std::set< std::string >() ) );
  }
  if ( ! lfn.empty() ) {
    m_fidToLfns.find( dbHandler->fid() )->second.insert( lfn );
    m_lfnToDb.insert( std::make_pair( lfn, *iDatabase ) );
  }
}

void SessionTool::deregisterDatabaseHandler( DatabaseHandler* dbHandler ) {
  std::set< DatabaseHandler* >::iterator idb = m_databases.find( dbHandler );
  if ( idb != m_databases.end() ) {
    m_fidToDb.erase( (*idb)->fid() );
    m_pfnToDb.erase( (*idb)->pfn() );
    std::map< std::string, std::set< std::string > >::iterator iLfnSet = m_fidToLfns.find( (*idb)->fid() );
    if ( iLfnSet != m_fidToLfns.end() ) {
      for ( std::set< std::string >::iterator iLfn = iLfnSet->second.begin(); iLfn != iLfnSet->second.end(); ++iLfn ) {
        m_lfnToDb.erase( *iLfn );
      }
      m_fidToLfns.erase( iLfnSet );
    }
    m_databases.erase( idb );
  }
}

DatabaseHandler* SessionTool::lookupByFID( const std::string& fid ) {
  std::map< std::string, DatabaseHandler* >::iterator idb = m_fidToDb.find( fid );
  return idb == m_fidToDb.end() ? 0 : idb->second;
}

DatabaseHandler* SessionTool::lookupByPFN( const std::string& pfn ) {
  std::map< std::string, DatabaseHandler* >::iterator idb = m_pfnToDb.find( pfn );
  return idb == m_pfnToDb.end() ? 0 : idb->second;
}

DatabaseHandler* SessionTool::lookupByLFN( const std::string& lfn ) {
  std::map< std::string, DatabaseHandler* >::iterator idb = m_lfnToDb.find( lfn );
  return idb == m_lfnToDb.end() ? 0 : idb->second;
}

bool SessionTool::disconnectAll() {
  bool ret = true;
  for( auto& iManager : m_technologies ) {
    if( !iManager.second->disconnectAll() ) ret = false;
  }
  return ret;
}

bool SessionTool::startTransaction( Io::IoFlag type ) {
  if( m_transactionType != Io::INVALID ) return false;
  m_transactionType = type;
  return true;
}

bool SessionTool::commit() {
  if( m_transactionType != Io::INVALID ) {
    bool OK = true;
    for( auto db : m_databases ) {
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

bool SessionTool::commitAndHold() {
  if( m_transactionType != Io::INVALID ) {
    bool OK = true;
    for( auto db : m_databases ) {
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

std::unique_ptr<Database> SessionTool::databaseHandle( const std::string& dbName,
                                                       Database::Specification::NameType dbNameType ) {
  if( m_transactionType != Io::INVALID ) {
     return std::make_unique<Database>( *this, dbName, dbNameType );
  }
  return nullptr;
}

Gaudi::IFileCatalog& SessionTool::fileCatalog() {
  return *m_catalog;
}

MicroSessionManager& SessionTool::microSessionManager( long technology ) {
  DbType dbType( technology );
  long majorType = dbType.majorType();
  auto iManager = m_technologies.find( majorType );
  if ( iManager != m_technologies.end() ) {
    return *(iManager->second);
  }
  // Technology does not exist. Create the new session.
  auto mgr = new MicroSessionManager( *this, majorType, m_ageLimit );
  m_technologies.insert( std::make_pair( majorType, mgr ) );
  return *mgr;
}

DatabaseHandler* SessionTool::connect( long technology,
                                       Io::IoFlag mode,
                                       const std::string& fid,
                                       const std::string& pfn ) {
  return this->microSessionManager(technology).connect(mode, fid, pfn);
}

void SessionTool::disconnect( DatabaseHandler* database ) {
  return this->microSessionManager(database->technology()).disconnect(database);
}

std::string SessionTool::fidForPfn( long technology, const std::string& pfn ) {
  return this->microSessionManager(technology).fidForPfn(pfn);
}

bool SessionTool::attributeOfType( long technology,
                                   const std::string& attributeName,
                                   void* data,
                                   const std::type_info& typeInfo,
                                   const std::string& option ) {
  return this->microSessionManager(technology).attributeOfType(attributeName, data, typeInfo, option);
}

bool SessionTool::setAttributeOfType( long technology,
                                      const std::string& attributeName,
                                      const void* data,
                                      const std::type_info& typeInfo,
                                      const std::string& option ) {
  return this->microSessionManager(technology).setAttributeOfType(attributeName, data, typeInfo, option);
}
