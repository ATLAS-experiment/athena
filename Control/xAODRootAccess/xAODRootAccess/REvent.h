// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_REVENT_H
#define XAODROOTACCESS_REVENT_H

// Local include(s).
#include "xAODRootAccess/Event.h"

// ROOT include(s).
#include <ROOT/RNTupleReader.hxx>

// System include(s).
#include <memory>
#include <string_view>

namespace xAOD::Experimental {

// Forward declaration(s).
class RObjectManager;

/// @short Tool for accessing xAOD files outside of Athena, version RNTuple
///
/// Proper access to xAOD files in ROOT (outside of Athena) needs to
/// be done through such an object. It takes care of reading and
/// writing xAOD files together with their file format metadata,
/// setting up smart pointers correctly, etc.
///

class REvent : public Event {

 public:
  /// Default constructor
  REvent();

  /// Destructor
  virtual ~REvent();

  /// @name Setup functions
  /// @{

  /// Set up the reading of an input file
  StatusCode readFrom(std::string_view fileName);

  /// @}

  /// @name Persistent data accessor/modifier functions
  /// @{

  /// Get how many entries are available from the current input file(s)
  ::Long64_t getEntries() const;
  /// Function loading a given entry of the input TTree
  ::Int_t getEntry(::Long64_t entry, ::Int_t getall = 0);

  /// @}

 private:
  /// @name Functions implemented from @c xAOD::Event
  /// @{

  /// Check if an input file is connected to the object
  bool hasInput() const override;
  /// Check if an output file is connected to the object
  bool hasOutput() const override;

  /// Function determining the list keys associated with a type name
  StatusCode getNames(const std::string& targetClassName,
                      std::vector<std::string>& vkeys,
                      bool metadata) const override;

  /// Function setting up access to a particular object
  StatusCode connectObject(const std::string& key, bool silent) override;
  /// Function setting up access to a particular metadata object
  StatusCode connectMetaObject(const std::string& key, bool silent) override;
  /// Function setting up access to a set of auxiliary branches
  StatusCode connectAux(const std::string& prefix, bool standalone) override;
  /// Function setting up access to a set of auxiliary branches for a
  /// metadata object
  StatusCode connectMetaAux(const std::string& prefix,
                            bool standalone) override;

  /// Function connecting a DV object to its auxiliary store
  StatusCode setAuxStore(const std::string& key, Details::IObjectManager& mgr,
                         bool metadata) override;

  /// Record an object into a connected output file
  StatusCode record(void* obj, const std::string& typeName,
                    const std::string& key, bool overwrite, bool metadata,
                    bool isOwner) override;
  /// Record an auxiliary store into a connected output file
  StatusCode recordAux(TVirtualManager& mgr, const std::string& key,
                       bool metadata) override;

  /// @}

  /// Function to initialise the statistics for all Tree content
  StatusCode initStats();

  /// event uses RNTupleReader:
  StatusCode setUpDynamicStore(RObjectManager& mgr,
                               ROOT::RNTupleReader& ntupleReader);

  /// The main event data reader
  std::unique_ptr<ROOT::RNTupleReader> m_eventReader;
  /// Whether the input has an event RNTuple or not
  bool m_inputNTupleIsMissing = false;
  /// The metadata reader
  std::unique_ptr<ROOT::RNTupleReader> m_metaReader;

  /// The entry to look at from the input tree
  ::Long64_t m_entry{};

};  // class REvent

}  // namespace xAOD::Experimental

#endif  // XAODROOTACCESS_REVENT_H
