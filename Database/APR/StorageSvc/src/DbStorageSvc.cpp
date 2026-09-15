/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//  ====================================================================
//
//  DbStorageSvc.cpp
//  --------------------------------------------------------------------
//
//  Package   : StorageSvc (POOL)
//  Author    : Markus Frank
//
//  ====================================================================

// Framework include files
#include "PersistentDataModel/Token.h"
#include "DbStorageSvc.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbTransform.h"
#include "StorageSvc/DbConnection.h"
#include "DbDatabaseObj.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/IOODatabase.h"

#include "Gaudi/PluginService.h"

#include <memory>

using namespace std;


namespace pool  {

  // factory function implementation
  IStorageSvc* createStorageSvc(const string& componentName){
    return new DbStorageSvc(componentName);
  }

  typedef const DbTypeInfo    *DbTypeInfoH;
  typedef const DbDatabaseObj *DbDatabaseH;
  typedef       DbDatabaseObj *DbDatabaseHNC;


   
/// Standard Constructor.
DbStorageSvc::DbStorageSvc(const string& name)
: APRMessaging(name),
  m_name(name),
  m_refCount(0),
  m_domH(POOL_StorageType),
  m_ageLimit(2),
  m_type(POOL_StorageType),
  m_implementation(nullptr)
{
  static const char * const als = getenv("POOL_STORAGESVC_DB_AGE_LIMIT");  
  if ( als )    {
    int alimit = 2;
    istringstream buf(als);
    buf >> alimit;
    if ( alimit > 0 && buf.good() )  {
      m_ageLimit = alimit;
      ATH_MSG_INFO( ">   User defined db age limit ($POOL_STORAGESVC_DB_AGE_LIMIT) set to: " << m_ageLimit );
    }
  }  
  // declareProperty("AgeLimit", m_ageLimit);
}

/// Standard destructor.
DbStorageSvc::~DbStorageSvc() {
  m_domH.close().ignore();
  m_domH = 0;
  if( m_implementation ) {
    m_implementation->release();
    m_implementation = nullptr;
  }
}

//--- IInterface::addRef
unsigned int DbStorageSvc::addRef()   {
  m_refCount++;
  return m_refCount;
}

//--- IInterface::release
unsigned int DbStorageSvc::release()   {
  int count = --m_refCount;
  if( count <= 0) {
    delete this;
  }
  return count;
}

/// IService implementation: Initilize Service                          
StatusCode DbStorageSvc::initialize()   {
  return StatusCode::SUCCESS;
}

/// IService implementation: Finalize Service                           
StatusCode DbStorageSvc::finalize()   {
  StatusCode rc = m_domH.close();
  m_domH = 0;
  return rc;
}

std::string DbStorageSvc::getContName(FileDescriptor& refDB, Token& persToken) const {
  if ( m_domH.isValid() )   {
    DbDatabase dbH(DbDatabaseHNC(refDB.dbc()->handle()));
    if ( dbH.isValid() )  {
      return dbH.cntName(persToken);
    }
  }
  return "";
}

/// Retrieve persistent shape from Storage manager.
StatusCode DbStorageSvc::getShape( FileDescriptor&       fDesc,
                                   const Guid&           objType,
                                   ShapeH&               shape)
{
  shape = 0;
  if ( /*0 != &fDesc &&*/ m_domH.isValid() )   {
    DbDatabase dbH(DbDatabaseHNC(fDesc.dbc()->handle()));
    if ( !dbH.isValid() )  {
      StatusCode sc = dbH.open(m_domH, fDesc.PFN(), fDesc.FID(), Io::READ);
      if ( !sc.isSuccess() )    {
        ATH_MSG_ERROR( "Failed to open the Database!" );
        return sc;
      }
    }
    if ( dbH.isValid() )  {
      if ( !dbH.info() ) { // ageing may close DbDatabaseObj w/o invalidating DbDatabase
        StatusCode sc = dbH.open(m_domH, fDesc.PFN(), fDesc.FID(), Io::READ);
        if ( !sc.isSuccess() )    {
          ATH_MSG_ERROR( "Failed to re-open the Database!" );
          return sc;
        }
      }
      shape = dbH.objectShape(objType);
      if ( shape )  {
        return StatusCode::SUCCESS;
      }
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_ERROR( "The storage service is not properly initialized." );
  return StatusCode::FAILURE;
}

/// Create a persistent shape.
ShapeH DbStorageSvc::createShape( const Guid& shapeID )
{
  const DbTypeInfo* typ_info = nullptr;
  if( DbTransform::getShape(shapeID, typ_info).isSuccess() ) {
    return typ_info;
  }
  typ_info = DbTypeInfo::create(shapeID);
  if( typ_info )   {
    ATH_MSG_INFO( "Building shape according to reflection information using shape ID for: " << endmsg
                  << typ_info->clazz().Name() << " [" << shapeID.toString() << "]" );
  } else {
    ATH_MSG_ERROR( "The shape with ID=" << shapeID.toString() << " is unknown." );
  }
  return typ_info;
}

/// Register object for write
StatusCode DbStorageSvc::allocate( FileDescriptor&       fDesc,
                                   const string&         refCont,
                                   int                   technology,
                                   const void*           object,
                                   ShapeH                shape,
                                   Token*&               refpToken)
{
   StatusCode sc = StatusCode::FAILURE;
   refpToken = 0;

   if( shape && object ) {
      void* handle = fDesc.dbc()->handle();
      DbDatabase dbH(static_cast<DbDatabaseHNC>(handle));
      DbContainer cntH(dbH.type());
      sc = cntH.open( dbH, 
                      refCont,
                      DbTypeInfoH(shape),
                      DbType(technology),
                      Io::WRITE);
      if ( sc.isSuccess() ) {
         Token* t = new Token(cntH.token());
         t->setClassID(shape->shapeID());
         sc = cntH.allocate(object, shape, t->oid());
         if ( sc.isSuccess() )  {
            sc = dbH.makeLink(t, t->oid());
            if ( sc.isSuccess() )  {
               refpToken = t;
               return StatusCode::SUCCESS;
            }
         }
         t->release();
      }
   }
   ATH_MSG_ERROR( "Cannot allocate persistent object." << endmsg
       << " Shape Handle :" << static_cast<const void*>(shape)  << endmsg
       << " FID=" << fDesc.FID() << endmsg
       << " Cnt=" << refCont );
   return sc;
}

/// Read a persistent object from the medium.
StatusCode DbStorageSvc::read( const FileDescriptor& fDesc,
                               const Token&          token,
                               ShapeH                shape,
                               void**                object)
{

  Io::IoFlag mode = Io::READ;
  if ( m_domH.isValid() ) {
    DbType typ(token.technology());
    if ( m_domH.type() == typ ) {
      DbDatabase dbH(m_domH.type());
      const string& fid = fDesc.FID();
      if( dbH.open(m_domH, fDesc.PFN(), fid, mode).isSuccess() ) {
         if( dbH.read( token, shape, object).isSuccess() ) {
            return StatusCode::SUCCESS;
         } else {
            ATH_MSG_ERROR( "Could not read object: " << token.toString() );
              return StatusCode::FAILURE;
         }
      }
      ATH_MSG_ERROR( "The requested Database: " << token.dbID().toString() << " cannot be opened!" );
    }
    else {
       ATH_MSG_ERROR( "Wait a minute...You cannot mix the technologies: " << typ.storageName() << " and " << m_domH.type().storageName() );
    }
  }
  return StatusCode::FAILURE;
}

/// Start a new Database Session.
StatusCode DbStorageSvc::startSession(Io::IoFlag accessmode, int technology, int ageLimit) {
  m_type   = DbType(technology).majorType();
  if( m_domH.open(db(), m_type, accessmode).isSuccess() )  {
      m_domH.setAgeLimit(ageLimit==-1 ? m_ageLimit : ageLimit);
      return StatusCode::SUCCESS;
  }
  ATH_MSG_ERROR( "Cannot connect to the domain: " << DbType(technology).storageName() );
  return StatusCode::FAILURE;
}

/// Check the existence of a logical Database unit.
StatusCode DbStorageSvc::existsConnection(const FileDescriptor& fDesc) {
  DbDatabase dbH = m_domH.find(fDesc.FID());
  if ( dbH.isValid() )  {  // Already connected to database ...
    return StatusCode::SUCCESS;
  }
  if( m_domH.existsDbase(fDesc.PFN()) ) {
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Connect to a logical Database unit.
StatusCode DbStorageSvc::connect(Io::IoFlag mod, FileDescriptor& fDesc) {
  StatusCode sc = StatusCode::FAILURE;
  fDesc.setDbc(0);
  DbDatabase dbH = m_domH.find(fDesc.FID());
  if ( !dbH.isValid() )  {
    sc = dbH.open(m_domH, fDesc.PFN(), fDesc.FID(), mod);
    if ( !sc.isSuccess() )    {
      ATH_MSG_ERROR( "Cannot connect to Database: FID=" << fDesc.FID() << " PFN=" << fDesc.PFN() );
      return sc;
    }
  }
  else {
    sc = StatusCode::SUCCESS;
  }
  DbConnection* dbc = new DbConnection(dbH.type().type(), dbH.name(), dbH.ptr());
  fDesc.setDbc(dbc);
  return sc;
}

/// Disconnect from a logical Database unit.
StatusCode DbStorageSvc::disconnect(FileDescriptor& fDesc) {
  DbConnection* dbc = dynamic_cast<DbConnection*>(fDesc.dbc());
  ATH_MSG_DEBUG( "Disconnect request for database: FID=" << fDesc.FID() << " PFN=" << fDesc.PFN() );
  if ( dbc )   {
    DbDatabase  dbH(DbDatabaseHNC(dbc->handle()));
    dbc->release();
    fDesc.setDbc(0);
    ATH_MSG_DEBUG( "Closing database: FID=" << fDesc.FID() );
    return dbH.close();
  }
  return StatusCode::FAILURE;
}

/// Query the access mode of a Database unit.
StatusCode DbStorageSvc::openMode(FileDescriptor& refDB, Io::IoFlag& mode) {
  DbConnection* dbc = dynamic_cast<DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(DbDatabaseHNC(dbc->handle()));
    if ( dbH.isOpen() )  {
      mode = dbH.openMode();
      return StatusCode::SUCCESS;
    }
  }
  mode = Io::INVALID;
  return StatusCode::FAILURE;
}

/// End/Finish an existing Transaction sequence.
StatusCode DbStorageSvc::endTransaction(FileDescriptor& refDB, Transaction::Action typ)
{
   return refDB.dbc()->handle()->transAct( typ );
}

/// Access technology implementations
IOODatabase* DbStorageSvc::db() {
  if( !m_implementation ) {
    const std::string &nam = m_type.storageName();
    m_implementation = Gaudi::PluginService::Factory<IOODatabase*()>::create(nam).release();
    if( !m_implementation ) {
      ATH_MSG_FATAL( "Failed to load plugin for " << nam << " storage type" );
    }
  }
  return m_implementation;
}

} // namespace pool
