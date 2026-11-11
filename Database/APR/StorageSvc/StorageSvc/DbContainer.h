/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_DBCONTAINER_H
#define STORAGESVC_DBCONTAINER_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbHandleBase.h"
#include "StorageSvc/pool.h"

#include <cstdint>

class StatusCode;

// Forward declarations
class ITechContainer;
class DbDatabase;
class DbTypeInfo;
class DbContainerObj;
class DbOption;
class Shape;

/** @class DbContainer DbContainer.h StorageSvc/DbContainer.h
    *
    * Description:
    * This class is a handle managing access and lifetime for the Container implementation
    * object: DbContainerObj
    *
    * @author  M.Frank
    */
class DbContainer : public DbHandleBase<DbContainerObj>  {
  private:
    /// Assign transient object properly (including reference counting)
    void switchPtr( DbContainerObj* obj);

  public:
    /// Constructor with initializing arguments
    explicit DbContainer(const DbType& typ=pool::POOL_StorageType) { m_type=typ;          }
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
    Io::IoFlag openMode() const;
    /// Access to the size of the container
    uint64_t size();
    /// Access to the Database the container resides in
    DbDatabase& containedIn();
    /// Let the implementation access the internals
    const ITechContainer* info()  const;
    ITechContainer* info();
    /// Retrieve persistent type information by name
    const DbTypeInfo* objectShape(const Guid& nam);
    /// Access the token of the container object
    const Token* token() const;
    /// Close the container the handle points to
    StatusCode close();

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
    StatusCode open(DbDatabase&         dbH,
                    const std::string&  nam,
                    const DbTypeInfo*   typ,
                    const DbType&       dbtyp,
                    Io::IoFlag        mod);

    /// Check if we can access the residing in \<file\> container for reading with the given type
    /** @param   dbH     [IN]    Valid handle to database object
      * @param   nam     [IN]    Name of the container to be opened.
      * @param   dbtyp   [IN]    Database type (including minor type)
      *
      * @return Status code indicating success or failure.
      */
    StatusCode checkAccess(DbDatabase&        dbH,
                           const std::string& nam,
                           const DbType&      dbtyp);

    /// Check if the container was opened
    bool isOpen() const;
    /// Execute Database Transaction Action
    StatusCode transAct(Transaction::Action action);
    /// Pass options to the implementation
    StatusCode setOption(const DbOption& refOpt);
    /// Access options
    StatusCode getOption(DbOption& refOpt);

    /** Access objects through select staements.                            
      * This access type is ideal for relational Databases.
      * Other technologies only have very limited support for this
      * interface.
      */
    //@{ 
    /// Fetch next object address to set token
    StatusCode next(Token::OID_t& linkH);
    //@}

    /** Access objects using pointer and shape
      */
    //@{
    /// In place allocation of object location
    StatusCode allocate(const void* object, const Shape* shape, Token::OID_t& oid);
    /// Select object in the container identified by its handle
    StatusCode load(void** ptr, const Shape* shape, const Token::OID_t& lH);
    //@}

    /** Access objects by handle directly.
        This is the generic "direct" object access.
    */
    //@{
    /// Store object in location
    StatusCode store(const void* object, const DbTypeInfo* typ);
    //@}
};

#endif
