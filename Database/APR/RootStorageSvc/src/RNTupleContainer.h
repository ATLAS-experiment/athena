/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//====================================================================
//    Root Database Container RNTuple implementation
//--------------------------------------------------------------------
//    Author     : M.Nowak
//====================================================================
#ifndef POOL_RNTUPLECONTAINER_H
#define POOL_RNTUPLECONTAINER_H 1

// Framework include files
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbContainerImp.h"
#include "StorageSvc/DbDatabase.h"

#include "ROOT/RNTupleView.hxx"

#include <memory>
#include <optional>
#include <string>
#include <vector>

// Forward declarations
class TClass;
namespace RootAuxDynIO { class IRootAuxDynReader; class IRNTupleAuxDynWriter; }
namespace RootStorageSvc { class RNTupleWriterHelper; }

#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 35, 0 )
namespace ROOT { using ROOT::Experimental::RNTupleView; }
namespace ROOT::Experimental { class RNTupleReader; }
namespace ROOT { using ROOT::Experimental::RNTupleReader; }
#else
namespace ROOT { class RNTupleReader; }
#endif

/*
 * POOL namespace declaration
 */
namespace pool {

// Forward declaration
class DbColumn;
class RootDatabase;

/** @class RNTupleContainer RNTupleContainer.h src/RNTupleContainer.h
 *
 * Description:
 * RNTUPLE specific implementation of Database Container.
 */

class RNTupleContainer : public DbContainerImp
{
  using DbContainerImp::save;

  /// Definition of a field info structure
  struct FieldDesc : public DbColumn
  {
    std::string fieldname;
    std::optional< ROOT::RNTupleView<void> > view;
    std::string sgkey;
    TClass*     clazz = nullptr;
    void*       object = nullptr;

    // if writing char* keep a copy here as an std::string
    std::string str;

    // ----  extra variables used for AuxDyn attributes
    // number of rows written to this branch so far
    size_t rows_written = 0;

    // AuxDyn RNTuple reader (managed by the Database)
    std::unique_ptr<RootAuxDynIO::IRootAuxDynReader> auxdyn_reader;

    // AuxDyn RNTuple writer
    std::unique_ptr<RootAuxDynIO::IRNTupleAuxDynWriter> auxdyn_writer;

    FieldDesc(const DbColumn& c);
    FieldDesc(FieldDesc const& other) = delete;
    FieldDesc(FieldDesc&& other) = default;
    ~FieldDesc() = default;

    FieldDesc& operator=(FieldDesc const& other) = delete;
    FieldDesc& operator=(FieldDesc&& other) = default;

    const std::string typeName();
  };

 protected:
   /// reference to exact type description
   const DbTypeInfo*  m_type{};
   /// List of field descriptors
   std::vector<FieldDesc>  m_fieldDescs;
   /// Parent Database handle
   DbDatabase         m_dbH;
   /// Root database file reference
   RootDatabase*      m_rootDb;
   /// Number of bytes written/read during last operation. Set to -1 if it failed.
   int                m_ioBytes;
   /// flag set on writing to prevent double writes in the same commit
   bool               m_isDirty;

   uint64_t           m_index;
   uint64_t           m_indexSize;
   int64_t            m_indexBump;
   const uint32_t     m_indexMulti;

   /// Internal cache of the RNTupleWriterHelper
   RootStorageSvc::RNTupleWriterHelper*     m_ntupleWriter = nullptr;

   /// Internal cache of the native RNTupleReader
   ROOT::RNTupleReader*       m_ntupleReader{};

   /// Factory object from AuxDynIO plugin that creates AuxDyn readers and writers
   std::unique_ptr<RootAuxDynIO::IFactoryTool>       m_auxDynTool;

 public:
   /// Standard constructor
  RNTupleContainer();

  virtual ~RNTupleContainer();

  /// Close the container and deallocate resources
  virtual DbStatus close() override final;

  /// Open the container for object access
  virtual DbStatus open(DbDatabase& dbH, const std::string& nam,
                        const DbTypeInfo* info,
                        DbAccessMode mod) override final;

  /// Check if we can access the container for reading with the given type
  virtual DbStatus checkAccess(DbDatabase& dbH,
                               const std::string& nam) const override final;

  /// Access options
  /** @param opt      [IN]  Reference to option object.
   *
   * @return DbStatus code indicating success or failure.
   */
  virtual DbStatus getOption(DbOption& opt) override final;

  /// Set options
  /** @param opt      [IN]  Reference to option object.
   *
   * @return DbStatus code indicating success or failure.
   */
  virtual DbStatus setOption(const DbOption& opt) override final;

  /// Ask if a given shape is supported
  virtual DbStatus isShapeSupported(const DbTypeInfo* typ) const override final;

  /// Number of entries within the container
  virtual uint64_t size() override final;

  /// Return the name of the container
  const std::string& getName() const { return m_name; }

  /// Return true if this branch container was updated and it's TTree needs to
  /// be Filled
  bool isDirty() const { return m_isDirty; }

  /// Clear the dirty status (used after containing TTree was Filled)
  void clearDirty() { m_isDirty = false; }

  /// Find object by object identifier and load it into memory
  /** @param  ptr   [IN/OUT]   address of the pointer to object
   *  @param  shape     [IN]   Object Shape
   *  @param  oid      [OUT]   Object OID
   *
   *  @return Status code indicating success or failure.
   */
  virtual DbStatus loadObject(void** ptr, ShapeH shape,
                              Token::OID_t& oid) override final;

  /// Commit single entry to container
  virtual DbStatus writeObject(ActionList::value_type&) override final;

  virtual uint64_t nextRecordId() override final;

  virtual void useNextRecordId(uint64_t nextID) override final;

  /// Define selection criteria
  virtual DbStatus select(DbSelect& criteria) override final;

  /// Equivalent to next()
  using DbContainerImp::fetch;
  virtual DbStatus fetch(DbSelect& sel) override final;

  /// Execute transaction action
  virtual DbStatus transAct(Transaction::Action action) override final;

  /// Add single entry to container
  virtual DbStatus save(DbObjectHandle<DbObject>& objH) override final;

 private:
  /// Init a field description for an object (i.e. find TClass etc.)
  DbStatus initObjectFieldDesc(FieldDesc& dsc);
};

}  // namespace pool
#endif
