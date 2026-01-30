/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database implementation
//--------------------------------------------------------------------
//
//        Package    : RootStorageSvc (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#ifndef POOL_ROOTKEYCONTAINER_H
#define POOL_ROOTKEYCONTAINER_H 1

// Framework include files
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainerImp.h"

// Forward declarations
class StatusCode;
namespace CINT { class IOHandler; }
class TDirectory;
class TBuffer;
class TClass;

namespace pool  { 
  // Forward declarations
  class RootKeyIOHandler;

  /** @class RootKeyContainer RootKeyContainer.h src/RootKeyContainer.h
    *
    * Description:
    * ROOT specific implementation of Database container.
    * Since objects in root are stored in "trees with branches",
    * this object corresponds top a tuple (Tree/Branch), where
    * each object type (determined by the location of the transient
    * object within the data store) is accessed by the "Event" number
    * inside its tree.
    *
    * @author  M.Frank
    * @date    1/8/2002
    * @version 1.0
    */
  class RootKeyContainer : public DbContainerImp {
    /// Reference to the root tree object
    TDirectory*        m_dir;
    /// Parent Database handle
    DbDatabase         m_dbH;
    /// Root database file reference
    RootDatabase*         m_rootDb;
    /// CINT IO handler to allow user overloads....
    RootKeyIOHandler*  m_ioHandler;
    /// Policy flag
    int                m_policy;
    /// Number of bytes written/read during last operation. Set to -1 if it failed.
    int                m_ioBytes;

  protected:
    /// Commit single entry to container
    virtual StatusCode writeObject(ActionList::value_type&) override;
  public:
    explicit RootKeyContainer(const std::string& name);
    virtual ~RootKeyContainer();
    RootKeyContainer(const RootKeyContainer&) = delete;
    RootKeyContainer& operator=(const RootKeyContainer&) = delete;
    /// Close the container and deallocate resources
    virtual StatusCode close() override;
    /// Open the container for object access
    virtual StatusCode open(DbDatabase& dbH,
                            const std::string& nam, 
                            const DbTypeInfo* info,
                            DbAccessMode mod) override;
    /// Check if we can access the container for reading with the given type
    virtual StatusCode checkAccess(DbDatabase& dbH,
                                 const std::string& nam) const override final;
    /// Number of entries within the container
    virtual uint64_t size() override;
    /// Number of record in the container
    virtual uint64_t nextRecordId() override;
    /// Fetch next object address to set token
    virtual StatusCode next(Token::OID_t& linkH) override;

    /// Find object by object identifier and load it into memory
   /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  oid      [OUT]   Object OID
      *
      * @return Status code indicating success or failure.
      */
    virtual StatusCode loadObject( void** ptr, ShapeH shape,
                                   Token::OID_t& oid) override;

    /// Interface Implementation: Find entry in container
    virtual StatusCode load( void** ptr, ShapeH shape,
                             const Token::OID_t& linkH,
                             Token::OID_t& oid,
                             bool          any_next) override;

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

    /// Execute end of object modification requests during a transaction
    /** @param refTr    [IN]  Transaction reference
      *
      * @return StatusCode code indicating success or failure.  
      */
    /// Execute transaction action
    virtual StatusCode transAct(Transaction::Action action) override;
  };
}
#endif //POOL_ROOTKEYCONTAINER_H
