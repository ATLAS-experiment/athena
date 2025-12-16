/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "Container.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbConnection.h"
#include "TokenIterator.h"

pool::PersistencySvc::Container::Container( FileDescriptor& fileDescriptor,
                                            long technology,
                                            const std::string& name ):
  pool::IContainer( name ),
  m_fileDescriptor( fileDescriptor ),
  m_technology( technology )
{}

pool::PersistencySvc::Container::~Container()
{}

pool::ITokenIterator*
pool::PersistencySvc::Container::tokens()
{
  return new pool::PersistencySvc::TokenIterator( m_fileDescriptor,
                                                  this->name() );
}

const std::string&
pool::PersistencySvc::Container::parentDatabaseName() const
{
  return m_fileDescriptor.FID();
}

long
pool::PersistencySvc::Container::technology() const
{
  return m_technology;
}

const pool::ITechnologySpecificAttributes&
pool::PersistencySvc::Container::technologySpecificAttributes() const
{
  return static_cast< const pool::ITechnologySpecificAttributes& >( *this );
}

pool::ITechnologySpecificAttributes&
pool::PersistencySvc::Container::technologySpecificAttributes()
{
  return static_cast< pool::ITechnologySpecificAttributes& >( *this );
}

bool
pool::PersistencySvc::Container::attributeOfType( const std::string& attributeName,
                                                  void* data,
                                                  const std::type_info& typeInfo,
                                                  const std::string& option )
{
  pool::DbOption containerOption( attributeName, option );
  DbConnection* dbc = dynamic_cast<DbConnection*>(m_fileDescriptor.dbc());
  if ( !dbc ) {
     return false;
  }
  DbDatabase  dbH( static_cast<DbDatabaseObj*>(dbc->handle()));
  DbContainer cntH(dbH.find(this->name()));
  if ( !cntH.isValid() )  {
    cntH.open(dbH, this->name(), 0, dbH.type(), pool::READ);
  }
  pool::DbStatus sc = cntH.getOption(containerOption);
  if ( !sc.isSuccess() ) return false;
  if ( containerOption.i_getValue( typeInfo, data ).isSuccess() ) {
    return true;
  }
  else {
    return false;
  }
}


bool
pool::PersistencySvc::Container::setAttributeOfType( const std::string& attributeName,
                                                     const void* data,
                                                     const std::type_info& typeInfo,
                                                     const std::string& option )
{
  pool::DbOption containerOption( attributeName, option );
  pool::DbStatus sc = containerOption.i_setValue( typeInfo, const_cast<void*>( data ) );
  if ( !sc.isSuccess() ) return false;
  DbConnection* dbc = dynamic_cast<DbConnection*>(m_fileDescriptor.dbc());
  if ( !dbc ) {
     return false;
  }
  DbDatabase  dbH( static_cast<DbDatabaseObj*>(dbc->handle()));
  DbContainer cntH(dbH.find(this->name()));
  if ( !cntH.isValid() )  {
    cntH.open(dbH, this->name(), 0, dbH.type(), pool::READ);
  }
  sc = cntH.setOption(containerOption);
  if ( sc.isSuccess() ) {
    return true;
  }
  else {
    return false;
  }
}
