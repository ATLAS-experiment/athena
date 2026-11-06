/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_TECHFACTORY_H
#define STORAGESVC_TECHFACTORY_H

// Framework include files
#include "StorageSvc/pool.h"

// Forward declarations
namespace pool    
{
  class ITechDomain;
  class ITechDatabase;
  class ITechContainer;
  class DbType;
}
using pool::ITechDomain;
using pool::ITechDatabase;
using pool::ITechContainer;


/** @class ITechFactory interface definition.
    *
    * <P> The ITechFactory interface allows to:
    * <UL>
    * <LI> Create Database domains.
    * <LI> Create Database files.
    * <LI> Create Database containers.
    * </UL>
    *
    * @author  Markus Frank
    */
class ITechFactory : public pool::RefCounter  {
   public:  
    virtual ~ITechFactory() = default;

   public:
    /// Name of the Database implementation
    virtual const std::string&  name () const = 0;

    /// Create Database domain
    virtual ITechDomain* createDomain(void) = 0;
    /// Create Database
    virtual ITechDatabase* createDatabase(void) = 0;
    /// Create Database container
    virtual ITechContainer* createContainer(const std::string& name, const pool::DbType& typ) = 0;

    ///  delete object if ref count drops to 0
    void release() { if( pool::RefCounter::subRef() <= 0 ) delete this; }
};


#endif
