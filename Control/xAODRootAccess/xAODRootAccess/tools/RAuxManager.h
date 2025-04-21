// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_TOOLS_RAUXMANAGER_H
#define XAODROOTACCESS_TOOLS_RAUXMANAGER_H

// Local include(s):
#include "TVirtualManager.h"

// System include(s):
#include <functional>
#include <memory>

// Forward declaration(s):
namespace SG {
class IConstAuxStore;
}

namespace xAOD {

// Forward declaration(s):
class RAuxStore;

/// @short Manager for RAuxStore objects
///
/// This class is used when connecting RAuxStore objects to the
/// input ntuple as the auxiliary store of a DV container.
///
class RAuxManager : public TVirtualManager {

 public:
  /// Constructor getting hold of an auxiliary store object
  RAuxManager(RAuxStore* store, ::Long64_t& entry,
              ::Bool_t sharedOwner = kTRUE);

  /// Function for updating the object in memory if needed
  virtual ::Int_t getEntry(::Int_t getall = 0) override;

  /// Function getting a const pointer to the object being handled
  virtual const void* object() const override;
  /// Function getting a pointer to the object being handled
  virtual void* object() override;
  /// Function replacing the object being handled
  virtual void setObject(void* obj) override;

  /// Create the object for the current event
  virtual ::Bool_t create() override;
  /// Check if the object was set for the current event
  virtual ::Bool_t isSet() const override;
  /// Reset the object at the end of processing of an event
  virtual void reset() override;

  /// Get a type-specific pointer to the managed object
  RAuxStore* getStore();
  /// Get a convenience pointer to the managed object
  const SG::IConstAuxStore* getConstStore() const;

 private:
  /// The auxiliary store object
  std::shared_ptr<RAuxStore> m_store;
  /// Pointer to the auxiliary store object
  RAuxStore* m_storePtr;
  /// Entry number
  std::reference_wrapper<::Long64_t> m_entry;

};  // class RAuxManager

}  // namespace xAOD

#endif  // XAODROOTACCESS_TOOLS_TAUXMANAGER_H
