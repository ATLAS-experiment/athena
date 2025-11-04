/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//  ====================================================================
//
//  DbStorageExplorer.cpp
//  --------------------------------------------------------------------
//
//  Package   : StorageSvc (POOL)
//  Author    : Markus Frank
//
//  ====================================================================

// Framework include files
#include "DbStorageSvc.h"
#include "DbStorageExplorer.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/FileDescriptor.h"
#include "POOLCore/DbPrint.h"

#include <memory>
#include <stdexcept>
using namespace std;
using namespace pool;

typedef const DbDatabaseObj *DbDatabaseH;
typedef       DbDatabaseObj *DbDatabaseHNC;

static DbDatabaseHNC __DB(FileDescriptor&     fDesc)  {
  DbDatabaseHNC handle = DbDatabaseHNC(fDesc.dbc()->handle());
  return handle;
}

/// Standard Constructor.
DbStorageExplorer::DbStorageExplorer( const string& nam,
                                      DbDomain& domH,
                                      DbStorageSvc* pOuter)
    : AthMessaging(nam), m_pOuter(pOuter), m_domH(domH), m_name(nam)
{
  if ( 0 == m_pOuter )  {
    string msg = "DbStorageExplorer> "
                      "DbStorageExplorer components can only be aggregated"
                      " -- No outer interface defined!";
    throw runtime_error(msg);
  }
}

/// Standard destructor.
DbStorageExplorer::~DbStorageExplorer() 
{
}

DbStatus DbStorageExplorer::queryInterface(const Guid& riid,void** ppvInterface)  {
  if ( riid == IStorageExplorer::interfaceID() )  {
    *ppvInterface = static_cast<IStorageExplorer*>(this);
    m_pOuter->addRef();
    return Success;
  }
  return m_pOuter->queryInterface(riid, ppvInterface);
}

//--- IInterface::addRef
unsigned int DbStorageExplorer::addRef()  {
  return m_pOuter->addRef();
}

//--- IInterface::release
unsigned int DbStorageExplorer::release() {
  return m_pOuter->release();
}

/// Start a new Database Session.
DbStatus 
DbStorageExplorer::startSession(int accessmode,int technology,SessionH& refSession) {
  return m_pOuter->startSession(accessmode, technology, refSession);
}

/// End the Database session.
DbStatus DbStorageExplorer::endSession(const SessionH session) {
  return m_pOuter->endSession(session);
}

/// Connect to a logical Database unit.
DbStatus 
DbStorageExplorer::connect(const SessionH session,int mod,FileDescriptor& fDesc)  {
  return m_pOuter->connect(session, mod, fDesc);
}

/// Disconnect from a logical Database unit.
DbStatus DbStorageExplorer::disconnect(FileDescriptor& fDesc) {
  return m_pOuter->disconnect(fDesc);
}

/// Access the containers in a given database.
DbStatus 
DbStorageExplorer::containers(FileDescriptor& refDB,TokenVec& conts,bool intern)  {
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    return dbH.containers(conts, intern);
  }
  return Error;
}

/// Retrieve existing parameter by name
DbStatus
DbStorageExplorer::dbParam(FileDescriptor& refDB,const string& nam,string& val) {
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    return dbH.param(nam, val);
  }
  return Error;
}

/// Access options for a given database domain.
DbStatus DbStorageExplorer::getDomainOption( const SessionH  sessionH, DbOption& opt) {
  return m_pOuter->getDomainOption(sessionH, opt);
}

/// Set options for a given database domain.
DbStatus DbStorageExplorer::setDomainOption( const SessionH  sessionH, const DbOption& opt) {
  return m_pOuter->setDomainOption(sessionH, opt);
}

/// Access options for a given database.
DbStatus DbStorageExplorer::getDatabaseOption(FileDescriptor& refDB,DbOption& opt)  {
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    return dbH.getOption(opt);
  }
  return Error;
}

/// Set options for a given database.
DbStatus
DbStorageExplorer::setDatabaseOption(FileDescriptor& refDB,const DbOption& opt) {
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    return dbH.setOption(opt);
  }
  return Error;
}

/// Access options for a given database.
DbStatus 
DbStorageExplorer::getContainerOption(FileDescriptor& refDB,const string& cntName,DbOption& opt)
{
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    DbContainer cntH(dbH.find(cntName));
    if ( !cntH.isValid() )  {
      cntH.open(dbH, cntName, 0, dbH.type(), pool::READ);
    }
    return cntH.getOption(opt);
  }
  ATH_MSG_ERROR( "The container " << cntName << " cannot be accessed." << endmsg
              << " In order to access options the container must have been opened." );
  return Error;
}

/// Set options for a given database.
DbStatus DbStorageExplorer::setContainerOption(FileDescriptor& refDB,
                                               const string& cntName,
                                               const DbOption& opt)
{
  const DbConnection* dbc = dynamic_cast<const DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase  dbH(__DB(refDB));
    DbContainer cntH(dbH.find(cntName));
    if ( !cntH.isValid() )  {
      cntH.open(dbH, cntName, 0, dbH.type(), pool::READ);
    }
    return cntH.setOption(opt);
  }
  ATH_MSG_ERROR( "The container " << cntName << " cannot be accessed." << endmsg
              << " In order to access options the container must have been opened." );
  return Error;
}

/// Initiate a selection on a given container.
DbStatus DbStorageExplorer::select(FileDescriptor&       refDB,
                                   const string&         cntName,
                                   DbSelect&             sel)
{
  DbConnection* dbc = dynamic_cast<DbConnection*>(refDB.dbc());
  if ( dbc )   {
    DbDatabase dbH(DbDatabaseHNC(dbc->handle()));
    if ( dbH.isValid() )  {
      return sel.start(dbH, cntName);
    }
  }
  ATH_MSG_ERROR( "Cannot scan container. Invalid database handle." );
  return Error;
}

/// Access the next token of a sequence
DbStatus DbStorageExplorer::next(DbSelect& context, Token*& refpToken)  {
  return context.next(refpToken);
}
