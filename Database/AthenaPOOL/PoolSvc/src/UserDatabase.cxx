/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "UserDatabase.h"
#include "UserSession.h"
#include "DatabaseHandler.h"
#include "MicroSessionManager.h"
#include "DatabaseRegistry.h"

#include "PoolSvc/IFileCatalog.h"

#include "StorageSvc/DbType.h"
#include "StorageSvc/pool.h"

#include <exception>

static const std::string& emptyString = "";

pool::UserDatabase::UserDatabase( pool::UserSession& session,
                                  const std::string& name,
                                  const pool::DatabaseSpecification::NameType nameType ):
  APRMessaging("PersistencySvc::UserDB"),                                                  
  m_session( session ),
  m_catalog( session.fileCatalog() ),
  m_transactionType( session.type() ),
  m_registry( session.registry() ),
  m_name( name ),
  m_nameType( nameType ),
  m_technology( 0 ),
  m_technologySet( false ),
  m_databaseHandler( 0 ),
  m_openMode( Io::INVALID ),
  m_alreadyConnected( false ),
  m_the_fid( "" ),
  m_the_pfn( "" )
{
  this->checkInRegistry();
}


pool::UserDatabase::~UserDatabase()
{}


void
pool::UserDatabase::connectForRead()
{
  if( !m_databaseHandler && m_transactionType != Io::INVALID ) {
    // Check if the database is already connected
    if( !checkInRegistry() ) {
      // It is not. Connect !
      switch( m_nameType ) {
      case pool::DatabaseSpecification::PFN:
        if ( this->fid().empty() ) {
          throw std::runtime_error( "PFN \"" + m_name + "\" is not existing (APR: \" UserDatabase::connectForRead \" from \" PersistencySvc \")" );
        }
        break;

      case pool::DatabaseSpecification::FID:
        if ( this->pfn().empty() ) {
          throw std::runtime_error( "FID \"" + m_name + "\" is not existing in the catalog (APR: \" UserDatabase::connectForRead \" from \" PersistencySvc \")" );
        }
        break;
      case pool::DatabaseSpecification::LFN:
        {
          std::string lfn = m_name;
          if ( this->fid().empty() ) {
            throw std::runtime_error( "LFN \"" + m_name + "\" is not existing in the catalog (APR: \" UserDatabase::connectForRead \" from \" PersistencySvc \")" );
          }
          this->connectForRead();
          m_registry.registerDatabaseHandler( m_databaseHandler, lfn );
        }
      return;
      break;
      default:
        throw std::runtime_error( "Unknown database name type (APR: \" UserDatabase::connectForRead \" from \" PersistencySvc \")" );
      };

      // Now we have all the usefull information to open the file.
      // Check the registry now that we have the FID (in case of ambiguous PFNs)
      m_databaseHandler = m_registry.lookupByFID( m_the_fid );
      if( !m_databaseHandler ) {
         // still no luck - make a new connection
         m_databaseHandler = m_session.microSessionManager( m_technology ).connect( m_transactionType, m_the_fid, m_the_pfn );
      }
      if( m_databaseHandler ) {
        m_openMode = Io::READ;
      }
      else {
        throw std::runtime_error( "Could not connect to the file (APR: \" UserDatabase::connectForRead \" from \" PersistencySvc \")" );
      }
    }
  }
}


