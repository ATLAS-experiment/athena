// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_TOOLS_AUXSTOREBASE_H
#define XAODROOTACCESS_TOOLS_AUXSTOREBASE_H

// Framework include(s).
#include "AsgMessaging/StatusCode.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/tools/threading.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "xAODCore/AuxSelection.h"

// System include(s).
#include <memory>
#include <string>
#include <string_view>

namespace xAOD::details {

/// Common base class for the auxiliary store implementations
///
/// This class provides the common functionality for the auxiliary store
/// implementations in xAODRootAccess. Reducing the amount of code duplication
/// in the different classes.
///
class AuxStoreBase : public SG::IAuxStore, public SG::IAuxStoreIO {

 public:
  /// "Structural" modes of the object
  enum class EStructMode {
    kUndefinedStore = 0,  ///< The structure mode is not defined
    kContainerStore = 1,  ///< The object describes an entire container
    kObjectStore = 2      ///< The object describes a single object
  };

  /// Constructor
  AuxStoreBase(bool topStore = true,
               EStructMode mode = EStructMode::kUndefinedStore);
  /// Destructor
  virtual ~AuxStoreBase();

  /// Get what structure mode the object was constructed with
  EStructMode structMode() const;
  /// Set the structure mode of the object to a new value
  void setStructMode(EStructMode mode);

  /// Get the currently configured object name prefix
  const std::string& prefix() const;
  /// Set the object name prefix
  virtual void setPrefix(std::string_view prefix) = 0;

  /// Check if the object is a "top store", or not
  bool isTopStore() const;
  /// Set whether the object should behave as a "top store" or not
  void setTopStore(bool value = true);

  /// @name Functions implementing the @c SG::IConstAuxStore interface
  /// @{

  /// Get a pointer to a given array
  virtual const void* getData(SG::auxid_t auxid) const override;

  /// Return vector interface for one aux data item.
  virtual const SG::IAuxTypeVector* getVector(SG::auxid_t auxid) const override;

  /// Get the types(names) of variables handled by this container
  virtual const SG::auxid_set_t& getAuxIDs() const override;

  /// Get the types(names) of decorations handled by this container
  virtual const SG::auxid_set_t& getDecorIDs() const override;

  /// Get a pointer to a given array, creating the array if necessary
  virtual void* getDecoration(SG::auxid_t auxid, std::size_t size,
                              std::size_t capacity) override;

  /// Test if a variable is a decoration.
  virtual bool isDecoration(SG::auxid_t auxid) const override;

  /// Lock the object, and don't let decorations be added
  virtual void lock() override;
  /// Remove the decorations added so far. Only works for transient
  /// decorations.
  virtual bool clearDecorations() override;

  /// Lock a decoration.
  virtual void lockDecoration(SG::auxid_t auxid) override;

  /// Return the number of elements in the store
  virtual std::size_t size() const override;

  /// Return (const) interface for a linked variable.
  virtual const SG::IAuxTypeVector* linkedVector(
      SG::auxid_t auxid) const override;
  /// Return (non-const) interface for a linked variable.
  virtual SG::IAuxTypeVector* linkedVector(SG::auxid_t auxid) override;

  /// @}

  /// @name Functions implementing the @c SG::IAuxStore interface
  /// @{

  /// Get a pointer to a given array, creating the array if necessary
  virtual void* getData(SG::auxid_t auxid, std::size_t size,
                        std::size_t capacity) override;

  /// Return a set of writable data identifiers
  virtual const SG::auxid_set_t& getWritableAuxIDs() const override;

  /// Resize the arrays to a given size
  virtual bool resize(std::size_t size) override;
  /// Reserve a given size for the arrays
  virtual void reserve(std::size_t size) override;
  /// Shift the contents of the stored arrays
  virtual void shift(std::size_t pos, std::ptrdiff_t offs) override;
  /// Insert contents of another store via move.
  virtual bool insertMove(std::size_t pos, SG::IAuxStore& other,
                          const SG::auxid_set_t& ignore) override;

  /// @}

  /// @name Functions implementing the SG::IAuxStoreIO interface
  /// @{

  /// Get a pointer to the data being stored for one aux data item
  virtual const void* getIOData(SG::auxid_t auxid) const override;

  /// Return the type of the data to be stored for one aux data item
  virtual const std::type_info* getIOType(SG::auxid_t auxid) const override;

  /// Get the types(names) of variables created dynamically
  virtual const SG::auxid_set_t& getDynamicAuxIDs() const override;

  /// Select dynamic auxiliary attributes for writing
  virtual void selectAux(const std::set<std::string>& attributes);

  /// Get the IDs of the selected aux variables
  virtual SG::auxid_set_t getSelectedAuxIDs() const override;

  /// @}

 protected:
  /// Check if an auxiliary variable is selected for ouput writing
  bool isAuxIDSelected(SG::auxid_t auxid) const;

  /// @name Functions needed from the derived classes
  /// @{

  /// Reset all (transient) information in the object
  virtual void reset() = 0;

  /// Check if a given variable is available from the input
  virtual bool hasEntryFor(SG::auxid_t auxid) const = 0;
  /// Load a single variable from the input
  virtual StatusCode getEntryFor(SG::auxid_t auxid) = 0;
  /// Check if an output is being written by the object
  virtual bool hasOutput() const = 0;

  /// Connect a variable to the input
  virtual StatusCode setupInputData(SG::auxid_t auxid) = 0;
  /// Connect a variable to the output
  virtual StatusCode setupOutputData(SG::auxid_t auxid) = 0;

  /// Get a pointer to an input object, as it is in memory, for @c getIOData()
  virtual const void* getInputObject(SG::auxid_t auxid) const = 0;
  /// Get the type of an input object, for @c getIOType()
  virtual const std::type_info* getInputType(SG::auxid_t auxid) const = 0;

  /// @}

  /// Struct collecting all member variables of this base class
  struct Members {

    /// The "structural" mode of the object
    EStructMode m_structMode = EStructMode::kUndefinedStore;
    /// Flag stating whether this is a "top store"
    bool m_topStore = true;
    /// Static prefix for the branch names
    std::string m_prefix{};
    /// Dynamic prefix for the branch names
    std::string m_dynPrefix{};

    /// Store for the in-memory-only variables
    std::unique_ptr<SG::AuxStoreInternal> m_transientStore{};

    /// Internal list of auxiliary variable IDs handled currently by the object
    SG::auxid_set_t m_auxIDs{};
    /// Internal list of auxiliary decoration IDs handled currently by the
    /// object
    SG::auxid_set_t m_decorIDs{};
    /// Variables handled currently by the object (indexed by auxiliary ID)
    std::vector<std::unique_ptr<SG::IAuxTypeVector> > m_vecs{};
    /// The current size of the container being described
    std::size_t m_size = 0u;

    /// Per variable lock status (indexed by auxiliary ID)
    std::vector<bool> m_isDecoration{};

  };  // struct BaseMembers

  /// Member variables of the base class
  Members m_data;

  /// Mutex type for multithread synchronization
  using mutex_t = AthContainers_detail::mutex;
  /// Guard type for multithreaded synchronisation
  using guard_t = AthContainers_detail::lock_guard<mutex_t>;

 private:
  /// Object helping to select which auxiliary variables to write
  AuxSelection m_selection;
  /// Is this container locked?
  bool m_locked = false;
  /// Mutex objects used for multithreaded synchronisation
  mutable mutex_t m_mutex1, m_mutex2;

};  // class AuxStoreBase

}  // namespace xAOD::details

#endif  // XAODROOTACCESS_TOOLS_AUXSTOREBASE_H
