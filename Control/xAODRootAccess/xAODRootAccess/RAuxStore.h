// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_RAUXSTORE_H
#define XAODROOTACCESS_RAUXSTORE_H

// Local include(s).
#include "xAODRootAccess/tools/AuxStoreBase.h"

// ROOT include(s).
#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>

// System include(s).
#include <cstdint>
#include <memory>
#include <string_view>

// Make the RNTuple types available in the ROOT namespace
// with all versions of ROOT.
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 1)
namespace ROOT {
using Experimental::RNTupleReader;
using Experimental::RNTupleWriter;
using Experimental::REntry;
}  // namespace ROOT
#endif  // ROOT_VERSION_CODE < ROOT_VERSION(6, 36, 0)


namespace xAOD {

/// @short "ROOT @c RNTuple implementation" of @c IAuxStore
///
/// @author Mads Lildholdt Hansen <mlha20@student.aau.dk>
/// @author Nikolaj Kofod Krogh <nkrogh20@student.aau.dk>
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class RAuxStore : public details::AuxStoreBase {

 public:
  /// Constructor
  RAuxStore(std::string_view prefix = "", bool topStore = true,
            EStructMode mode = EStructMode::kUndefinedStore);
  /// Destructor
  virtual ~RAuxStore();

  /// Set the object name prefix
  virtual void setPrefix(std::string_view prefix) override;

  /// Connect the object to an input RNTuple
  StatusCode readFrom(ROOT::RNTupleReader& reader);
  /// Add the variables of the store to an output RNTuple
  StatusCode writeTo(ROOT::RNTupleWriter& writer);

  /// Get entry from the input RNTuple
  StatusCode getEntry(std::int64_t entry, int getall = 0);
  /// Commit a new entry to the output RNTuple
  StatusCode commitTo(ROOT::REntry& entry);

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

};  // class RAuxStore
}  // namespace xAOD

#endif  // XAODROOTACCESS_RAUXSTORE_H
