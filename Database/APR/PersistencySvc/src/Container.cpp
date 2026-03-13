/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
  m_name( name ),
  m_fileDescriptor( fileDescriptor ),
  m_technology( technology )
{}

pool::ITokenIterator*
pool::PersistencySvc::Container::tokens()
{
  return new pool::PersistencySvc::TokenIterator( m_fileDescriptor,
                                                  this->name() );
}

bool
pool::PersistencySvc::Container::attributeOfType( const std::string& attributeName,
                                                  void* data,
                                                  const std::type_info& typeInfo,
                                                  const std::string& option )
{
  if( !m_fileDescriptor.dbc() ) return false;

  pool::DbOption containerOption( attributeName, option );
  
  DbDatabase  dbH( m_fileDescriptor.dbc()->handle() );
  DbContainer cntH( dbH.find(name()) );
  if( !cntH.isValid() and !cntH.open(dbH, name(), 0, dbH.type(), pool::READ).isSuccess() ) {
    return false;
  }
  if( !cntH.getOption(containerOption).isSuccess() ) return false;
  return containerOption.i_getValue(typeInfo, data).isSuccess();
}


bool
pool::PersistencySvc::Container::setAttributeOfType( const std::string& attributeName,
                                                     const void* data,
                                                     const std::type_info& typeInfo,
                                                     const std::string& option )
{
  if( !m_fileDescriptor.dbc() ) return false;

  pool::DbOption containerOption( attributeName, option );
  if( !containerOption.i_setValue(typeInfo, const_cast<void*>( data )).isSuccess() ) return false;
  
  DbDatabase  dbH( m_fileDescriptor.dbc()->handle() );
  DbContainer cntH( dbH.find(name()) );
  if( !cntH.isValid() and !cntH.open(dbH, name(), 0, dbH.type(), pool::READ).isSuccess() ) {
    return false;
  }
  return cntH.setOption(containerOption).isSuccess();
}
