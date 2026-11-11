/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITECHCONTAINER_H
#define ITECHCONTAINER_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/pool.h"

#include <vector>
#include <cstdint>

// Forward declarations
class DbTypeInfo;
class DbDatabase;
class DbContainer;
class DbOption;
class Shape;

/** @class ITechContainer ITechContainer.h StorageSvc/ITechContainer.h 
    *
    * Description:
    * Interface to the technology specific implementation of the Container object.
    * (usually defined in a separate dynamically loaded plugin package)
    *
    * @author  M.Frank
    */
class ITechContainer    {
  protected:
    /// Destructor (called only by sub-classes)
    virtual ~ITechContainer() = default;

  public:
    /// Release the technology specific implementation
    virtual void release() = 0;
    /// Access to container size
    virtual uint64_t size() = 0;
    /// Get container name
    virtual std::string name() const = 0;
    /// Set options
    virtual StatusCode setOption(const DbOption& refOpt) = 0;
    /// Access options
    virtual StatusCode getOption(DbOption& refOpt) = 0;
    /// Store object in location
    virtual StatusCode store(const void* object,
                             DbContainer&  cntH,
                             const Shape* shape) = 0;
    /// In place allocation of object location
    virtual StatusCode allocate(DbContainer& cntH,
                                const void* object,
                                const Shape* shape,
                                Token::OID_t& oid) = 0;

    /// Number of next record in the container (=size if no delete is allowed)
    virtual uint64_t nextRecordId() = 0;
    /// Suggest next Record ID for tbe next object written - used only with synced indexes
    virtual void useNextRecordId(uint64_t) = 0;

    /// Close the container
    virtual StatusCode close() = 0;
    /// Open the container
    virtual StatusCode open( DbDatabase&        dbH, 
                             const std::string& nam, 
                             const DbTypeInfo* info, 
                             Io::IoFlag mode) = 0;
    /// Check if we can access the container for reading with the given type
    virtual StatusCode checkAccess(DbDatabase&        dbH,
                                 const std::string& nam) const = 0;
    /// Fetch next object address to set token
    virtual StatusCode next(Token::OID_t& linkH) = 0;

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
    virtual StatusCode load( void** ptr, const Shape* shape,
                           const Token::OID_t& lnkH,
                           Token::OID_t&       oid,
                           bool                any_next=false) = 0;

    /// Execute Transaction Action
    virtual StatusCode transAct(Transaction::Action) = 0;
};

#endif
