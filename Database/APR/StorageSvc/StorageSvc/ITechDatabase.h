/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

  /** @class ITechDatabase ITechDatabase.h StorageSvc/ITechDatabase.h
    *
    * Description:
    * Interface to the technology specific implementation of a Database object
    *
    * @author  M.Frank
    */
  class ITechDatabase    {
  public:
    virtual ~ITechDatabase() = default;

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
    virtual StatusCode close(Io::IoFlag mode)  = 0;

    /// Open Database object
    /** @param dbH      [IN]  StorageSvc handle to the database being opened.
      * @param nam      [IN]  Name of the database to be opened.
      * @param mode     [IN]  Desired session access mode.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode open(const DbDatabase&   dbH, 
                            const std::string&  nam, 
                            Io::IoFlag          mode)  = 0;

    /// Execute Database Transaction action
    /** @param   action     [IN]  action to perform
      * @return Status code indicating success or failure.
      */
    virtual StatusCode transAct(Transaction::Action action) = 0;
 };
}      // End namespace pool
#endif // POOL_IDBDATABASE_H
