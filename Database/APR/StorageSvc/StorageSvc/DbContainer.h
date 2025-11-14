/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbContainer handle definitions
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBCONTAINER_H
#define POOL_DBCONTAINER_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbHandleBase.h"
#include "StorageSvc/pool.h"
#include "StorageSvc/Transaction.h"

#include <cstdint>

/*
 * POOL namespace declaration
 */
namespace pool  {

  // Forward declarations
  class IDbContainer;
  class DbSelect;
  class DbDatabase;
  class DbTypeInfo;
  class DbContainerObj;
  class DbOption;

  typedef const class Shape        *ShapeH;

  /** @class DbContainer DbContainer.h StorageSvc/DbContainer.h
    *
    * Description:
    * Definition of the object describing a database container
    * Handle managing a DbContainerObj, which is a container of persistent
    * objects.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class DbContainer : public DbHandleBase<DbContainerObj>  {
  private:
    /// Assign transient object properly (including reference counting)
    void switchPtr( DbContainerObj* obj);

  public:
    /// Constructor with initializing arguments
    explicit DbContainer(const DbType& typ=POOL_StorageType) { m_type=typ;          }
    /// Copy constructor
    DbContainer(const DbContainer& c) : Base()  { switchPtr(c.m_ptr);      }
    /// Constructor taking transient object
    DbContainer (DbContainerObj* ptr)           { switchPtr(ptr);          }
    /// Standard Destructor
    virtual ~DbContainer()                      { switchPtr(0);            }
    /// Assignment operator
    DbContainer& operator=(const DbContainer& copy) {
      if ( &copy != this )  {
        m_type = copy.m_type;
        switchPtr( copy.m_ptr );
      }
      return *this;
    }
    /// Assignment operator to reset the handle easily using 0
    DbContainer& operator=(const int /* nuller */ ) {
      switchPtr(0);
      return *this;
    }
    /// Access reference counter
    int refCount() const;
    /// Name of the container the handle is supposed to point to
    const std::string& name() const;
    /// Mode of the handle (READ,WRITE,...)
    DbAccessMode openMode() const;
    /// Access to the size of the container
    uint64_t size();
    /// Access to the Database the container resides in
    DbDatabase& containedIn();
    /// Let the implementation access the internals
    const IDbContainer* info()  const;
    IDbContainer* info();
    /// Retrieve persistent type information by name
    const DbTypeInfo* objectShape(const Guid& nam);
    /// Access the token of the container object
    const Token* token() const;
    /// Close the container the handle points to
    DbStatus close();

    /// Open the container residing in \<file\> with given name and access mode
    /** @param   dbH     [IN]    Valid handle to database object
      * @param   nam     [IN]    Name of the container to be opened.
      * @param   typ     [IN]    Type information in the event the container
      *                          must be created.
      * @param   dbtyp   [IN]    Database type (including minor type)
      * @param   mod     [IN]    Access mode.
      *
      * @return Status code indicating success or failure.
      */
    DbStatus open(DbDatabase&        dbH, 
                  const std::string&  nam, 
                  const DbTypeInfo*   typ, 
                  const DbType&       dbtyp,
                  DbAccessMode        mod);

    /// Check if we can access the residing in \<file\> container for reading with the given type
    /** @param   dbH     [IN]    Valid handle to database object
      * @param   nam     [IN]    Name of the container to be opened.
      * @param   dbtyp   [IN]    Database type (including minor type)
      *
      * @return Status code indicating success or failure.
      */
    DbStatus checkAccess(DbDatabase&        dbH,
                         const std::string& nam,
                         const DbType&      dbtyp);

    /// Check if the container was opened
    bool isOpen() const;
    /// Execute Database Transaction Action
    DbStatus transAct(Transaction::Action action);
    /// Pass options to the implementation
    DbStatus setOption(const DbOption& refOpt);
    /// Access options
    DbStatus getOption(DbOption& refOpt);

    /** Access objects through select staements.                            
      * This access type is ideal for relational Databases.
      * Other technologies only have very limited support for this
      * interface.
      */
    //@{ 
    /// Perform selection. The statement belongs to the container afterwards.
    DbStatus select(DbSelect& sel);
    /// Fetch next object address of the selection to set token
    DbStatus fetch(DbSelect& sel);
    //@}

    /** Access objects using pointer and shape
      */
    //@{
    /// In place allocation of object location
    DbStatus allocate(const void* object, ShapeH shape, Token::OID_t& oid);
    /// Select object in the container identified by its handle
    DbStatus load(void** ptr, ShapeH shape, const Token::OID_t& lH);
    //@}

    /** Access objects by handle directly.
        This is the generic "direct" object access.
    */
    //@{
    /// Store object in location
    DbStatus store(const void* object, const DbTypeInfo* typ);
    //@}
  };
}       // End namespace pool

#endif  // POOL_DBCONTAINER_H
