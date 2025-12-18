/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//  Package    : pool/StorageSvc (The pool framework)
//
//  @author      M.Frank
//
//====================================================================
#ifndef POOL_IDBDATABASE_H
#define POOL_IDBDATABASE_H

// Framework include files
#include "StorageSvc/pool.h"
#include "StorageSvc/Transaction.h"

class StatusCode;

/*
 *  POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class DbOption;
  class DbDomain;
  class DbDatabase;
  class DbContainer;

  /** @class IDbDatabase IDbDatabase.h StorageSvc/IDbDatabase.h
    *
    * IDbDatabase interface
    *
    * Description:
    * Interface to the implementation specific part of a Database object
    *
    * @author  M.Frank
    * @version 1.0
    */
  class IDbDatabase    {
  public:
    virtual ~IDbDatabase()   { }

    /// Access the size of the database: May be undefined for some technologies
    virtual long long int size()  const = 0;

    /// Set options
    /** @param refOpt   [IN]  Reference to option object.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode setOption(const DbOption& refOpt) = 0;

    /// Access options
    /** @param refOpt   [IN]  Reference to option object.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode getOption(DbOption& refOpt) = 0;

    /// Close database access
    /** @param mode     [IN]  Desired session access mode.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode close(DbAccessMode mode)  = 0;

    /// Open Database object
    /** @param domH     [IN]  Handle to valid domain object
      *                       (validity ensured by upper levels).
      * @param nam      [IN]  Name of the database to be opened.
      * @param mode     [IN]  Desired session access mode.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode open(const DbDomain&     domH, 
                          const std::string&  nam, 
                          DbAccessMode        mode)  = 0;

    /// Callback after successful open of a database object
    /** @param dbH      [IN]  Handle to valid database object
      * @param mode     [IN]  Desired session access mode.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode onOpen(DbDatabase& dbH, 
                            DbAccessMode      mode) = 0;

    /// Re-open database with changing access permissions
    /** @param mode     [IN]  Desired session access mode.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode reopen(DbAccessMode mode) = 0;

    /// Execute Database Transaction action
    /** @param   action     [IN]  action to perform
      * @return Status code indicating success or failure.
      */
    virtual StatusCode transAct(Transaction::Action action) = 0;
 };
}      // End namespace pool
#endif // POOL_IDBDATABASE_H
