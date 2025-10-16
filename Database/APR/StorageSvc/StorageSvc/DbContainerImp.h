/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//  Package    : StorageSvc (The POOL project)
//
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBCONTAINERIMP_H
#define POOL_DBCONTAINERIMP_H 1

/// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/IDbContainer.h"

// STL include files
#include <map>
#include <vector>
#include <utility>

/*
 * POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class IDbContainer;

  /** @class DbContainerImp DbContainerImp.h StorageSvc/DbContainerImp.h
    *
    *  "Generic" Container implementation
    *
    *  Description: Generic helper class to implement stuff common to all
    *  existing Database containers. The base implementations can allways 
    *  be overwritten.
    *
    *  @author  M.Frank
    *  @version 1.0
    */
  class DbContainerImp : virtual public IDbContainer
  {
  protected:

    /// List of actions to execute at commit
    struct DbAction {
      const void*         object;
      const Shape*        shape;
      Token::OID_t        link;
      AccessMode          action;

      DbAction() : object(nullptr), shape(nullptr), action(NONE)   { }
      DbAction(const void* obj, const Shape* s, const Token::OID_t&  l, AccessMode a)
            : object(obj), shape(s), link(l), action(a)   { }

      const void*       dataAtOffset(size_t offset) {
         return static_cast<const char*>(object) + offset;
      }
    };
    
    typedef std::vector< DbAction > ActionList;
    
  private:
    /// Transaction fifo storage for writing
    ActionList            m_stack;
    /// Current size of the transaction stack
    size_t                m_size;
    /// Number of objects to be written out during open transaction
    size_t                m_writeSize;
    /// Accumulated stack entry types
    int                   m_stackType;
  protected:
    /// Container name
    std::string           m_name;
    /// Flag to indicate if object updates are supported
    bool                  m_canUpdate;
    /// Flag to indicate if object removals are supported
    bool                  m_canDestroy;

    /// Standard destructor
    virtual ~DbContainerImp();
    /// Access accumulated stack entry types
    int stackType()   const
    { return m_stackType;                                                     }
    /// Access stack size
    size_t stackSize()  const   
    { return m_size;                                                          }
    /// Internal: get access to stack entry
    ActionList::value_type* stackEntry(size_t which) 
    { return (which <= m_size) ? &(*(m_stack.begin()+which)) : 0;             }
    /// Commit single entry to container
    virtual DbStatus writeObject(ActionList::value_type& /* entry */)  
    { return Error;                                                   }
    /// Execute object modification requests during a transaction
    virtual DbStatus commitTransaction();

  public:
    DbContainerImp();
    /// Release instance (Abstract interfaces do not expose destructor!)
    virtual void release() override                    { delete this;           }
    /// Size of the container
    virtual uint64_t size() override;
    /// Get container name
    virtual std::string name() const override
    { return m_name; }
    /// Number of next record in the container (=size if no delete is allowed)
    virtual uint64_t nextRecordId() override;
    /// Suggest next Record ID for tbe next object written - used only with synced indexes
    virtual void useNextRecordId(uint64_t) override {};
    /// Close the container and deallocate resources
    virtual DbStatus close() override;

    /// Access options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus getOption(DbOption& opt) override;

    /// Set options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus setOption(const DbOption& opt) override;

    /// Execute Transaction Action
    virtual DbStatus transAct(Transaction::Action) override;
    /// In place allocation of raw memory for the transient object
    virtual void* allocate(   unsigned long siz, 
                              DbContainer&  cntH,
                              ShapeH shape) override;
    /// In place allocation of object location
    virtual DbStatus allocate(DbContainer& cntH, 
                              const void* object,
                              ShapeH shape,
                              Token::OID_t& oid) override;
    /// In place deletion of raw memory
    virtual DbStatus free(    void* ptr,
                              DbContainer& cntH) override;
    /// Fetch next object address of the selection to set token
    virtual DbStatus fetch(DbSelect&      sel) override;
    /// Add single entry to container
    virtual DbStatus save(  DbObjectHandle<DbObject>& objH) override;

    /// Save new object in the container and return its handle
    /** @param  cntH      [IN]   Handle to container object.
      * @param  object    [IN]   Data object
      * @param  linkH     [OUT]  Internal OID to identify object.
      * @return DbStatus code indicating success or failure.
      */
    virtual DbStatus save(DbContainer&  cntH,
                          const void* object,
                          ShapeH shape,
                          Token::OID_t& linkH) override;

    /// Find object within the container and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  linkH     [IN]   Preferred object OID
      * @param  oid      [OUT]   Actual object OID
      * @param  any_next  [IN]   On selection, objects may be skipped.
      *                          If objects are skipped, the actual oid
      *                          will differ from the preferred oid.
      * @return Status code indicating success or failure.
      */
    virtual DbStatus load( void** ptr, ShapeH shape, 
                           const Token::OID_t& lnkH, 
                           Token::OID_t&       oid,
                           bool                any_next) override;
    /// Clear Transaction stack containing transaction requests
    virtual DbStatus clearStack();
    /// Fetch refined object address. Default implementation returns identity
    virtual DbStatus fetch(const Token::OID_t& linkH, Token::OID_t& stmt);

    /// Find object by object identifier and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  oid      [OUT]   Object OID
      *
      * @return Status code indicating success or failure.
      */
    virtual DbStatus loadObject(void** ptr, ShapeH shape, Token::OID_t& oid) = 0;

  };
}       // End namespace pool
#endif  // POOL_DBCONTAINERIMP_H
