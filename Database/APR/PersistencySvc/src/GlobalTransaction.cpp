/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalTransaction.h"
#include "DatabaseHandler.h"

pool::PersistencySvc::GlobalTransaction::GlobalTransaction( pool::PersistencySvc::DatabaseRegistry& registry ) :
  pool::APRMessaging("APR/PersistencySvc"),
  m_type( pool::ITransaction::UNDEFINED ),
  m_databases( registry )
{}


pool::PersistencySvc::GlobalTransaction::~GlobalTransaction()
{
}


bool
pool::PersistencySvc::GlobalTransaction::start( pool::ITransaction::Type type )
{
  if ( this->isActive() || type == pool::ITransaction::UNDEFINED ) return false;
  m_type = type;
  return true;
}


bool
pool::PersistencySvc::GlobalTransaction::commit()
{
  if ( this->isActive() ) {
    bool OK = true;
    for ( pool::PersistencySvc::DatabaseRegistry::iterator iDb = m_databases.begin();
          iDb != m_databases.end(); ++iDb ) {
      bool bCommit = (*iDb)->commitTransaction(); // This has to be replaced with a two phase commit
      if ( ! bCommit ) {
        ATH_MSG_ERROR("Could not commit the transaction for the database with:" << endmsg
                      << "FID = " << (*iDb)->fid() << endmsg
                      << "PFN = " << (*iDb)->pfn() );
      }
      OK = OK && bCommit;
    }
    m_type = pool::ITransaction::UNDEFINED;
    return OK;
  }
  return false;
}


bool
pool::PersistencySvc::GlobalTransaction::commitAndHold()
{
  if ( this->isActive() ) {
    bool OK = true;
    for ( pool::PersistencySvc::DatabaseRegistry::iterator iDb = m_databases.begin();
          iDb != m_databases.end(); ++iDb ) {
      bool bCommit = (*iDb)->commitAndHoldTransaction(); // This has to be replaced with a two phase commit
      if ( ! bCommit ) {
        ATH_MSG_ERROR("Could not commit and hold the transaction for the database with:" << endmsg
            << "FID = " << (*iDb)->fid() << endmsg
            << "PFN = " << (*iDb)->pfn() );
      }
      OK = OK && bCommit;
    }
    return OK;
  }
  return false;
}


bool
pool::PersistencySvc::GlobalTransaction::isActive() const
{
  if ( m_type == pool::ITransaction::UNDEFINED ) return false;
  return true;
}


void
pool::PersistencySvc::GlobalTransaction::update()
{
  if ( m_type == pool::ITransaction::READ ) {
    m_type = pool::ITransaction::UPDATE;
  }
}


pool::ITransaction::Type
pool::PersistencySvc::GlobalTransaction::type() const
{
  return m_type;
}
