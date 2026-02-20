/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "POOLCore/DbPrint.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/Transaction.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbTransform.h"
#include "StorageSvc/DbConnection.h"
#include "DbDatabaseObj.h"
#include "StorageSvc/FileDescriptor.h"

#include <vector>
#include <memory>
#include <map>

using namespace std;


namespace pool  {

  // factory function implementation
  IStorageSvc* createStorageSvc(const string& componentName){
    return new DbStorageSvc(componentName);
  }

  typedef const DbTypeInfo    *DbTypeInfoH;
  typedef const DbDatabaseObj *DbDatabaseH;
  typedef       DbDatabaseObj *DbDatabaseHNC;

  class DbClassMap : public map<TypeH, Guid> {};


   
/// Standard Constructor.
DbStorageSvc::DbStorageSvc(const string& name)
: APRMessaging(name),
  m_name(name),
  m_refCount(0),
  m_sesH(),
  m_domH(POOL_StorageType),
  m_ageLimit(2),
  m_type(POOL_StorageType)
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
  m_sesH = 0;
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

//--- IInterface::queryInterface
StatusCode DbStorageSvc::queryInterface(const Guid& riid, void** ppvInterface)  {
  if ( IStorageSvc::interfaceID() == riid )  {
    *ppvInterface = static_cast<IStorageSvc*>(this);
  }
  addRef();
  return StatusCode::SUCCESS;
}

/// IService implementation: Initilize Service                          
StatusCode DbStorageSvc::initialize()   {
  return StatusCode::SUCCESS;
}

/// IService implementation: Finalize Service                           
StatusCode DbStorageSvc::finalize()   {
  StatusCode rc = m_domH.close();
  m_domH = 0;
  m_sesH = 0;
  return rc;
}

std::string DbStorageSvc::getContName(FileDescriptor& refDB, Token& persToken)  {
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
      StatusCode sc = dbH.open(m_domH, fDesc.PFN(), fDesc.FID(), pool::READ);
      if ( !sc.isSuccess() )    {
        ATH_MSG_ERROR( "Failed to open the Database!" );
        return sc;
      }
    }
    if ( dbH.isValid() )  {
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
      DbDatabase dbH((DbDatabaseHNC)handle);
      DbContainer cntH(dbH.type());
      sc = cntH.open( dbH, 
                      refCont,
                      DbTypeInfoH(shape),
                      DbType(technology),
                      pool::CREATE|pool::UPDATE);
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
       << " Shape Handle :" << (const void*)shape  << endmsg
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

  pool::AccessMode mode = pool::READ;
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
StatusCode DbStorageSvc::startSession(int accessmode,int technology,SessionH& refSession)  {
  m_type   = DbType(technology).majorType();
  int typ  = DbType(technology).majorType();
  if ( m_type.majorType() == typ )  {  // Maybe implement this later
    refSession = 0;
    if ( m_sesH.open().isSuccess() )  {
      if ( m_domH.open(m_sesH, m_type, accessmode).isSuccess() )  {
        m_domH.setAgeLimit(m_ageLimit);
        refSession = SessionH(m_domH.ptr());
        return StatusCode::SUCCESS;
      }
      ATH_MSG_ERROR( "Cannot connect to the domain: " << DbType(technology).storageName() );
      return StatusCode::FAILURE;
    }
    ATH_MSG_ERROR( "Cannot start the Database session." );
    return StatusCode::FAILURE;
  }
  ATH_MSG_ERROR( "Cannot start database session, the technology type does not match." );
  return StatusCode::FAILURE;
}

/// End the Database session.
StatusCode DbStorageSvc::endSession(const SessionH session) {
  StatusCode sc = StatusCode::FAILURE;
  if( session == SessionH(m_domH.ptr()) )  {
    sc = m_domH.close();
    m_sesH = 0;
  }
  return sc;
}

/// Check the existence of a logical Database unit.
StatusCode 
DbStorageSvc::existsConnection( const SessionH session, int /* mode */,const FileDescriptor& fDesc) {
  if ( m_domH.isValid() && session == SessionH(m_domH.ptr()) )   {
    DbDatabase dbH = m_domH.find(fDesc.FID());
    if ( dbH.isValid() )  {  // Already connected to database ...
      return StatusCode::SUCCESS;
    }
    if( m_domH.existsDbase(fDesc.PFN()) ) {
      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

/// Connect to a logical Database unit.
StatusCode DbStorageSvc::connect(const SessionH session,int mod,FileDescriptor& fDesc)  {
  StatusCode sc = StatusCode::FAILURE;
  fDesc.setDbc(0);
  if ( m_domH.isValid() && session == SessionH(m_domH.ptr()) )   {
    DbDatabase dbH = m_domH.find(fDesc.FID());
    if ( dbH.isValid() )  {
      int all = pool::READ + pool::CREATE + pool::UPDATE;
      int wr  = pool::CREATE + pool::UPDATE;
      int m   = dbH.openMode();
      if ( (m&all) && mod == pool::READ )
        ;
      else if ( m&wr && mod&pool::CREATE )
        ;
      else if ( m&wr && mod&pool::UPDATE )
        ;
      else
        dbH.close().ignore();
    }
    // No Else!
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
  }
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
StatusCode DbStorageSvc::openMode(FileDescriptor& refDB, int& mode) {
  DbConnection* dbc = dynamic_cast<DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(DbDatabaseHNC(dbc->handle()));
    if ( dbH.isOpen() )  {
      mode = dbH.openMode();
      return StatusCode::SUCCESS;
    }
  }
  mode = pool::NOT_OPEN;
  return StatusCode::FAILURE;
}


/// End/Finish an existing Transaction sequence.
StatusCode DbStorageSvc::endTransaction( ConnectionH connection, Transaction::Action typ)
{
   return ( (DbDatabaseObj*)connection->handle() )->transAct( typ );
}

/// Access options for a given database domain.
StatusCode DbStorageSvc::getDomainOption(const SessionH  sessionH, DbOption& opt)  {
  if ( m_domH.isValid() && sessionH == SessionH(m_domH.ptr()) )   {
    return m_domH.getOption(opt);
  }
  ATH_MSG_ERROR( "Cannot connect to proper technology domain." );
  return StatusCode::FAILURE;
}

/// Set options for a given database domain.
StatusCode
DbStorageSvc::setDomainOption(const SessionH  sessionH, const DbOption& opt)  {
  if ( m_domH.isValid() && sessionH == SessionH(m_domH.ptr()) )   {
    return m_domH.setOption(opt);
  }
  ATH_MSG_ERROR( "Cannot connect to proper technology domain." );
  return StatusCode::FAILURE;
}

} // namespace pool
