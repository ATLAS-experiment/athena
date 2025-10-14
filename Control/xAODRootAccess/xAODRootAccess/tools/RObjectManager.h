// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H
#define XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H

// Local include(s).
#include "xAODRootAccess/tools/IObjectManager.h"

// ROOT include(s).
#include <ROOT/RNTupleView.hxx>

// System include(s).
#include <functional>
#include <memory>

namespace xAOD::Experimental {

/// @short Manager for EDM objects created by ROOT
///
/// This class is used when an EDM object is meant to be created
/// by ROOT's schema evolution system, behind the scenes.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
/// @author RD Schaffer <R.D.Schaffer@cern.ch>
///
class RObjectManager : public Details::IObjectManager {

 public:
  /// Constructor
  RObjectManager(ROOT::RNTupleView<void> field, const ::Long64_t& entry,
                 std::unique_ptr<THolder> holder);
  /// Destructor
  ~RObjectManager();

  /// Accessor to the field (non-const)
  ROOT::RNTupleView<void>& field();
  /// Accessor to the field (const)
  const ROOT::RNTupleView<void>& field() const;

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

 private:
  /// The typeless object taking care of reading the field
  ROOT::RNTupleView<void> m_field;
  /// Holder object for the EDM object
  std::unique_ptr<THolder> m_holder;
  /// Entry number to load next
  std::reference_wrapper<const ::Long64_t> m_entryToLoad;
  /// The last entry that was loaded for this field
  ::Long64_t m_entry;
  /// Was the object set for the current event?
  ::Bool_t m_isSet;

};  // class RObjectManager

}  // namespace xAOD::Experimental

#endif  // XAODROOTACCESS_TOOLS_ROBJECTMANAGER_H
