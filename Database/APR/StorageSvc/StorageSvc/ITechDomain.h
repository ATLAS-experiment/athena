/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef POOL_ITECHDOMAIN_H
#define POOL_ITECHDOMAIN_H

// Framework include files
#include "StorageSvc/pool.h"

class StatusCode;

/*
 *  POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class DbSession;
  class DbOption;
  class DbType;

  /** @class IDbDomain IDbDomain.h StorageSvc/IDbDomain.h
    * 
    * Description:
    * Interface to the technology specific implementation of the Database Domain object
    *
    * @author  M.Frank
    */
  class ITechDomain   {
  public:
    virtual ~ITechDomain() = default;
    
    /// Set domain specific options
    /** @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode setOption(const DbOption& refOpt) = 0;

    /// Access domain specific options
    /** @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode getOption(DbOption& refOpt) const = 0;
  };
}      // End namespace pool
#endif // POOL_IDBDOMAIN_H
