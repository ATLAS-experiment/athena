// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_TAUXSTORE_H
#define XAODROOTACCESS_TAUXSTORE_H

// Framework include(s).
#include "AsgMessaging/StatusCode.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"

// Local include(s).
#include "xAODRootAccess/tools/AuxStoreBase.h"

// System include(s).
#include <memory>
#include <string_view>

// Forward declaration(s):
class TTree;

namespace xAOD {

/// @short "ROOT @c TTree implementation" of @c IAuxStore
///
/// This is a "D3PDReader-like" implementation for the
/// auxiliary store interface. It is meant to provide very
/// efficient access to a relatively small number of auxiliary
/// variables.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TAuxStore : public details::AuxStoreBase {

 public:
  /// Constructor
  TAuxStore(std::string_view prefix = "", bool topStore = true,
            EStructMode mode = EStructMode::kUndefinedStore,
            int basketSize = 2048, int splitLevel = 0);
  /// Destructor
  virtual ~TAuxStore();

  /// Set the object name prefix
  virtual void setPrefix(std::string_view prefix) override;

  /// Get the size of the baskets created for the output branches
  int basketSize() const;
  /// Set the size of the baskets created for the output branches
  void setBasketSize(int value);

  /// Get the split level of the output branches
  int splitLevel() const;
  /// Set the split level of the output branches
  void setSplitLevel(int value);

  /// Connect the object to an input TTree
  StatusCode readFrom(::TTree& tree, bool printWarnings = true);
  /// Connect the object to an output TTree
  StatusCode writeTo(::TTree& tree);

  /// Read the values from the TTree entry that was loaded with
  /// @c TTree::LoadTree()
  int getEntry(int getall = 0);

  /// @name Functions implementing functionality for @c AuxStoreBase
  /// @{

  /// Tell the object that all branches will need to be re-read
  virtual void reset() override;

 private:
  /// Check if a given variable is available from the input
  virtual bool hasEntryFor(SG::auxid_t auxid) const override;
  /// Load a single variable from the input
  virtual StatusCode getEntryFor(SG::auxid_t auxid) override;
  /// Check if an output is being written by the object
  virtual bool hasOutput() const override;

  /// Connect a variable to the input
  virtual StatusCode setupInputData(SG::auxid_t auxid) override;
  /// Connect a variable to the output
  virtual StatusCode setupOutputData(SG::auxid_t auxid) override;

  /// Get a pointer to an input object, as it is in memory, for @c getIOData()
  virtual const void* getInputObject(SG::auxid_t auxid) const override;
  /// Get the type of an input object, for @c getIOType()
  virtual const std::type_info* getInputType(SG::auxid_t auxid) const override;

  /// @}

  /// Type holding the internals of the object
  struct impl;
  /// Pointer to the internal object
  std::unique_ptr<impl> m_impl;

};  // class TAuxStore

}  // namespace xAOD

#endif  // XAODROOTACCESS_TAUXSTORE_H
