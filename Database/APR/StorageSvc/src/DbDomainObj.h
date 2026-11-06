/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDomainObj object definition
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBDOMAINOBJ_H
#define POOL_DBDOMAINOBJ_H 1

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbPrint.h"
#include "DbAccessObj.h"

class ITechFactory;

/*
 *  POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class DbDatabaseObj;
  class DbOption;
  class ITechDomain;

  /** Db objects: DbDomainObj

      Description:
      Implementation independent part of a Database domain object
      objects.

      @author  M.Frank
      @version 1.0
  */
  class DbDomainObj : public  DbAccessObj<std::string, DbDatabaseObj >, public APRMessaging  {
  private:
    /// Maximal age of files allowed.
    int             m_maxAge;
    /// Technology dependent stuff
    ITechDomain*      m_info;
  public:
    /// Constructor
    DbDomainObj(ITechFactory* imp, const DbType& typ, Io::IoFlag mode = Io::READ);
    /// Standard destructor
    virtual ~DbDomainObj();
    /// Access to technology dependent implementation
    ITechDomain* info()                 {    return m_info;     }
    const ITechDomain* info() const     {    return m_info;     }
    /// Set the maximal allowed age limit for files in this domain
    void       setAgeLimit(int value)   {    m_maxAge = value;  }
    /// Access the maximal age limit
    int        ageLimit()  const        {    return m_maxAge;   }
    /// Check for Database existence within domain
    bool existsDbase(const std::string& nam);
    /// Open domain with possible change of access mode
    StatusCode open(Io::IoFlag mode);
    /// Open domain in default access mode
    StatusCode open();
    /// Close domain
    StatusCode close();
    /// Increase the age of all open databases
    StatusCode ageOpenDbs();
    /// Check if databases are present, which aged a lot and need to be closed
    StatusCode closeAgedDbs();
    /// Set domain specific options
    /** @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode indicating success or failure.
      */
    StatusCode setOption(const DbOption& refOpt);
    /// Access domain specific options
    /** @param refOpt   [IN]  Reference to option object
      *
      * @return StatusCode indicating success or failure.
      */
    StatusCode getOption(DbOption& refOpt) const;
  };
}      // End namespace pool
#endif // POOL_DBDOMAINOBJ_H
