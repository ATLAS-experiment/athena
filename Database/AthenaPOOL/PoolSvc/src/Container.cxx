/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Container.h"
#include "TokenIterator.h"

#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbConnection.h"

pool::Container::Container( FileDescriptor& fileDescriptor, const std::string& name ) :
  m_name( name ),
  m_fileDescriptor( fileDescriptor )
{}

pool::ITokenIterator*
pool::Container::tokens()
{
  return new pool::TokenIterator( m_fileDescriptor, this->name() );
}

bool
pool::Container::attributeOfType( const std::string& attributeName,
                                  void* data,
                                  const std::type_info& typeInfo,
                                  const std::string& option )
{
  if( !m_fileDescriptor.dbc() ) return false;

  pool::DbOption containerOption( attributeName, option );
  
  DbDatabase  dbH( m_fileDescriptor.dbc()->handle() );
  DbContainer cntH( dbH.find(name()) );
  if( !cntH.isValid() && !cntH.open(dbH, name(), 0, dbH.type(), Io::READ).isSuccess() ) {
    return false;
  }
  if( !cntH.getOption(containerOption).isSuccess() ) return false;
  return containerOption.i_getValue(typeInfo, data).isSuccess();
}


bool
pool::Container::setAttributeOfType( const std::string& attributeName,
                                     const void* data,
                                     const std::type_info& typeInfo,
                                     const std::string& option )
{
  if( !m_fileDescriptor.dbc() ) return false;

  pool::DbOption containerOption( attributeName, option );
  if( !containerOption.i_setValue(typeInfo, const_cast<void*>( data )).isSuccess() ) return false;
  
  DbDatabase  dbH( m_fileDescriptor.dbc()->handle() );
  DbContainer cntH( dbH.find(name()) );
  if( !cntH.isValid() && !cntH.open(dbH, name(), 0, dbH.type(), Io::READ).isSuccess() ) {
    return false;
  }
  return cntH.setOption(containerOption).isSuccess();
}
