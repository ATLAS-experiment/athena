/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbContainerObj class definitions
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBCONTAINEROBJ_H
#define POOL_DBCONTAINEROBJ_H 1

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbAccessObj.h"
#include "StorageSvc/DbContainer.h"
#include "POOLCore/DbPrint.h"
/*
 *  POOL namespace declaration
 */
namespace pool  {

  typedef const class Shape        *ShapeH;

  /** @class DbContainerObj DbContainerObj.h src/DbContainerObj.h
    *
    * Description:
    *
    * Implementation independent part of a container object
    * objects.
    *
    * There is a ring of protection around the object. The object can
    * only be accessed through its handle, the technology dependent 
    * code and the DbDatabaseObj object hosting the container.
    * This should ensure proper reference counting and inhibit non
    * existing references flying around.
    *
    * @author  M.Frank
    * @version 1.0
    */
  struct DbObjectHolder  {
    DbObject* m_obj;
    explicit DbObjectHolder(DbObject* p) : m_obj(p) {}
    int release();
  };
  class DbContainerObj : public  DbAccessObj<DbObject*, DbObjectHolder >, public APRMessaging {
  private:
    /// Pointer to interface of the technology dependent part
    IDbContainer*                 m_info;
    /// Container token
    const Token*                  m_tokH;
    /// Handle to hosting Database
    DbDatabase                    m_dbH;
    /// Flag indication StatusCode of technology dependent container
    bool                          m_isOpen;

    /// Check database access
    bool hasAccess();
  public:
    /// Standard constructor of a container object using the Database handle as a clustering hint
    /** @param   dbH     [IN]    Valid handle to database object
      * @param   nam     [IN]    Name of the container to be opened.
      * @param   dbtyp   [IN]    Database type (including minor type)
      * @param   mod     [IN]    Access mode.
      *
      * @return Status code indicating success or failure.
      */
    DbContainerObj( DbDatabase&        dbH,
                    const std::string& nam,
                    const DbType&      dbtyp,
                    DbAccessMode       mod);
    /// Standard destructor
    virtual ~DbContainerObj();
    /// Access to internals
    const IDbContainer* info()  const       {  return m_info;         }
    IDbContainer* info()                    {  return m_info;         }
    /// Retrieve persistent type information by name
    const DbTypeInfo* objectShape(const Guid& nam);
    /// Handle to Database (CONST)
    DbDatabase& database()                  {  return m_dbH;          }
    /// Access the token of the container object
    const Token* token() const              {  return m_tokH;         }
    /// Flag if container was opened
    bool isOpen() const                     {  return m_isOpen;       }
    /// Check if database is in read-only mode
    bool isReadOnly() const      
    { return !(mode()&pool::UPDATE) && !(mode()&pool::CREATE);    }
    /// Cancel transaction flag
    void cancelTransaction()                { }
    /// Size of the Database container (=# of objects)
    uint64_t size();
    /// Open the container
    StatusCode open(const DbTypeInfo* typ);
    /// Check if we can access the container
    StatusCode checkAccess();
    /// Close the container
    StatusCode close();
    /// Retire the container
    StatusCode retire();
    /// Execute Database Transaction Action
    StatusCode transAct(Transaction::Action);
    /// Pass options to the implementation
    StatusCode setOption(const DbOption& opt);
    /// Access options
    StatusCode getOption(DbOption& refOpt);

    //@{

    /// Store object in location
    StatusCode store(const void* object, DbContainer& cntH, ShapeH shape);

    /// In place allocation of object location
    StatusCode allocate(DbContainer& cntH, const void* object, ShapeH shape, Token::OID_t& oid);

    /// Select object in the container identified by its handle
    StatusCode load( void** ptr, ShapeH shape,
                     const Token::OID_t& linkH,
                     Token::OID_t&       oid,
                     bool          any_next);
    //@}

    /// Fetch next object address to set token
    StatusCode next(Token::OID_t& linkH);
  };
}       // End namespace pool
#endif  // POOL_DBCONTAINEROBJ_H