void*
pool::UserDatabase::readObject( const Token& token, void* object )
{
   if( !m_databaseHandler) {
      throw std::runtime_error( "Could not open a database for read object. (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
   }
   return m_databaseHandler->readObject( token, object );
}


void
pool::UserDatabase::connectForWrite()
{
  if( !m_databaseHandler && m_transactionType != Io::INVALID ) {
    if ( m_transactionType != Io::WRITE && m_transactionType != Io::APPEND ) {
      throw std::runtime_error( "Could not open a database for write outside an update transaction. (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
    }

    if ( this->checkInRegistry() ) {
      if ( m_openMode == Io::READ ) {
        throw std::runtime_error( "Could not open a database for write that is already connected for read. (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
      }
    }
    else { // The database is not yet connected.
      bool dbRegistered = false;
      switch( m_nameType ) {
      case pool::DatabaseSpecification::PFN:
        m_the_pfn = m_name;
        if ( this->fid().empty() ) {
	  // Check if the technology is already set
	  if( ! m_technologySet ) {
	     throw std::runtime_error( "The back end technology has not been specified (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
	  }
	  // register in the catalog 
	  pool::DbType dbType( m_technology );
	  pool::DbType dbTypeMajor( dbType.majorType() );
	  m_catalog.registerPFN( m_the_pfn.substr(0, m_the_pfn.find('?')), dbTypeMajor.storageName(), m_the_fid );
    ATH_MSG_DEBUG("registered PFN: " << m_the_pfn << " with FID:" << m_the_fid);
	  dbRegistered = true;
        }
        break;
      case pool::DatabaseSpecification::FID:
        m_the_fid = m_name;
        if ( this->pfn().empty() ) {
          throw std::runtime_error( "Could not find the FID \"" + m_name + "\" in the file catalog (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
        }
        break;
      case pool::DatabaseSpecification::LFN:
        {
          std::string lfn = m_name;
          if ( this->fid().empty() ) {
            throw std::runtime_error( "Could not find the LFN \"" + m_name  + "\" in the file catalog (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
          }
          this->connectForWrite();
          m_registry.registerDatabaseHandler( m_databaseHandler, lfn );
        }
      return;
      break;
      default:
        throw std::runtime_error( "Unknown database name type (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
      };

      m_databaseHandler = m_session.microSessionManager( m_technology ).connect( m_transactionType, m_the_fid, m_the_pfn );
      if( !m_databaseHandler ) {
        if( dbRegistered ) {
          // creation failed, remove entry from the in-memory catalog
          m_catalog.deleteFID( m_the_fid );
        }
        throw std::runtime_error( "Could not connect to the file (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
      }
      m_openMode = m_transactionType == Io::WRITE ? Io::WRITE : Io::APPEND;
    } // Connection established

  } // Database handler retrieved
}


Token*
pool::UserDatabase::writeObject( const std::string& containerName,
                                                    long minorTechnology,
                                                    const void* object,
                                                    const RootType& type )
{
   if( !m_databaseHandler) {
      throw std::runtime_error( "Could not open a database for write object. (APR: \" UserDatabase::connectForWrite \" from \" PersistencySvc \")" );
   }
   return m_databaseHandler->writeObject( containerName, minorTechnology, object, type );
}


void
pool::UserDatabase::disconnect()
{
  if ( m_databaseHandler ) {
    m_session.microSessionManager( m_technology ).disconnect( m_databaseHandler );
    m_openMode = Io::INVALID;
  }
}


Io::IoFlag
pool::UserDatabase::openMode() const
{
  return m_openMode;
}


const std::string&
pool::UserDatabase::fid()
{
  if ( m_databaseHandler ) return m_databaseHandler->fid();
  else {
    if ( m_nameType == pool::DatabaseSpecification::FID ) return m_name;
    else if ( ! m_the_fid.empty() ) return m_the_fid;
    else {
      if ( m_nameType == pool::DatabaseSpecification::PFN ) {
         std::string technology;
         m_catalog.lookupFileByPFN( m_name.substr(0, m_name.find('?')), m_the_fid, technology );
         ATH_MSG_DEBUG("lookupPFN: " << m_name << " returned FID: '" << m_the_fid << "' tech=" << technology);
         if ( ! m_the_fid.empty() ) {
            if( technology.empty() ) {
               m_nameType = DatabaseSpecification::LFN;
               ATH_MSG_DEBUG("Retrying 'connect' using assumed PFN " << m_name << " as LFN (no tech found in PFC)" );
               return m_the_fid;
            }
            m_the_pfn = m_name;
            m_technology = pool::DbType::getType( technology ).majorType();
            m_alreadyConnected = true;
         }
         else {
           if( m_transactionType != Io::WRITE ) { // Fetch the FID from the db itself !
              if( !m_technologySet ) {
                 ATH_MSG_DEBUG("Opening database '" << m_name << "' with no technology set");
                 m_technology = pool::ROOT_StorageType.type();
                 m_technologySet = true;
              }
              m_the_fid = m_session.microSessionManager( m_technology ).fidForPfn( m_name );
              if( ! m_the_fid.empty() ) {
                 // sanity check - verify that the FID is not registered in PFC under a different name
                 std::string  pfn, tech;
                 m_catalog.getFirstPFN( m_the_fid, pfn, tech );
                 if( !pfn.empty() ) {
                    ATH_MSG_WARNING("Opening file '" << m_name << "' which is already registered in the Catalog as '" << pfn 
                                    <<"' (GUID " << m_the_fid << ") - this is not supported and may even lead to a crash!" );
                 }
                 m_the_pfn = m_name;
                 m_alreadyConnected = true;
              }
           }
        }
      } /* not PFN */
      else if ( m_nameType == pool::DatabaseSpecification::LFN ) {
         m_the_fid = m_catalog.lookupLFN( m_name );
      }
      if ( ! m_the_fid.empty() ) {
        m_name = m_the_fid;
        m_nameType = pool::DatabaseSpecification::FID;
      }
      return m_the_fid;
    }
    return emptyString;
  }
}


const std::string&
pool::UserDatabase::pfn()
{
  if( m_databaseHandler )  return m_databaseHandler->pfn();
  if( m_nameType == pool::DatabaseSpecification::PFN )  return m_name;
  if( ! m_the_pfn.empty() )  return m_the_pfn;
  
  if( m_nameType == pool::DatabaseSpecification::LFN ) {
     m_the_fid = m_catalog.lookupLFN( m_name );
     if ( ! m_the_fid.empty() ) {
        m_name = m_the_fid;
        m_nameType = pool::DatabaseSpecification::FID;
        // go from FID=>PFN in the next step
     }
  }
  if( m_nameType == pool::DatabaseSpecification::FID ) {
     std::string technology;
     m_catalog.getFirstPFN( m_name, m_the_pfn, technology );
     if( !m_the_pfn.empty() ) {
        m_the_fid = m_name;
        m_technology = pool::DbType::getType( technology ).majorType();
        m_alreadyConnected = true;
     }
     return m_the_pfn; 
  }
  return emptyString;
}


bool
pool::UserDatabase::setTechnology( long technology )
{
  if ( m_alreadyConnected ) return false;
  else {
    pool::DbType dbType( technology );
    m_technology = dbType.majorType();
    m_technologySet = true;
    return true;
  }
}


long
pool::UserDatabase::technology() const
{
  return m_technology;
}


std::vector< std::string >
pool::UserDatabase::containers()
{
  std::vector< std::string > containers;
  if ( m_databaseHandler ) {
    containers = m_databaseHandler->containers();
  }
  return containers;
}


pool::IContainer*
pool::UserDatabase::containerHandle( const std::string& name )
{
  pool::IContainer* container = 0;
  if ( m_databaseHandler ) {
    container = m_databaseHandler->container( name );
  }
  return container;
}


bool
pool::UserDatabase::checkInRegistry()
{
  // Check first if the database is already connected.
  switch( m_nameType ) {
  case pool::DatabaseSpecification::PFN:
    m_databaseHandler = m_registry.lookupByPFN( m_name );
    break;
  case pool::DatabaseSpecification::FID:
    m_databaseHandler = m_registry.lookupByFID( m_name );
    break;
  case pool::DatabaseSpecification::LFN:
    m_databaseHandler = m_registry.lookupByLFN( m_name );
    break;
  default:
    throw std::runtime_error( "Only PFN, LFN and FID database types are currently supported (APR: \" UserDatabase::checkInRegistry \" from \" PersistencySvc \")" );
  };
  if ( m_databaseHandler ) {
    m_alreadyConnected = true;
    m_technology = m_databaseHandler->technology();
    if ( m_databaseHandler->accessMode() == Io::APPEND ) {
      m_openMode = Io::APPEND;
    }
    else if ( m_databaseHandler->accessMode() == Io::WRITE ) {
      m_openMode = Io::WRITE;
    }
    else {
      m_openMode = Io::READ;
    }
    m_the_fid = m_name = m_databaseHandler->fid();
    m_the_pfn = m_databaseHandler->pfn();
    m_nameType = pool::DatabaseSpecification::FID;
    return true;
  }
  else return false;
}

pool::FileDescriptor*
pool::UserDatabase::fileDescriptor() 
{
  return m_databaseHandler? &m_databaseHandler->fileDescriptor() : nullptr;
}


pool::ITechnologySpecificAttributes&
pool::UserDatabase::technologySpecificAttributes()
{
  return static_cast< pool::ITechnologySpecificAttributes& >( *this );
}


bool
pool::UserDatabase::attributeOfType( const std::string& attributeName,
                                                     void* data,
                                                     const std::type_info& typeInfo,
                                                     const std::string& option )
{
  if ( ! m_databaseHandler ) return false;
  else return m_databaseHandler->attribute( attributeName, data, typeInfo, option );
}


bool
pool::UserDatabase::setAttributeOfType( const std::string& attributeName,
                                                        const void* data,
                                                        const std::type_info& typeInfo,
                                                        const std::string& option )
{
  if ( ! m_databaseHandler ) return false;
  else return m_databaseHandler->setAttribute( attributeName, data, typeInfo, option );
}
