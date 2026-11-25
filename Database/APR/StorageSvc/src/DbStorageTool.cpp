/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PersistentDataModel/Token.h"
#include "DbStorageTool.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbTransform.h"
#include "StorageSvc/DbConnection.h"
#include "DbDatabaseObj.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/ITechFactory.h"

#include "Gaudi/PluginService.h"

#include <memory>

/// Standard Constructor.
DbStorageTool::DbStorageTool(const std::string& type, const std::string& name, const IInterface* parent)
: AthAlgTool(type, name, parent),
  m_name(name),
  m_domH(pool::POOL_StorageType),
  m_ageLimit(2),
  m_type( m_domH.type() ),
  m_techFactory(nullptr)
{ }

/// Standard destructor.
DbStorageTool::~DbStorageTool() {
  endSession().ignore();
  if( m_techFactory ) {
    m_techFactory->release();
    m_techFactory = nullptr;
  }
}


std::string DbStorageTool::getContName(FileDescriptor& refDB, Token& persToken) const {
  if ( m_domH.isValid() )   {
    DbDatabase dbH( refDB.dbc()->handle() );
    if ( dbH.isValid() )  {
      return dbH.cntName(persToken);
    }
  }
  return "";
}

/// Retrieve persistent tinfo from Storage manager.
StatusCode DbStorageTool::getShape( FileDescriptor&       fDesc,
                                   const Guid&           objType,
                                   const DbTypeInfo*&    tinfo)
{
  tinfo = 0;
  if( m_domH.isValid() )   {
    DbDatabase dbH( fDesc.dbc()->handle()) ;
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
      tinfo = dbH.objectShape(objType);
      if ( tinfo )  {
        return StatusCode::SUCCESS;
      }
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_ERROR( "The storage service is not properly initialized." );
  return StatusCode::FAILURE;
}

/// Create a persistent tinfo.
const DbTypeInfo* DbStorageTool::createShape( const Guid& typeID )
{
  const DbTypeInfo* typ_info = nullptr;
  if( DbTransform::getShape(typeID, typ_info).isSuccess() ) {
    return typ_info;
  }
  typ_info = DbTypeInfo::create(typeID);
  if( typ_info )   {
    ATH_MSG_INFO( "Building shape according to reflection information using shape ID for: " << endmsg
                  << typ_info->clazz().Name() << " [" << typeID.toString() << "]" );
  } else {
    ATH_MSG_ERROR( "The tinfo with ID=" << typeID.toString() << " is unknown." );
  }
  return typ_info;
}

/// Register object for write
StatusCode DbStorageTool::write( FileDescriptor&       fDesc,
                                const std::string&    refCont,
                                int                   technology,
                                const void*           object,
                                const RootType        type,
                                Token*&               refpToken)
{
   StatusCode sc = StatusCode::FAILURE;
   refpToken = 0;

   // Get the persistent tinfo.
   const DbTypeInfo* tinfo = nullptr;
   Guid guid = DbReflex::guid(type);
   if( !getShape( fDesc, guid, tinfo ).isSuccess() ) {
      tinfo = createShape( guid );
   }
   if( tinfo && object ) {
      DbDatabase dbH( fDesc.dbc()->handle() );
      DbContainer cntH(dbH.type());
      sc = cntH.open( dbH, refCont, tinfo, DbType(technology), Io::WRITE );
      if ( sc.isSuccess() ) {
         Token* t = new Token(cntH.token());
         t->setClassID(tinfo->typeID());
         sc = cntH.allocate(object, tinfo, t->oid());
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
   ATH_MSG_ERROR( "Cannot store object." << endmsg
       << " Shape Handle :" << static_cast<const void*>(tinfo)  << endmsg
       << " FID=" << fDesc.FID() << endmsg
       << " Cnt=" << refCont );
   return sc;
}


/// Read a persistent object from the medium.
StatusCode DbStorageTool::read( FileDescriptor& fDesc,
                               const Token&    token,
                               void**          object)
{
  // Get the persistent tinfo
  const DbTypeInfo* tinfo = nullptr;
  if( !getShape( fDesc, token.classID(), tinfo ).isSuccess() ) {
    return StatusCode::FAILURE;
  }
  Io::IoFlag mode = Io::READ;
  if ( m_domH.isValid() ) {
    DbType typ(token.technology());
    if ( m_domH.type() == typ ) {
      DbDatabase dbH(m_domH.type());
      const std::string& fid = fDesc.FID();
      if( dbH.open(m_domH, fDesc.PFN(), fid, mode).isSuccess() ) {
         if( dbH.read( token, tinfo, object).isSuccess() ) {
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
StatusCode DbStorageTool::startSession(Io::IoFlag accessmode, int technology, int ageLimit) {
  m_type   = DbType(technology).majorType();
  if( m_domH.open(getTechFactory(), m_type, accessmode).isSuccess() )  {
      m_domH.setAgeLimit(ageLimit==-1 ? m_ageLimit : ageLimit);
      return StatusCode::SUCCESS;
  }
  ATH_MSG_ERROR( "Cannot connect to the domain: " << DbType(technology).storageName() );
  return StatusCode::FAILURE;
}

/// Connect to a logical Database unit.
StatusCode DbStorageTool::connect(Io::IoFlag mod, FileDescriptor& fDesc) {
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
StatusCode DbStorageTool::disconnect(FileDescriptor& fDesc) {
  DbConnection* dbc = fDesc.dbc();
  ATH_MSG_DEBUG( "Disconnect request for database: FID=" << fDesc.FID() << " PFN=" << fDesc.PFN() );
  if ( dbc )   {
    DbDatabase  dbH( dbc->handle() );
    dbc->release();
    fDesc.setDbc(0);
    ATH_MSG_DEBUG( "Closing database: FID=" << fDesc.FID() );
    return dbH.close();
  }
  return StatusCode::FAILURE;
}

/// Query the access mode of a Database unit.
StatusCode DbStorageTool::openMode(FileDescriptor& refDB, Io::IoFlag& mode) {
  DbConnection* dbc = refDB.dbc();
  if ( dbc )   {
    DbDatabase  dbH( dbc->handle() );
    if ( dbH.isOpen() )  {
      mode = dbH.openMode();
      return StatusCode::SUCCESS;
    }
  }
  mode = Io::INVALID;
  return StatusCode::FAILURE;
}

/// End/Finish an existing Transaction sequence.
StatusCode DbStorageTool::endTransaction(FileDescriptor& refDB, Transaction::Action typ)
{
   return refDB.dbc()->handle()->transAct( typ );
}

/// Access technology implementations
ITechFactory* DbStorageTool::getTechFactory() {
  if( !m_techFactory ) {
    const std::string &nam = m_type.storageName();
    m_techFactory = Gaudi::PluginService::Factory<ITechFactory*()>::create(nam).release();
    if( !m_techFactory ) {
      ATH_MSG_FATAL( "Failed to load plugin for " << nam << " storage type" );
    }
  }
  return m_techFactory;
}
