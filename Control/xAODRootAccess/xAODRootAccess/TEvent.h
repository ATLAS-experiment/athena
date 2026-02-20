// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TEVENT_H
#define XAODROOTACCESS_TEVENT_H

// Local include(s).
#include "xAODRootAccess/Event.h"

// Project include(s).
#include "AthContainersInterfaces/IAuxStoreHolder.h"

// ROOT include(s).
#include <Rtypes.h>

// System include(s).
#include <memory>
#include <regex>
#include <string>
#include <string_view>

// Forward declaration(s).
class TFile;
class TChain;
class TTree;
namespace CP {
class xAODWriterAlg;
}
class xAODTEventBranch;
class xAODTMetaBranch;
namespace SG {
class IAuxStore;
}

namespace xAOD {

// Forward declaration(s).
class TAuxStore;
class TObjectManager;
class TChainStateTracker;
class TFileMerger;
class TEvent;
class TTreeMgr;
::TTree* MakeTransientTree ATLAS_NOT_THREAD_SAFE(TEvent &, const char* );

/// @short Tool for accessing xAOD files outside of Athena
///
/// Proper access to xAOD files in ROOT (outside of Athena) needs to
/// be done through such an object. It takes care of reading and
/// writing xAOD files together with their file format metadata,
/// setting up smart pointers correctly, etc.
///
/// For a detailed description of the usage of this class, see:
///   <Link to be added here...>
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TEvent : public Event {

  // Declare the friend functions/classes:
  friend ::TTree* MakeTransientTree(TEvent &, const char* );
  friend class ::xAODTEventBranch;
  friend class ::xAODTMetaBranch;
  friend class xAOD::TFileMerger;
  friend class xAOD::TTreeMgr;
  friend class CP::xAODWriterAlg;

public:
  /// Auxiliary store "mode"
  enum EAuxMode {
    kBranchAccess = 0, ///< Access auxiliary data branch-by-branch
    kClassAccess = 1,  ///< Access auxiliary data using the aux containers
    kAthenaAccess = 2  ///< Access containers/objects like Athena does
  };

  /// Default constructor
  TEvent(EAuxMode mode = kClassAccess);
  /// Constructor connecting the object to an input TFile
  TEvent(::TFile* file, EAuxMode mode = kClassAccess);
  /// Constructor connecting the objects to an input TTree/TChain
  TEvent(::TTree* tree, EAuxMode mode = kClassAccess);
  /// Destructor
  virtual ~TEvent();

  /// Change the pattern used for collecting information from other MetaData
  /// trees NB: Additional  MetaData trees are only expected for augmented files
  /// This function also allows user to redefine MetaData tree pattern
  /// to skip trees that would not be proper MetaData ones
  /// i.e. trees not containing an EventFormat* branch
  void setOtherMetaDataTreeNamePattern(const std::string &pattern);

  /// Get what auxiliary access mode the object was constructed with
  EAuxMode auxMode() const;

  /// @name Setup functions
  /// @{


  /// Set up the reading of an input file from TFile
  /// This method implements the interface from Event
  StatusCode readFrom(::TFile& inFile) override;

  /// This is the 'native' interface for reading from a TFile, 
  /// allowing the specification of TTree cache use and the event tree name
  StatusCode readFrom(::TFile* file, bool useTreeCache = true,
                      std::string_view treeName = EVENT_TREE_NAME);
  /// Connect the object to a new input tree/chain
  StatusCode readFrom(::TTree* tree, bool useTreeCache = true);
  /// Connect the object to an output file
  StatusCode writeTo(TFile& file) override;
  /// Connect the object to an output file
  StatusCode writeTo(::TFile* file, int autoFlush = 200,
                     std::string_view treeName = EVENT_TREE_NAME);
  /// Finish writing to an output file
  StatusCode finishWritingTo(TFile& file) override;
  /// Finish writing to an output file
  StatusCode finishWritingTo(::TFile* file);

  /// @}

  /// @name Event data accessor/modifier functions
  /// @{

  // Bring the definition of Event::record into scope to allow a TEvent object to record object
  using Event::record;

  /// Add an auxiliary store object to the output
  SG::IAuxStore* recordAux(const std::string& key,
                           SG::IAuxStoreHolder::AuxStoreType type =
                               SG::IAuxStoreHolder::AST_ContainerStore);

  /// @}

  /// @name Persistent data accessor/modifier functions
  /// @{

  /// Get how many entries are available from the current input file(s)
  ::Long64_t getEntries() const override;
  /// Function loading a given entry of the input TTree
  ::Int_t getEntry(::Long64_t entry, ::Int_t getall = 0) override;

  /// Get how many files are available on the currently defined input
  ::Long64_t getFiles() const;
  /// Load the first event for a given file from the input TChain
  ::Int_t getFile(::Long64_t file, ::Int_t getall = 0);

  /// Function filling one event into the output tree
  ::Int_t fill() override;

  /// @}

protected:
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

  /// Internal function for adding an auxiliary store object to the output
  StatusCode record(std::unique_ptr<TAuxStore> store, const std::string& key);
  /// Function adding dynamic variable reading capabilities to an auxiliary
  /// store object
  StatusCode setUpDynamicStore(TObjectManager& mgr, ::TTree* tree);
  /// Function saving the dynamically created auxiliary properties
  StatusCode putAux(::TTree& outTree, TVirtualManager& mgr, bool metadata);
  /// Function setting up an existing auxiliary store for writing
  StatusCode recordAux(TAuxStore* store, const std::string& key);

  /// The auxiliary access mode
  EAuxMode m_auxMode;

  /// The main tree that we are reading from
  ::TTree* m_inTree = nullptr;
  /// Internal status flag showing that an input file is open, but it
  /// doesn't contain an event tree
  bool m_inTreeMissing = false;
  /// The (optional) chain provided as input
  ::TChain* m_inChain = nullptr;
  /// Optional object for tracking the state changes of an input TChain
  std::unique_ptr<TChainStateTracker> m_inChainTracker;
  /// The number of the currently open tree in the input chain
  ::Int_t m_inTreeNumber = -1;
  /// Pointer to the metadata tree in the input file
  ::TTree* m_inMetaTree = nullptr;
  /// The entry to look at from the input tree
  ::Long64_t m_entry = -1;

  /// The tree that we are writing to
  std::unique_ptr<::TTree> m_outTree;

  // Regular expression to match other MetaData trees in augmented files
  // Other MetaData trees should be called MetaData_* but not named
  // MetaDataHdr_* Those additional trees are only expected for augmented files
  std::regex m_otherMetaDataTreeNamePattern =
      std::regex("^MetaData(?!Hdr)_.*$");
}; // class TEvent

} // namespace xAOD

#endif // XAODROOTACCESS_TEVENT_H
