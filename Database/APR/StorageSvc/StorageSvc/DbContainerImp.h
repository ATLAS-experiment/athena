/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "StorageSvc/pool.h"
#include "StorageSvc/IDbContainer.h"
#include "POOLCore/DbPrint.h"
#include "GaudiKernel/StatusCode.h"

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
  class DbContainerImp : virtual public IDbContainer, public APRMessaging
  {
  protected:

    /// List of actions to execute at commit
    struct DbAction {
      const void*         object;
      const Shape*        shape;
      Token::OID_t        link;

      DbAction() : object(nullptr), shape(nullptr) { }
      DbAction(const void* obj, const Shape* s, const Token::OID_t&  l)
            : object(obj), shape(s), link(l) { }

      const void*       dataAtOffset(size_t offset) {
         return static_cast<const char*>(object) + offset;
      }
    };
    
    typedef std::vector< DbAction > ActionList;
    
  private:
    /// Transaction fifo storage for writing
    ActionList            m_writeStack;
    /// Current size of the transaction stack
    size_t                m_size;
  protected:
    /// Container name
    std::string           m_name;

    /// Standard destructor
    virtual ~DbContainerImp();
    /// Commit single entry to container
    virtual StatusCode writeObject(ActionList::value_type& /* entry */)  
    { return StatusCode::FAILURE;                                                   }
    /// Execute object modification requests during a transaction
    virtual StatusCode commitTransaction();

  public:
    explicit DbContainerImp(const std::string& name);
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
    virtual StatusCode close() override;

    /// Access options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode getOption(DbOption& opt) override;

    /// Set options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode setOption(const DbOption& opt) override;

    /// Execute Transaction Action
    virtual StatusCode transAct(Transaction::Action) override;
    /// Store object in location
    virtual StatusCode store(      const void* object,
                                 DbContainer&  cntH,
                                 ShapeH shape) override;
    /// In place allocation of object location
    virtual StatusCode allocate(DbContainer& cntH,
                              const void* object,
                              ShapeH shape,
                              Token::OID_t& oid) override;
    /// Fetch next object address to set token
    virtual StatusCode next(Token::OID_t& linkH) override;

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
    virtual StatusCode load( void** ptr, ShapeH shape,
                           const Token::OID_t& lnkH,
                           Token::OID_t&       oid,
                           bool                any_next) override;

    /// Find object by object identifier and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  oid      [OUT]   Object OID
      *
      * @return Status code indicating success or failure.
      */
    virtual StatusCode loadObject(void** ptr, ShapeH shape, Token::OID_t& oid) = 0;

  };
}       // End namespace pool
#endif  // POOL_DBCONTAINERIMP_H
