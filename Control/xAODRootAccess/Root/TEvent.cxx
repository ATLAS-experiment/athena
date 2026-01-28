// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/TEvent.h"

#include "IOUtils.h"

// System include(s):
#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>

// ROOT include(s):
#include <TBranch.h>
#include <TChain.h>
#include <TChainElement.h>
#include <TError.h>
#include <TFile.h>
#include <TFriendElement.h>
#include <TKey.h>
#include <TMethodCall.h>
#include <TSystem.h>
#include <TTree.h>

// Gaudi/Athena include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#ifndef XAOD_STANDALONE
#include "SGTools/CurrentEventStore.h"
#include "SGTools/DataProxy.h"
#endif  // not XAOD_STANDALONE
#include "CxxUtils/ClassName.h"
#include "CxxUtils/no_sanitize_undefined.h"

// Interface include(s):
#include "xAODRootAccessInterfaces/TActiveEvent.h"

// xAOD include(s):
#include "xAODCore/AuxSelection.h"
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/PerfStats.h"
#include "xAODCore/tools/ReadStats.h"

// Local include(s):
#include "xAODRootAccess/TActiveStore.h"
#include "xAODRootAccess/TAuxStore.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TVirtualIncidentListener.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/TAuxBranchManager.h"
#include "xAODRootAccess/tools/TAuxManager.h"
#include "xAODRootAccess/tools/TChainStateTracker.h"
#include "xAODRootAccess/tools/TDirectoryReset.h"
#include "xAODRootAccess/tools/TEventFormatRegistry.h"
#include "xAODRootAccess/tools/TFileAccessTracer.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/TIncident.h"
#include "xAODRootAccess/tools/TObjectManager.h"
#include "xAODRootAccess/tools/TPrimitiveAuxBranchManager.h"
#include "xAODRootAccess/tools/Utils.h"

namespace xAOD {

/// Size of a possible TTreeCache
static const ::Int_t CACHE_SIZE = -1;

TEvent::TEvent(EAuxMode mode) : Event("xAOD::TEvent"), m_auxMode(mode) {}

TEvent::TEvent(::TFile* file, EAuxMode mode) : TEvent(mode) {

  // Let the initialisation function deal with setting up the object.
  readFrom(file).ignore();
}

TEvent::TEvent(::TTree* tree, EAuxMode mode) : TEvent(mode) {

  // Let the initialisation function deal with setting up the object.
  readFrom(tree).ignore();
}

TEvent::~TEvent() {

  // Check that the user didn't forget to call finishWritingTo().
  if (m_outTree) {
    ATH_MSG_ERROR(
        "Did not call finishWritingTo() before destroying the TEvent object!");
  }

  // Clear the input and output objects before the input/output files would be
  // closed. Otherwise we can be left with "TTree related" objects pointing
  // nowhere.
  m_inputObjects.clear();
  m_outputObjects.clear();
}

/// @returns The auxiliary data access mode currently in use
///
TEvent::EAuxMode TEvent::auxMode() const { return m_auxMode; }

void TEvent::setOtherMetaDataTreeNamePattern(const std::string &pattern) {
  // Only change if pattern provided is not empty
  if (pattern.size()) {
    // User provided a regular expression for other MetaData trees
    m_otherMetaDataTreeNamePattern = std::regex(pattern);
  }
}

/// Interface implementation for reading - forward to readFrom below
StatusCode TEvent::readFrom(TFile& inFile) {
  ATH_CHECK(readFrom(&inFile));
  return StatusCode::SUCCESS;
}


/// This function takes care of connecting the event object to a new input
/// file. It reads in the metadata of the input file needed for reading
/// the file.
///
/// @param file Pointer to the file being read
/// @param useTreeCache Flag for turning on/off the usage of TTreeCache
/// @param treeName Name of the input tree
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::readFrom(::TFile* file, bool useTreeCache,
                            std::string_view treeName) {

  // If no file was specified, return gracefully.
  if (file == nullptr) {
    ATH_MSG_DEBUG("No input file specified for readFrom(...)");
    return StatusCode::SUCCESS;
  }

  // Clear the cached input objects.
  m_inputObjects.clear();
  m_inputMissingObjects.clear();
  m_inputMetaObjects.clear();
  {
    upgrading_lock_t lock(m_branchesMutex);
    lock.upgrade();
    m_branches.clear();
  }

  // Reset the internal flags.
  m_inTreeMissing = kFALSE;
  m_entry = -1;

  // Make sure we return to the current directory:
  TDirectoryReset dr;

  // Set up the file access tracer.
  static TFileAccessTracer tracer ATLAS_THREAD_SAFE;
  tracer.add(*file);

  // Look for the metadata tree:
  m_inMetaTree = file->Get<TTree>(METADATA_OBJECT_NAME);
  if (m_inMetaTree == nullptr) {
    ATH_MSG_ERROR("Couldn't find metadata tree on input. Object is unusable!");
    return StatusCode::FAILURE;
  }

  // Set metadata entry to be read
  // NB: no reading is done calling LoadTree
  if (m_inMetaTree->LoadTree(0) < 0) {
    ATH_MSG_ERROR("Failed to load entry 0 for metadata tree");
    return StatusCode::FAILURE;
  }

  // A sanity check.
  if (m_inMetaTree->GetEntries() != 1) {
    ATH_MSG_WARNING("Was expecting a metadata tree with size 1, instead of "
                    << m_inMetaTree->GetEntries() << ".");
    ATH_MSG_WARNING("The input file was most probably produced by hadd...");
  }

  // Make sure that the xAOD::EventFormat dictonary is loaded.
  // This may not be the case if streamer information reading is turned
  // off.
  static const std::string eventFormatTypeName =
      Utils::getTypeName(typeid(EventFormat));
  ::TClass* cl = ::TClass::GetClass(eventFormatTypeName.c_str());
  if (cl == nullptr) {
    ATH_MSG_WARNING("Couldn't load the xAOD::EventFormat dictionary");
  }

  // Helper lambda for collecting the event format metadata from an RNTuple
  // with a given name.
  auto readEventFormatMetadata =
      [&](std::string_view thisTreeName) -> StatusCode {
    // Look for the metadata tree:
    TTree* metaTree = file->Get<TTree>(thisTreeName.data());
    if (metaTree == nullptr) {
      ATH_MSG_ERROR("Couldn't find metadata tree \"" << thisTreeName
                                                     << "\"on input.");
      return StatusCode::FAILURE;
    }
    // Set metadata entry to be read.
    if (metaTree->LoadTree(0) < 0) {
      ATH_MSG_ERROR("Failed to load entry 0 for metadata tree \""
                    << thisTreeName << "\"");
      return StatusCode::FAILURE;
    }

    // Check if the EventFormat branch is available:
    const std::string eventFormatBranchName =
        Utils::getFirstBranchMatch(m_inMetaTree, "EventFormat");
    if (!metaTree->GetBranch(eventFormatBranchName.c_str())) {
      // This can happen when the file was produced by an Athena job that
      // didn't have any input events itself. This means that the file
      // doesn't actually have any useful metadata.
      ATH_MSG_INFO("Input file provides no event or metadata");
      return StatusCode::RECOVERABLE;
    }

    // Read in the event format object:
    EventFormat* format = 0;
    ::TBranch* br = 0;
    const Int_t status =
        metaTree->SetBranchAddress(eventFormatBranchName.c_str(), &format, &br);
    if (status < 0) {
      ATH_MSG_ERROR("Failed to connect to xAOD::EventFormat object");
      return StatusCode::FAILURE;
    }

    // Merge the object into our private member.
    br->GetEntry(0);
    for (const auto &[key, element] : *format) {
      m_inputEventFormat.add(element);
    }

    // This is a strange place. The object has to be deleted, as it is the
    // responsibility of the user code to do so. But if I also explicitly
    // tell the branch to forget about the address of the pointer, then
    // all hell breaks loose...
    delete format;

    // Return gracefully.
    return StatusCode::SUCCESS;
  };

  // Read in the metadata from the "main" metadata ntuple.
  m_inputEventFormat = {};
  const StatusCode sc = readEventFormatMetadata(METADATA_OBJECT_NAME);
  if (sc.isRecoverable()) {
    m_inTree = nullptr;
    m_inTreeMissing = true;
    return StatusCode::SUCCESS;
  }
  ATH_CHECK(sc);

  // List all the other Metadata trees in the input file
  // Having several metatrees can happen for augmented files for instance
  // as one metadata tree per stream is produced
  std::set<std::string> lOtherMetaTreeNames = {};
  TList* lKeys = file->GetListOfKeys();

  if (lKeys) {
    for (int iKey = 0; iKey < lKeys->GetEntries(); iKey++) {
      // iterate over keys and add
      std::string keyName = lKeys->At(iKey)->GetName();
      // Make sure the key corresponds to a metadata tree but
      // do not add the current metadata tree in the list of other trees
      // and do not add the metadata tree handlers to the list
      if ((keyName != METADATA_OBJECT_NAME) &&
          std::regex_match(keyName, m_otherMetaDataTreeNamePattern)) {
        // Make sure key corresponds to a tree
        const char* className = ((::TKey* )lKeys->At(iKey))->GetClassName();
        static constexpr Bool_t LOAD = kFALSE;
        static constexpr Bool_t SILENT = kTRUE;
        ::TClass* cl = ::TClass::GetClass(className, LOAD, SILENT);
        if ((cl != nullptr) && cl->InheritsFrom(::TTree::Class())) {
          // key is corresponding to a metadata tree
          lOtherMetaTreeNames.insert(std::move(keyName));
        }
      }
    }
  }

  // Loop over the other metadata trees found (if any).
  for (const std::string &metaTreeName : lOtherMetaTreeNames) {
    ATH_CHECK(readEventFormatMetadata(metaTreeName));
  }

  // Look for the event tree in the input file.
  m_inTree = file->Get<TTree>(treeName.data());
  if (m_inTree == nullptr) {
    // This is no longer an error condition. As it can happen for DxAODs
    // that don't have any events in them. But they still have metadata
    // that needs to be collected.
    m_inTreeMissing = kTRUE;
  }

  // Turn on the cache if requested.
  if (m_inTree && useTreeCache && (!m_inTree->GetCacheSize())) {
    m_inTree->SetCacheSize(CACHE_SIZE);
    m_inTree->SetCacheLearnEntries(10);
  }

  // Init the statistics collection.
  ATH_CHECK(initStats());
  // Update the event counter in the statistics object.
  xAOD::ReadStats &stats = IOStats::instance().stats();
  if (m_inTree) {
    stats.setNEvents(stats.nEvents() + m_inTree->GetEntries());
  }

  // Notify the listeners that a new file was opened.
  const TIncident beginIncident(IncidentType::BeginInputFile);
  for (TVirtualIncidentListener* listener : m_listeners) {
    listener->handle(beginIncident);
  }
  // For now implement a very simple scheme in which we claim already
  // at the start that the entire file was processed. Since we have no way
  // of ensuring that the user indeed does this. And we can't delay calling
  // this function, as the user may likely close his/her output file before
  // closing the last opened input file.
  const TIncident endIncident(IncidentType::EndInputFile);
  for (TVirtualIncidentListener* listener : m_listeners) {
    listener->handle(endIncident);
  }

  // The initialisation was successful.
  return StatusCode::SUCCESS;
}

/// This version of the function sets up the object to read information
/// from a tree/chain. Using it with a TTree pointer makes not much sense,
/// but using it with a TChain pointer could be a very valid usage mode.
///
/// @param tree The pointer to a TTree or a TChain
/// @param useTreeCache Flag for switching TTreeCache usage on/off
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::readFrom(::TTree* tree, bool useTreeCache) {

  // Remember the info:
  m_inTree = nullptr;
  m_inTreeMissing = false;
  m_inChain = dynamic_cast<TChain* >(tree);
  m_inMetaTree = nullptr;

  if (m_inChain) {

    // Set up the caching on the chain level. The individual trees of the
    // input files will get a cache set up automatically after this.
    if (useTreeCache && (!m_inChain->GetCacheSize())) {
      m_inChain->SetCacheSize(CACHE_SIZE);
      m_inChain->SetCacheLearnEntries(10);
    }

    // Explicitly open the first file of the chain. To correctly auto-load
    // the dictionaries necessary. This doesn't happen automatically with
    // some ROOT versions...
    const TObjArray* files = m_inChain->GetListOfFiles();
    if (!files) {
      ATH_MSG_ERROR("Couldn't get the list of files from the input TChain");
      return StatusCode::FAILURE;
    }
    if (!files->GetEntries()) {
      ATH_MSG_ERROR("No files are present in the received TChain");
      return StatusCode::FAILURE;
    }
    const ::TChainElement* chEl =
        dynamic_cast<const ::TChainElement* >(files->At(0));
    if (!chEl) {
      ATH_MSG_ERROR("Couldn't cast object to TChainElement");
      return StatusCode::FAILURE;
    }
    {
      std::unique_ptr<TFile> dummyFile{TFile::Open(chEl->GetTitle())};
      if (!dummyFile) {
        ATH_MSG_ERROR("Couldn't open file " << chEl->GetTitle());
        return StatusCode::FAILURE;
      }
    }

    // Set up a tracker for the chain.
    if (!m_inChainTracker) {
      m_inChainTracker = std::make_unique<TChainStateTracker>();
    }
    m_inChainTracker->reset();
    tree->SetNotify(m_inChainTracker.get());

    // Stop at this point. The first file will be opened when the user
    // asks for the first event. Otherwise we open the first file of the
    // chain multiple times.
    m_inTreeNumber = -1;
    return StatusCode::SUCCESS;

  } else {

    // If it's a simple TTree, then let's fully initialise the object
    // using its file:
    m_inTreeNumber = -1;
    if (m_inChainTracker) {
      m_inChainTracker.reset();
    }
    ::TFile* file = tree->GetCurrentFile();
    return readFrom(file, useTreeCache, tree->GetName());
  }
}


/// This interface function should be called on a file opened be the user, before
/// any event processing would occur. It sets up the output event tree.
///
/// @param file The file that the event data should be written to
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::writeTo(TFile& file) {
  
  // Forward call
  ATH_CHECK(writeTo(&file));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function should be called on a file opened be the user, before
/// any event processing would occur. It sets up the output event tree.
///
/// @param file The file that the event data should be written to
/// @param autoFlush The auto-flush setting to use on the output TTree
/// @param treeName Name of the output event tree
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::writeTo(::TFile* file, int autoFlush,
                           std::string_view treeName) {

  // Just a simple security check.
  if (!file) {
    ATH_MSG_ERROR("Null pointer received!");
    return StatusCode::FAILURE;
  }

  // Check that the object is in the "right state":
  if (m_outTree) {
    ATH_MSG_ERROR("Object already writing to a file. Close that file first!");
    return StatusCode::FAILURE;
  }

  // Make sure we return to the current directory:
  TDirectoryReset dr;

  // Create the output TTree:
  file->cd();
  m_outTree = std::make_unique<TTree>(treeName.data(), "xAOD event tree");
  m_outTree->SetDirectory(file);
  m_outTree->SetAutoSave(1000000);
  m_outTree->SetAutoFlush(autoFlush);

  // Access the EventFormat object associated with this file:
  m_outputEventFormat =
      &(TEventFormatRegistry::instance().getEventFormat(file));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function needs to be called when the user is done writing events
/// to a file, before (s)he would close the file itself.
///
/// @param file The file that the event data is written to
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::finishWritingTo(TFile& file) {
  return finishWritingTo(&file);
}

/// This function needs to be called when the user is done writing events
/// to a file, before (s)he would close the file itself.
///
/// @param file The file that the event data is written to
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::finishWritingTo(::TFile* file) {

  // A small sanity check:
  if (!m_outTree) {
    ATH_MSG_ERROR("The object doesn't seem to be connected to an output file!");
    return StatusCode::FAILURE;
  }

  // Make sure we return to the current directory:
  TDirectoryReset dr;

  // Notify the listeners that they should write out their metadata, if they
  // have any.
  const TIncident incident(IncidentType::MetaDataStop);
  for (auto &listener : m_listeners) {
    listener->handle(incident);
  }

  // Write out the event tree, and delete it:
  m_outTree->AutoSave("FlushBaskets");
  m_outTree->SetDirectory(0);
  m_outTree.reset();

  // Now go to the output file:
  file->cd();

  // Check if there's already a metadata tree in the output:
  if (file->Get(METADATA_OBJECT_NAME)) {
    // Let's assume that the metadata is complete in the file already.
    return StatusCode::SUCCESS;
  }

  // Create the metadata tree.
  auto metatree =
      std::make_unique<TTree>(METADATA_OBJECT_NAME, "xAOD metadata tree");
  metatree->SetAutoSave(10000);
  metatree->SetAutoFlush(-30000000);
  metatree->SetDirectory(file);

  // Create the xAOD::EventFormat branch in it.
  try {
    metatree->Branch(
        "EventFormat",
        SG::normalizedTypeinfoName(typeid(xAOD::EventFormat)).c_str(),
        &m_outputEventFormat);
  } catch (const CxxUtils::ClassName::ExcBadClassName &e) {
    ::Error("xAOD::TEvent::finishWritingTo",
            XAOD_MESSAGE("Class name parsing fails for %s ! "), e.what());
    return StatusCode::FAILURE;
  }

  // Create a copy of the m_outputMetaObjects variable. This is necessary
  // because the putAux(...) function will modify this variable while we
  // loop over it.
  std::vector<std::pair<std::string, TObjectManager* >> outputMetaObjects;
  outputMetaObjects.reserve(m_outputMetaObjects.size());
  for (const auto &[key, mgr] : m_outputMetaObjects) {
    TObjectManager* objMgr = dynamic_cast<TObjectManager* >(mgr.get());
    if (objMgr == nullptr) {
      ATH_MSG_FATAL("Internal logic error detected");
      return StatusCode::FAILURE;
    }
    outputMetaObjects.emplace_back(key, objMgr);
  }

  // Now loop over all the metadata objects that need to be put into the
  // output file:
  for (auto &[key, mgr] : outputMetaObjects) {

    // Select a split level depending on whether this is an interface or an
    // auxiliary object:
    const ::Int_t splitLevel = (key.ends_with("Aux.") ? 1 : 0);
    // Create the new branch:
    *(mgr->branchPtr()) =
        metatree->Branch(key.c_str(), mgr->holder()->getClass()->GetName(),
                         mgr->holder()->getPtr(), 32000, splitLevel);
    if (!mgr->branch()) {
      ATH_MSG_ERROR("Failed to create metadata branch \""
                    << mgr->holder()->getClass()->GetName() << "/" << key
                    << "\"");
      return StatusCode::FAILURE;
    }
    // Set up the saving of all the dynamic auxiliary properties
    // of the object if it has any:
    static constexpr bool METADATA = true;
    ATH_CHECK(putAux(*metatree, *mgr, METADATA));
  }

  // Write the metadata objects:
  if (metatree->Fill() <= 0) {
    ATH_MSG_ERROR("Failed to write event format metadata into the output");
    metatree->SetDirectory(nullptr);
    return StatusCode::FAILURE;
  }

  // Now clean up:
  metatree->Write();
  metatree->SetDirectory(nullptr);
  m_outputEventFormat = nullptr;
  m_outputObjects.clear();
  m_outputMetaObjects.clear();

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function can be used to create/retrieve a ROOT-specific auxiliary
/// object container that can be used to write information in the output
/// file. Any auxiliary information written this way will however only be
/// readable in ROOT, using the <code>kBranchAccess</code> option.
///
/// @param key The name/prefix of the auxiliary store object/branches
/// @param type The type of the auxiliary store (object/container)
/// @returns An auxiliary store object that will write to the output
///
SG::IAuxStore* TEvent::recordAux(const std::string &key,
                                 SG::IAuxStoreHolder::AuxStoreType type) {

  // A sanity check:
  if (!m_outTree) {
    ATH_MSG_ERROR("No output tree given to the object");
    return nullptr;
  }

  // Check for an object with this name in the output list:
  Object_t::iterator itr = m_outputObjects.find(key);
  if (itr == m_outputObjects.end()) {
    // Create one if if it doesn't exist yet...
    // Translate the store type:
    TAuxStore::EStructMode mode = TAuxStore::EStructMode::kUndefinedStore;
    switch (type) {
      case SG::IAuxStoreHolder::AST_ObjectStore:
        mode = TAuxStore::EStructMode::kObjectStore;
        break;
      case SG::IAuxStoreHolder::AST_ContainerStore:
        mode = TAuxStore::EStructMode::kContainerStore;
        break;
      default:
        ATH_MSG_ERROR("Unknown store type (" << type << ") requested");
        return nullptr;
    }
    // Create and record the object:
    static constexpr bool TOP_STORE = true;
    if (record(std::make_unique<TAuxStore>(key, TOP_STORE, mode), key)
            .isFailure()) {
      ATH_MSG_ERROR("Couldn't connect TAuxStore object to the output");
      return nullptr;
    }
    // Update the iterator:
    itr = m_outputObjects.find(key);
  }

  // A security check:
  if (itr == m_outputObjects.end()) {
    ATH_MSG_ERROR("Internal logic error detected");
    return nullptr;
  }

  // Check that it is of the right type:
  TAuxManager* mgr = dynamic_cast<TAuxManager* >(itr->second.get());
  if (!mgr) {
    ATH_MSG_ERROR("Internal logic error detected");
    return nullptr;
  }

  // Extract the pointer out of it:
  TAuxStore* store = mgr->getStore();

  // Give it to the user:
  return store;
}

/// @returns The number of events in the input file(s)
///
::Long64_t TEvent::getEntries() const {

  if (m_inChain) {
    return m_inChain->GetEntries();
  } else if (m_inTree) {
    return m_inTree->GetEntries();
  } else if (m_inTreeMissing) {
    // The input file is empty:
    return 0;
  } else {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return 0;
  }
}

/// This function is used to move to looking at a new entry from the input
/// tree. It doesn't do any I/O operation at this point, it just remembers
/// which entry the objects should be loaded from later on.
///
/// Unless one calls the function with <code>getall==1</code>. In that case
/// all input is force-read. This is necessary when writing out an event
/// that was processed in a load-on-request manner.
///
/// @param entry The entry from the input tree to load
/// @param getall Parameter deciding if partial reading should be used or
///               not.
/// @returns The number of bytes read, or a negative number in case of an
///          error
///
::Int_t TEvent::getEntry(::Long64_t entry, ::Int_t getall) {

  // A little sanity check:
  if ((!m_inTree) && (!m_inChain)) {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return -1;
  }

  // If we have a chain as input:
  if (m_inChain) {
    // Make sure that the correct tree is loaded:
    const ::Long64_t fileEntry = m_inChain->LoadTree(entry);
    if (fileEntry < 0) {
      ATH_MSG_ERROR("Failure in loading entry " << entry
                                                << " from the input chain");
      return -1;
    }
    // Check if a new file was loaded:
    if ((m_inTreeNumber != m_inChain->GetTreeNumber()) ||
        m_inChainTracker->internalStateChanged()) {
      // Reset the tracker:
      m_inChainTracker->reset();
      // Connect to this new file:
      m_inTreeNumber = m_inChain->GetTreeNumber();
      ::TFile* file = m_inChain->GetFile();
      // The useTreeCache parameter is set to false, since the cache
      // is anyway set up through the TChain. It shouldn't be modified
      // on the file level.
      static constexpr bool USE_TREE_CACHE = false;
      if (readFrom(file, USE_TREE_CACHE, m_inChain->GetName()).isFailure()) {
        ATH_MSG_ERROR("Couldn't connect to input file #"
                      << m_inTreeNumber << " of the input chain");
        return -1;
      }
    }
    // Restore the previously received entry number.
    m_entry = fileEntry;
  }
  // If we have a regular file/tree as input:
  else {
    m_entry = entry;
  }

  // In order to make the reading of branches+tree cache work
  // NB: TTree::LoadTree() only set the entry that should be read for each
  // branch but no reading of the branch content is performed when calling that
  // function. The entry set that can be retrieved with
  // branch->GetTree()->GetReadEntry()
  // For friend trees, if an index was built, then the entry which is set for
  // the related branches is found by the LoadTree function by matching the the
  // major and minor values of the main tree and friend tree
  if (m_inTree && m_inTree->LoadTree(m_entry) < 0) {
    ATH_MSG_ERROR("Failure in loading entry " << m_entry
                                              << " from the input file");
    return -1;
  }

  // Stats counter needs to know it's the next event:
  IOStats::instance().stats().nextEvent();

  // The final number of bytes read.
  ::Int_t result = 0;

  // Check if objects need to be read in.
  if (getall) {
    if (m_auxMode == kAthenaAccess) {
      // In kAthenaAccess mode we need to use getInputObject(...) to load
      // all the input objects correctly.
      for (auto &[key, mgr] : m_inputObjects) {
        static const std::string dynStorePostfix = "Aux.Dynamic";
        if (key.ends_with(dynStorePostfix)) {
          // Ignore the dynamic store objects. They get loaded through
          // their parents.
        } else {
          // Load the objects and their auxiliary stores through the
          // getInputObject(...) function, which takes care of correctly
          // setting them up. The type is irrelevant here. We don't
          // really care about the exact type of the objects.
          static constexpr bool SILENT = true;
          static constexpr bool METADATA = false;
          getInputObject(key, typeid(int), SILENT, METADATA);
        }
      }
    } else {
      // In a "reasonable" access mode, we do something very simple:
      for (auto &[key, mgr] : m_inputObjects) {
        result += mgr->getEntry(getall);
      }
    }
  }

  // Notify the listeners that a new event was loaded:
  const TIncident incident(IncidentType::BeginEvent);
  for (auto &listener : m_listeners) {
    listener->handle(incident);
  }

  // Return the number of bytes read:
  return result;
}

/// This function can be used to get the number of input files that the
/// object currently knows about. This is meant to be used to collect the
/// metadata from each of the input files, or to access the metadata using
/// a transient tree. (xAOD::TMetaTree)
///
/// @returns The number of input files when reading from a TChain, 1 when
///          reading from a TFile, and 0 if the object is not connected to
///          any input
///
::Long64_t TEvent::getFiles() const {

  if (m_inChain) {
    return m_inChain->GetListOfFiles()->GetEntries();
  } else if (m_inTree || m_inTreeMissing) {
    return 1;
  } else {
    return 0;
  }
}

/// This function can be used to iterate over the input files specified for
/// the object. It should be most useful for collecting metadata about a set
/// of input files, if there's no regular event loop done to process them.
///
/// @param file   The file number to load
/// @param getall If set to 1, all connected branches are loaded
/// @returns The number of read bytes on success, or a negative number on
///          failure
///
::Int_t TEvent::getFile(::Long64_t file, ::Int_t getall) {

  // Check if the file number is valid:
  if ((file < 0) || (file >= getFiles())) {
    ATH_MSG_ERROR("Function called with invalid file number (" << file << ")");
    return -1;
  }

  // If we are not reading a TChain, return at this point. As the one and
  // only file is open already...
  if (!m_inChain) {
    return 0;
  }

  // Trigger the "scanning" of the input files, so the TChain would know
  // how many entries are in the various files.
  getEntries();

  // Calculate which entry/event we need to load:
  ::Long64_t entry = 0;
  for (::Long64_t i = 0; i < file; ++i) {
    entry += m_inChain->GetTreeOffset()[i];
  }

  // Load this entry using the regular event opening function:
  return getEntry(entry, getall);
}

/// This function needs to be called by the user at the end of processing
/// each event that is meant to be written out.
///
/// @returns The number of bytes written if successful, a negative number
///          if not
///
::Int_t TEvent::fill() {

  // A little sanity check:
  if (!m_outTree) {
    ATH_MSG_ERROR("Object not connected to an output file!");
    return 0;
  }

  // Make sure that all objects have been read in. The 99 as the value
  // has a special meaning for TAuxStore. With this value it doesn't
  // delete its transient (decoration) variables. Otherwise it does.
  // (As it's supposed to, when moving to a new event.)
  Int_t readBytes = 0;
  if (m_inChain != nullptr) {
    readBytes = getEntry(m_inChain->GetReadEntry(), 99);
  } else if (m_inTree != nullptr) {
    readBytes = getEntry(m_entry, 99);
  }
  if (readBytes < 0) {
    ATH_MSG_ERROR("getEntry failed!");
    return readBytes;
  }

  // Prepare the objects for writing. Note that we need to iterate over a
  // copy of the m_outputObjects container. Since the putAux(...) function
  // called inside the loop may itself add elements to the m_outputObject
  // container.
  std::string unsetObjects;
  std::vector<std::pair<std::string, TVirtualManager* >> outputObjectsCopy;
  outputObjectsCopy.reserve(m_outputObjects.size());
  for (const auto &[key, mgr] : m_outputObjects) {
    outputObjectsCopy.emplace_back(key, mgr.get());
  }
  for (auto &[key, mgr] : outputObjectsCopy) {
    // Check that a new object was provided in the event:
    if (!mgr->create()) {
      // We are now going to fail. But let's collect the names of
      // all the unset objects:
      if (unsetObjects.size()) {
        unsetObjects += ", ";
      }
      unsetObjects.append("\"" + key + "\"");
      continue;
    }
    // Make sure that any dynamic auxiliary variables that
    // were added to the object after it was put into the event,
    // get added to the output:
    static constexpr bool METADATA = false;
    if (putAux(*m_outTree, *mgr, METADATA).isFailure()) {
      ATH_MSG_ERROR("Failed to put dynamic auxiliary variables "
                    "in the output for object \""
                    << key << "\"");
      return 0;
    }
  }

  // Check if there were any unset objects:
  if (unsetObjects.size()) {
    ATH_MSG_ERROR("The following objects were not set in the current event: "
                  << unsetObjects);
    return 0;
  }

  // Write the entry, and check the return value:
  const ::Int_t ret = m_outTree->Fill();
  if (ret <= 0) {
    ATH_MSG_ERROR("Output tree filling failed with return value: " << ret);
  }

  // Reset the object managers.
  for (auto &[key, mgr] : m_outputObjects) {
    mgr->reset();
  }

  // Return the value:
  return ret;
}

bool TEvent::hasInput() const {

  return ((m_inTree != nullptr) || (m_inChain != nullptr));
}

bool TEvent::hasOutput() const { return (m_outTree.get() != nullptr); }

StatusCode TEvent::getNames(const std::string &targetClassName,
                            std::vector<std::string> &vkeys,
                            bool metadata) const {
  // The results go in here
  std::set<std::string> keys;

  // Get list of branches from
  // the input metadata tree or input tree
  std::vector<TObjArray* > fullListOfBranches = {};
  if (metadata) {
    if (m_inMetaTree) {
      // No friend tree expected for metadata tree
      // Only add the list of branches of the metadata tree
      ATH_MSG_DEBUG("Scanning for input metadata objects");
      fullListOfBranches.push_back(m_inMetaTree->GetListOfBranches());
    }
  } else {
    if (m_inTree) {
      ATH_MSG_DEBUG("Scanning for input data objects");
      // Add the list of branches of the main tree
      fullListOfBranches.push_back(m_inTree->GetListOfBranches());
      // If the input tree has friend trees
      // add as well the list of friend tree branches
      if (m_inTree->GetListOfFriends()) {
        // Get the list of friends
        TList* fList = m_inTree->GetListOfFriends();
        // Loop over friend elements
        for (TObject* feObj : *fList) {
          if (feObj) {
            // Get corresponding friend tree
            auto* pElement = dynamic_cast<TFriendElement* >(feObj);
            if (pElement == nullptr) {
              continue;
            }
            TTree* friendTree = pElement->GetTree();
            // Add list of branches of the friend tree
            fullListOfBranches.push_back(friendTree->GetListOfBranches());
          }
        }
      }
    }
  }

  // Loop over all list of branches (if any)
  for (const TObjArray* in : fullListOfBranches) {
    // Loop over all branches inside the current list of branches
    for (const TObject* obj : *in) {

      if (obj == nullptr) {
        continue;
      }
      const TBranch* element = dynamic_cast<const TBranch* >(obj);
      if (!element) {
        ATH_MSG_ERROR("Failure inspecting input data objects");
        return StatusCode::FAILURE;
      }
      const std::string objClassName = element->GetClassName();
      std::string key = obj->GetName();
      ATH_MSG_VERBOSE("Inspecting \"" << objClassName << "\" / \"" << key
                                      << "\"");
      if (objClassName == targetClassName) {
        ATH_MSG_DEBUG("Matched \"" << targetClassName << "\" to key \"" << key
                                   << "\"");
        keys.insert(std::move(key));
      }
    }
  }

  const Object_t &inAux = (metadata ? m_inputMetaObjects : m_inputObjects);

  ATH_MSG_DEBUG("Scanning input objects for \"" << targetClassName << "\"");
  for (const auto &[key, vmgr] : inAux) {
    // All (metadata) objects should be held by TObjectManager objects.
    const TObjectManager* mgr =
        dynamic_cast<const TObjectManager* >(vmgr.get());
    if (mgr == nullptr) {
      continue;
    }
    const std::string &objClassName = mgr->holder()->getClass()->GetName();
    ATH_MSG_VERBOSE("Inspecting \"" << objClassName << "\" / \"" << key
                                    << "\"");
    if (objClassName == targetClassName) {
      ATH_MSG_DEBUG("Matched \"" << targetClassName << "\" to key \"" << key
                                 << "\"");
      keys.insert(key);
    }
  }

  // Check for output objects.
  if ((metadata == false) && m_outTree) {
    const TObjArray* out = m_outTree->GetListOfBranches();
    ATH_MSG_DEBUG("Scanning for output data objects");

    for (const TObject* obj : *out) {
      if (obj == nullptr) {
        continue;
      }
      const TBranch* element = dynamic_cast<const TBranch* >(obj);
      if (element == nullptr) {
        ATH_MSG_ERROR("Failure inspecting output objects");
        return StatusCode::FAILURE;
      }
      const std::string objClassName = element->GetClassName();
      std::string key = obj->GetName();
      ATH_MSG_VERBOSE("Inspecting \"" << objClassName << "\" / \"" << key
                                      << "\"");
      if (objClassName == targetClassName) {
        ATH_MSG_DEBUG("Matched \"" << targetClassName << "\" to key \"" << key
                                   << "\"");
        keys.insert(std::move(key));
      }
    }
  }

  const Object_t &outAux = (metadata ? m_outputMetaObjects : m_outputObjects);

  // Search though the in-memory output objects.
  ATH_MSG_DEBUG("Scanning output objects for \"" << targetClassName << "\"");
  for (const auto &[key, vmgr] : outAux) {
    // All (metadata) objects should be held by TObjectManager objects.
    TObjectManager* mgr = dynamic_cast<TObjectManager* >(vmgr.get());
    if (mgr == nullptr) {
      continue;
    }
    const std::string &objClassName = mgr->holder()->getClass()->GetName();
    ATH_MSG_VERBOSE("Inspecting \"" << objClassName << "\" / \"" << key
                                    << "\"");
    if (objClassName == targetClassName) {
      ATH_MSG_DEBUG("Matched \"" << targetClassName << "\" to key \"" << key
                                 << "\"");
      keys.insert(key);
    }
  }

  vkeys.insert(vkeys.end(), keys.begin(), keys.end());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// This is one of the more important functions of the class. It connects the
/// event object to a given branch of the input TTree.
///
/// The type that is read in to memory is not actually determined by the type
/// written to the tree itself, but from the xAOD::EventFormat object. Which
/// is there to make it possible to possibly use ROOT read rules in the
/// future to read a different type than what's on disk.
///
/// Everything else is fairly basic ROOT stuff, the code just has to make
/// sure that the memory management is set up correctly for the created
/// object.
///
/// @param key The name of the branch to connect to
/// @param silent Set to @c true to make the code fail silently
///               in case the branch can't be connected to
/// @return The usual @c StatusCode types
///
StatusCode TEvent::connectObject(const std::string &key, bool silent) {

  // A little sanity check:
  if (hasInput() == false) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Increment the access counter on this container:
  IOStats::instance().stats().readContainer(key);

  // Check if the branch is already connected:
  if (m_inputObjects.contains(key)) {
    return StatusCode::SUCCESS;
  }
  // Check if it was already found to be missing.
  if (m_inputMissingObjects.contains(key)) {
    if (silent == false) {
      ATH_MSG_WARNING("Branch \"" << key << "\" not available on input");
    }
    return StatusCode::RECOVERABLE;
  }

  // Check if we have metadata about this branch:
  const xAOD::EventFormatElement* ef = nullptr;
  if (m_inputEventFormat.exists(key) == false) {
    if (silent == false) {
      ATH_MSG_WARNING("No metadata available for branch: " << key);
    }
  } else {
    ef = m_inputEventFormat.get(key);
  }

  // Check if the branch exists in our input tree:
  ::TBranch* br = m_inTree->GetBranch(key.c_str());
  if (br == nullptr) {
    if (!silent) {
      ATH_MSG_WARNING("Branch \"" << key << "\" not available on input");
    }
    m_inputMissingObjects.insert(key);
    return StatusCode::RECOVERABLE;
  }

  // Make sure that it's not in "MakeClass mode":
  br->SetMakeClass(0);

  // Decide about the type that we need to use for the reading of this
  // branch:
  std::string className = br->GetClassName();
  if (className == "") {
    if (ef) {
      // This is a fairly weird situation, but let's fall back to taking
      // the class name from the metadata object in this case.
      className = ef->className();
    } else {
      ATH_MSG_ERROR(
          "Couldn't find an appropriate type with a dictionary for branch \""
          << key << "\"");
      return StatusCode::FAILURE;
    }
  }
  ::TClass* realClass = ::TClass::GetClass(className.c_str());
  if (((!realClass) || (!realClass->IsLoaded())) && ef) {
    // We may need to do an actual schema evolution here, in which
    // case let's fall back on the class name coming from the metadata
    // object.
    className = ef->className();
    realClass = ::TClass::GetClass(className.c_str());
  }
  if ((!realClass) || (!realClass->IsLoaded())) {
    // Now we're in trouble...
    ATH_MSG_ERROR(
        "Couldn't find an appropriate type with a dictionary for branch \""
        << key << "\"");
    return StatusCode::FAILURE;
  }

  // Make sure that the current object is the "active event":
  setActive();

  // The data type is always "other" for us:
  static const ::EDataType dataType = kOther_t;

  // Check if the output already has this object. If it does, let's
  // assume that we have been copying the object to the output. Which
  // means that we need to resume filling the same memory address that
  // the output holder points to.
  void* ptr = nullptr;
  Object_t::const_iterator out_itr = m_outputObjects.find(key);
  if (out_itr != m_outputObjects.end()) {
    // It needs to be an object manager...
    TObjectManager* mgr = dynamic_cast<TObjectManager* >(out_itr->second.get());
    if (mgr == nullptr) {
      ATH_MSG_ERROR("Couldn't access output manager for: " << key);
      return StatusCode::FAILURE;
    }
    // Get the pointer out of it:
    ptr = mgr->holder()->get();
  }

  // If there is no output object, then let's create one ourselves.
  // This is the only way in which we can have the memory management of
  // THolder do the right thing with this object.
  if (ptr == nullptr) {
    ptr = realClass->New();
  }

  // Create the new manager object that will hold this EDM object:
  const bool renewOnRead = (m_auxMode == kAthenaAccess);
  auto mgr = std::make_unique<TObjectManager>(
      nullptr, std::make_unique<THolder>(ptr, realClass), renewOnRead);

  // One final check. If it's not an auxiliary store, then it must have
  // a split level of 0. Otherwise read rules may not work on it. Causing
  // *very* serious silent corruption in the data read, if we don't use
  // the "Athena read mode".
  if ((m_auxMode != kAthenaAccess) && (br->GetSplitLevel() != 0) &&
      (Details::isAuxStore(*(mgr->holder()->getClass())) == false)) {
    ATH_MSG_ERROR("Split level for branch \""
                  << key << "\" is " << br->GetSplitLevel()
                  << ". This can only be read in kAthenaAccess mode.");
    // Clean up:
    *(mgr->holder()->getPtr()) = nullptr;
    m_inputObjects.erase(key);
    return StatusCode::FAILURE;
  }

  // Now try to connect to the branch:
  const ::Int_t status =
      m_inTree->SetBranchAddress(key.c_str(), mgr->holder()->getPtr(),
                                 mgr->branchPtr(), realClass, dataType, kTRUE);
  if (status < 0) {
    ATH_MSG_ERROR("Couldn't connect variable of type \""
                  << className << "\" to input branch \"" << key
                  << "\". Return code: " << status);
    // Clean up:
    *(mgr->holder()->getPtr()) = 0;
    m_inputObjects.erase(key);
    return StatusCode::FAILURE;
  }

  // At this point we have successfully connected the branch.
  TObjectManager* mgrPtr = mgr.get();
  m_inputObjects[key] = std::move(mgr);

  // If it's an auxiliary store object, set it up correctly:
  if (Details::isAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(setUpDynamicStore(*mgrPtr, m_inTree));
  }

  // If there may be an auxiliary object connected to this one,
  // connect that as well:
  if (Details::hasAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(connectAux(
        key + "Aux.", Details::isStandalone(*(mgrPtr->holder()->getClass()))));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// This is the function doing the heavy lifting with creating metadata
/// objects in memory out of the payload of the input file.
///
/// @param key The key (branch name) of the metadata object to retrieve
/// @param silent Set to <code>kTRUE</code> to make the code fail silently
///               in case the branch can't be connected to
/// @returns The usual @c StatusCode types
///
StatusCode TEvent::connectMetaObject(const std::string &key, bool silent) {

  // A little sanity check:
  if (!m_inMetaTree) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Check if the branch is already connected:
  if (m_inputMetaObjects.contains(key)) {
    return StatusCode::SUCCESS;
  }

  // Check if the branch exists in our metadata tree:
  ::TBranch* br = m_inMetaTree->GetBranch(key.c_str());
  if (br == nullptr) {
    if (silent == false) {
      ATH_MSG_WARNING("Metadata branch \"" << key
                                           << "\" not available on input");
    }
    return StatusCode::RECOVERABLE;
  }

  // Check that we have an entry in the branch:
  if (br->GetEntries() == 0) {
    if (silent == false) {
      ATH_MSG_WARNING("Metadata branch \"" << key
                                           << "\" doesn't hold any data");
    }
    return StatusCode::RECOVERABLE;
  }

  // Make sure that it's not in "MakeClass mode":
  br->SetMakeClass(0);

  // Extract the type of the branch:
  ::TClass* cl = 0;
  ::EDataType dt = kOther_t;
  if (br->GetExpectedType(cl, dt) || (!cl)) {
    ATH_MSG_ERROR("Couldn't get the type for metadata branch \"" << key
                                                                 << "\"");
    return StatusCode::FAILURE;
  }

  // Create the object, and all of the managers around it:
  void* ptr = cl->New();
  const bool renewOnRead = (m_auxMode == kAthenaAccess);
  auto mgr = std::make_unique<TObjectManager>(
      nullptr, std::make_unique<THolder>(ptr, cl), renewOnRead);

  // Now try to connect to the branch:
  const ::Int_t status = m_inMetaTree->SetBranchAddress(
      key.c_str(), mgr->holder()->getPtr(), mgr->branchPtr(), cl, dt, kTRUE);
  if (status < 0) {
    ATH_MSG_ERROR("Couldn't connect variable of type \""
                  << cl->GetName() << "\" to input branch \"" << key
                  << "\". Return code: " << status);
    // Clean up:
    *(mgr->holder()->getPtr()) = 0;
    m_inputMetaObjects.erase(key);
    return StatusCode::FAILURE;
  }

  // Store the manager.
  TObjectManager *mgrPtr = mgr.get();
  m_inputMetaObjects[key] = std::move(mgr);

  // Read in the object:
  if (mgrPtr->getEntry() < 0) {
    ATH_MSG_ERROR("Couldn't read in metadata object with key \"" << key
                                                                 << "\"");
    return StatusCode::FAILURE;
  }

  // If it's an auxiliary store object, set it up correctly:
  if (Details::isAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(setUpDynamicStore(*mgrPtr, m_inMetaTree));
  }

  // If there may be an auxiliary object connected to this one,
  // connect that as well.
  if (Details::hasAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(connectMetaAux(
        key + "Aux.", Details::isStandalone(*(mgrPtr->holder()->getClass()))));
    static constexpr bool METADATA = true;
    ATH_CHECK(setAuxStore(key, *mgrPtr, METADATA));
  }

  // We succeeded:
  return StatusCode::SUCCESS;
}

/// This function is used internally to connect an auxiliary object to
/// the input. Based on the configuration of the object it will either
/// use TAuxStore, or the EDM object that was used to write the auxiliary
/// information in Athena.
///
/// @param prefix The prefix (main branch name) of the auxiliary data
/// @param standalone Type of the auxiliary store that should be created
/// @return The usual @c StatusCode types
///
StatusCode TEvent::connectAux(const std::string &prefix, bool standalone) {

  // A simple test...
  if (hasInput() == false) {
    ATH_MSG_ERROR("No input tree is available");
    return StatusCode::FAILURE;
  }

  // Check if we know anything about this auxiliary object:
  if ((!m_inTree->GetBranch(prefix.c_str())) &&
      (m_auxMode == kClassAccess || m_auxMode == kAthenaAccess)) {
    // If not, then let's just return right away. Not having
    // an auxiliary object with this name is not an error per se.
    return StatusCode::SUCCESS;
  }

  // Check if the branch is already connected.
  if (m_inputObjects.contains(prefix)) {
    return StatusCode::SUCCESS;
  }

  // Do different things based on the "auxiliary mode" we are in.
  if ((m_auxMode == kClassAccess) || (m_auxMode == kAthenaAccess)) {

    // In "class" and "athena" access modes just connect the concrete auxiliary
    // object to the input.
    static constexpr bool SILENT = false;
    ATH_CHECK(connectObject(prefix, SILENT));

    // Return gracefully.
    return StatusCode::SUCCESS;

  } else if (m_auxMode == kBranchAccess) {

    // In "branch access mode" let's create a TAuxStore object, and let
    // that take care of the auxiliary store access.
    static constexpr bool TOP_STORE = true;
    auto store = std::make_unique<TAuxStore>(
        prefix, TOP_STORE,
        (standalone ? TAuxStore::EStructMode::kObjectStore
                    : TAuxStore::EStructMode::kContainerStore));

    // Connect it to the input tree.
    ATH_CHECK(store->readFrom(*m_inTree));

    // We're using this object to read from the input, it needs to be
    // locked:
    store->lock();

    // Finally, set up an appropriate manager for it.
    static constexpr bool IS_OWNER = true;
    m_inputObjects[prefix] =
        std::make_unique<TAuxManager>(store.release(), IS_OWNER);

    // Return gracefully:
    return StatusCode::SUCCESS;
  }

  // There was some problem:
  ATH_MSG_ERROR("Unknown auxiliary access mode set (" << m_auxMode << ")");
  return StatusCode::FAILURE;
}

/// This function is used internally to connect an auxiliary metadata object
/// to the input. Based on the configuration of the object it will either
/// use TAuxStore, or the EDM object that was used to write the auxiliary
/// information in Athena.
///
/// @param prefix The prefix (main branch name) of the auxiliary data
/// @param standalone Type of the auxiliary store that should be created
/// @return The usual @c StatusCode types
///
StatusCode TEvent::connectMetaAux(const std::string &prefix, bool standalone) {

  // Check if the branch is already connected:
  if (m_inputMetaObjects.contains(prefix)) {
    return StatusCode::SUCCESS;
  }

  // A sanity check:
  if (!m_inMetaTree) {
    ATH_MSG_FATAL("Internal logic error detected");
    return StatusCode::FAILURE;
  }

  // Do different things based on the "auxiliary mode" we are in:
  if (m_auxMode == kClassAccess || m_auxMode == kAthenaAccess) {

    // In "class" and "athena" access modes just connect the concrete auxiliary
    // object to the input.
    static constexpr bool SILENT = false;
    ATH_CHECK(connectMetaObject(prefix, SILENT));

    // Return gracefully:
    return StatusCode::SUCCESS;

  } else if (m_auxMode == kBranchAccess) {

    // In "branch access mode" let's create a TAuxStore object, and let
    // that take care of the auxiliary store access.
    static constexpr bool TOP_STORE = true;
    auto store = std::make_unique<TAuxStore>(
        prefix, TOP_STORE,
        (standalone ? TAuxStore::EStructMode::kObjectStore
                    : TAuxStore::EStructMode::kContainerStore));

    // Connect it to the input tree.
    ATH_CHECK(store->readFrom(*m_inMetaTree));

    // We're using this object to read from the input, it needs to be
    // locked:
    store->lock();

    // Finally, set up an appropriate manager for it.
    static constexpr bool IS_OWNER = true;
    m_inputMetaObjects[prefix] =
        std::make_unique<TAuxManager>(store.release(), IS_OWNER);

    // Return gracefully.
    return StatusCode::SUCCESS;
  }

  // There was some problem:
  ATH_MSG_ERROR("Unknown auxiliary access mode set (" << m_auxMode << ")");
  return StatusCode::FAILURE;
}

/// Every time a DataVector is read in from the input for a new TTree
/// entry, one needs to re-connect it with its auxiliary store. This
/// function takes care of this.
///
/// @param mgr The manager object of the DV container
/// @param metadata Flag specifying whether we're dealing with a metadata
///                 or event data object
/// @return The usual @c StatusCode types
///
StatusCode TEvent::setAuxStore(const std::string &key,
                               Details::IObjectManager &mgr, bool metadata) {

  // Pre-compute some values.
  const bool isAuxStore = Details::isAuxStore(*(mgr.holder()->getClass()));

  // Check if we need to do anything.
  if ((Details::hasAuxStore(*(mgr.holder()->getClass())) == false) &&
      (isAuxStore == false)) {
    return StatusCode::SUCCESS;
  }

  // Select which object container to use:
  Object_t &objects = (metadata ? m_inputMetaObjects : m_inputObjects);

  // Look up the auxiliary object's manager:
  TVirtualManager* auxMgr = nullptr;
  std::string auxKey;
  if (isAuxStore) {
    auxMgr = &mgr;
    auxKey = key;
  } else {
    auto itr = objects.find(key + "Aux.");
    if (itr == objects.end()) {
      // Apparently there's no auxiliary object for this DV, so let's
      // give up:
      return StatusCode::SUCCESS;
    }
    auxMgr = itr->second.get();
    auxKey = key + "Aux.";
  }

  if (metadata == false) {
    // Make sure the auxiliary object is up to date:
    const ::Int_t readBytes = auxMgr->getEntry();
    if (readBytes < 0) {
      ATH_MSG_ERROR(
          "Couldn't load current entry for auxiliary object with key \""
          << auxKey << "\"");
      return StatusCode::FAILURE;
    }

    // Check if there is a separate auxiliary object for the dynamic
    // variables:
    const std::string dynAuxKey = auxKey + "Dynamic";
    auto dynAuxMgr = objects.find(dynAuxKey);

    if ((dynAuxMgr != objects.end()) &&
        (readBytes || (m_auxMode == kAthenaAccess) || (auxMgr == &mgr))) {
      // Do different things based on the access mode:
      if (m_auxMode != kAthenaAccess) {
        // In "normal" access modes just tell the dynamic store object
        // to switch to a new event.
        dynAuxMgr->second->getEntry();
      } else {
        // In "Athena mode" this object has already been deleted when
        // the main auxiliary store object was switched to the new
        // event. So let's re-create it:
        xAOD::TObjectManager &auxMgrRef =
            dynamic_cast<xAOD::TObjectManager &>(*auxMgr);
        ATH_CHECK(
            setUpDynamicStore(auxMgrRef, (metadata ? m_inMetaTree : m_inTree)));
        // Now tell the newly created dynamic store object which event
        // it should be looking at:
        auto dynAuxMgr = objects.find(dynAuxKey);
        if (dynAuxMgr == objects.end()) {
          ATH_MSG_ERROR("Internal logic error detected");
          return StatusCode::FAILURE;
        }
        dynAuxMgr->second->getEntry();
      }
    }
  }

  // Stop here if we've set up an auxiliary store.
  if (isAuxStore) {
    return StatusCode::SUCCESS;
  }

  // Access the auxiliary base class of the object/vector:
  SG::AuxVectorBase* vec = 0;
  SG::AuxElement* aux = 0;
  switch (mgr.holder()->typeKind()) {
    case THolder::DATAVECTOR: {
      void* vvec = mgr.holder()->getAs(typeid(SG::AuxVectorBase));
      vec = reinterpret_cast<SG::AuxVectorBase*>(vvec);
    } break;
    case THolder::AUXELEMENT: {
      void* vaux = mgr.holder()->getAs(typeid(SG::AuxElement));
      aux = reinterpret_cast<SG::AuxElement*>(vaux);
    } break;
    default:
      break;
  }

  // Check whether index tracking is enabled for the type. If not, then
  // we need to fix it...
  if (vec && (!vec->trackIndices())) {
    Details::forceTrackIndices(*vec);
  }

  // Check if we were successful:
  if ((!vec) && (!aux)) {
    ATH_MSG_FATAL("Couldn't access class \""
                  << mgr.holder()->getClass()->GetName()
                  << "\" as SG::AuxVectorBase or SG::AuxElement");
    return StatusCode::FAILURE;
  }

  // Get the auxiliary store object:
  const SG::IConstAuxStore* store = 0;
  if (m_auxMode == kBranchAccess) {
    // Get the concrete auxiliary manager:
    TAuxManager* amgr = dynamic_cast<TAuxManager* >(auxMgr);
    if (!amgr) {
      ATH_MSG_FATAL("Auxiliary manager for \""
                    << auxKey << "\" is not of the right type");
      return StatusCode::FAILURE;
    }
    store = amgr->getConstStore();
    // If the store still doesn't know its type, help it now:
    if (amgr->getStore()->structMode() ==
        TAuxStore::EStructMode::kUndefinedStore) {
      const TAuxStore::EStructMode mode =
          (vec ? TAuxStore::EStructMode::kContainerStore
               : TAuxStore::EStructMode::kObjectStore);
      amgr->getStore()->setStructMode(mode);
    }
  } else if (m_auxMode == kClassAccess || m_auxMode == kAthenaAccess) {
    // Get the concrete auxiliary manager:
    TObjectManager* omgr = dynamic_cast<TObjectManager* >(auxMgr);
    if (!omgr) {
      ATH_MSG_FATAL("Auxiliary manager for \""
                    << auxKey << "\" is not of the right type");
      return StatusCode::FAILURE;
    }
    void* p = omgr->holder()->getAs(typeid(SG::IConstAuxStore));
    store = reinterpret_cast<const SG::IConstAuxStore* >(p);
  }
  if (!store) {
    ATH_MSG_FATAL("Logic error detected in the code");
    return StatusCode::FAILURE;
  }

  // Connect the two:
  if (vec) {
    vec->setStore(store);
  } else if (aux) {
    aux->setStore(store);
  } else {
    ATH_MSG_FATAL("Logic error detected in the code");
    return StatusCode::FAILURE;
  }

  // We succeeded:
  return StatusCode::SUCCESS;
}

/// This is the function doing the heavy lifting when recording a new
/// object into the output tree/file. It makes sure that the object is
/// saved together with all of its dynamic auxiliary data if it has any.
///
/// @param obj A typeless pointer to the object that we want to record
/// @param typeName The type name of the output object
/// @param key The key (branch name) of the object to record
/// @param overwrite Flag selecting if it is allowed to overwrite an
///                  already existing object (used internally)
/// @param metadata Flag selecting if we are writing an event or a
///                 metadata object
/// @param isOwner Flag selecting if we should take ownership of the object
///                or not
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::record(void* obj, const std::string &typeName,
                          const std::string &key, bool overwrite, bool metadata,
                          bool isOwner) {

  // Check if we have an output tree when writing an event:
  if (!m_outTree && !metadata) {
    ATH_MSG_ERROR(
        "No output tree defined. Did you forget to call writeTo(...)?");
    return StatusCode::FAILURE;
  }
  assert(m_outputEventFormat != 0);

  // If this is metadata, just take ownership of it. The object will only
  // be recorded into the output file when calling finishWritingTo(...).
  if (metadata) {
    // Check whether we already have such an object:
    if ((!overwrite) &&
        (m_outputMetaObjects.find(key) != m_outputMetaObjects.end())) {
      ATH_MSG_ERROR("Meta-object \"" << typeName << "\"/\"" << key
                                     << "\" already recorded");
      return StatusCode::FAILURE;
    }
    // Check if we have a dictionary for this object:
    TClass* cl = TClass::GetClass(typeName.c_str());
    if (!cl) {
      ATH_MSG_ERROR("Didn't find dictionary for type: " << typeName);
      return StatusCode::FAILURE;
    }
    // Let's create a holder for the object:
    const bool renewOnRead = (m_auxMode == kAthenaAccess);
    m_outputMetaObjects[key] = std::make_unique<TObjectManager>(
        nullptr, std::make_unique<THolder>(obj, cl, isOwner), renewOnRead);
    // We're done. The rest will be done later on.
    return StatusCode::SUCCESS;
  }

  // Check if we accessed this object on the input. If yes, then this
  // key may not be used for recording.
  if ((!overwrite) && (m_inputObjects.find(key) != m_inputObjects.end())) {
    ATH_MSG_ERROR("Object \"" << typeName << "\"/\"" << key
                              << "\" already accessed from the input, can't be "
                                 "overwritten in memory");
    return StatusCode::FAILURE;
  }

  // Choose a split level.
  const Int_t splitLevel = (key.ends_with("Aux.") ? 1 : 0);

  // Check if we need to add it to the event record:
  Object_t::iterator vitr = m_outputObjects.find(key);
  if (vitr == m_outputObjects.end()) {

    // Check if we have a dictionary for this object:
    TClass* cl = TClass::GetClass(typeName.c_str());
    if (cl == nullptr) {
      ATH_MSG_ERROR("Didn't find dictionary for type: " << typeName);
      return StatusCode::FAILURE;
    }

    // Check if this is a new object "type" or not.
    if (!m_outputEventFormat->exists(key)) {
      m_outputEventFormat->add(
          EventFormatElement(key, cl->GetName(), "", getHash(key)));
    }

    // Let's create a holder for the object.
    const bool renewOnRead = (m_auxMode == kAthenaAccess);
    auto mgr = std::make_unique<TObjectManager>(
        nullptr, std::make_unique<THolder>(obj, cl, isOwner), renewOnRead);
    TObjectManager* mgrPtr = mgr.get();
    m_outputObjects[key] = std::move(mgr);

    // ... and let's add it to the output TTree.
    static constexpr Int_t basketSize = 32000;
    *(mgrPtr->branchPtr()) =
        m_outTree->Branch(key.c_str(), cl->GetName(),
                          mgrPtr->holder()->getPtr(), basketSize, splitLevel);
    if (!mgrPtr->branch()) {
      ATH_MSG_ERROR("Failed to create branch \"" << key << "\" out of type \""
                                                 << cl->GetName() << "\"");
      // Clean up:
      mgrPtr->holder()->setOwner(kFALSE);
      return StatusCode::FAILURE;
    }

    // Set up the saving of all the dynamic auxiliary properties
    // of the object if it has any:
    static constexpr bool METADATA = false;
    ATH_CHECK(putAux(*m_outTree, *mgrPtr, METADATA));

    // Return at this point, as we don't want to run the rest of
    // the function's code:
    return StatusCode::SUCCESS;
  }

  // Access the object manager:
  TObjectManager* omgr = dynamic_cast<TObjectManager* >(vitr->second.get());
  if (!omgr) {
    ATH_MSG_ERROR("Manager object of the wrong type encountered");
    return StatusCode::FAILURE;
  }

  // Check that the type of the object matches that of the previous
  // object:
  if (typeName != omgr->holder()->getClass()->GetName()) {
    // This may still be, when the ROOT dictionary name differs from the
    // "simple type name" known to C++. So let's get the ROOT name of the
    // new type:
    TClass* cl = TClass::GetClass(typeName.c_str());
    if ((!cl) ||
        ::strcmp(cl->GetName(), omgr->holder()->getClass()->GetName())) {
      ATH_MSG_ERROR("For output key \""
                    << key << "\" the previous type was \""
                    << omgr->holder()->getClass()->GetName()
                    << "\", but the newly requested type is \"" << typeName
                    << "\"");
      return StatusCode::FAILURE;
    }
  }

  // Replace the managed object.
  omgr->setObject(obj);

  // Replace the auxiliary objects.
  static constexpr bool METADATA = false;
  ATH_CHECK(putAux(*m_outTree, *omgr, METADATA));

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TEvent::recordAux(TVirtualManager &mgr, const std::string &key,
                             bool metadata) {

  // Check if the auxiliary store is a generic object.
  Details::IObjectManager* iomgr = dynamic_cast<Details::IObjectManager*>(&mgr);
  if (iomgr != nullptr) {
    // Record the auxiliary object using the main record function.
    static const bool OVERWRITE = true;
    static const bool IS_OWNER = true;
    ATH_CHECK(record(iomgr->object(), iomgr->holder()->getClass()->GetName(),
                     key, OVERWRITE, metadata, IS_OWNER));
    return StatusCode::SUCCESS;
  }

  // Check if it's a TAuxStore object.
  TAuxManager* auxmgr = dynamic_cast<TAuxManager* >(&mgr);
  if (auxmgr != nullptr) {
    // This type has to be an event object.
    if (metadata) {
      ATH_MSG_ERROR(
          "TAuxStore auxiliary objects can only be recorded for event data");
      return StatusCode::FAILURE;
    }
    // Record the auxiliary object with the dedicated record function.
    ATH_CHECK(recordAux(auxmgr->getStore(), key));
    return StatusCode::SUCCESS;
  }

  // Apparently we didn't recorgnize the auxiliary store type.
  ATH_MSG_ERROR("Unknown auxiliary store manager type encountered");
  return StatusCode::FAILURE;
}

/// This function is used internally to initialise the reading of an input
/// file. It prepares the "monitoring information" in memory that gets filled
/// while the code is running, with information about xAOD I/O.
///
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::initStats() {

  // If we're dealing with an empty input file, stop here:
  if (m_inTreeMissing) {
    return StatusCode::SUCCESS;
  }

  // A little sanity check:
  if (!m_inTree) {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return StatusCode::FAILURE;
  }

  // Reset the number of input branches information:
  IOStats::instance().stats().setBranchNum(0);

  // Loop over the EventFormat information
  EventFormat::const_iterator itr = m_inputEventFormat.begin();
  EventFormat::const_iterator end = m_inputEventFormat.end();
  for (; itr != end; ++itr) {

    // Get the name of the branch in question:
    const std::string &branchName = itr->second.branchName();

    // If it's an auxiliary container, scan it using TAuxStore:
    if (branchName.find("Aux.") != std::string::npos) {

      // But first decide whether it describes a container, or just
      // a single object. Since the file may have been written in
      // kBranchAccess mode, it's not necessarily a good idea to check
      // the type of the auxiliary class. So let's check the interface
      // class instead.
      //
      // Get the name of the interface object/container:
      const std::string intName = branchName.substr(0, branchName.size() - 4);
      if (!m_inputEventFormat.exists(intName)) {
        // When this happens, it may still be that both the interface and
        // the auxiliary container is missing from the file. As we didn't
        // check yet whether the auxiliary container is in place or not.
        // So, before printing a warning, let's check for this.
        // Unfortunately the check is pretty expensive, but this should
        // not be performance critical code after all...
        ::Bool_t auxFound = kFALSE;
        const std::string dynName = Utils::dynBranchPrefix(branchName);

        std::vector<TObjArray* > fullListOfBranches = {};
        // Add the list of branches of the main tree
        fullListOfBranches.push_back(m_inTree->GetListOfBranches());
        // If input tree has friend trees
        // add as well the list of friend tree branches
        if (m_inTree->GetListOfFriends()) {
          // Get the list of friends
          TList* fList = m_inTree->GetListOfFriends();
          // Loop over friend elements
          for (TObject* feObj : *fList) {
            if (feObj) {
              // Get corresponding friend tree
              auto* pElement = dynamic_cast<TFriendElement* >(feObj);
              if (not pElement)
                continue;
              TTree* friendTree = pElement->GetTree();
              // Add list of branches of the friend tree
              fullListOfBranches.push_back(friendTree->GetListOfBranches());
            }
          }
        }

        for (TObjArray* branches : fullListOfBranches) {
          for (Int_t i = 0; i < branches->GetEntriesFast(); ++i) {
            if (!branches->At(i))
              continue;

            const TString name(branches->At(i)->GetName());
            if (name.BeginsWith(branchName) || name.BeginsWith(dynName)) {
              auxFound = kTRUE;
              break;
            }
          }
        }
        if (auxFound) {
          ATH_MSG_WARNING("Couldn't find interface object/container \""
                          << intName << "\" belonging to branch \""
                          << branchName << "\"");
        }
        continue;
      }

      // Get the type of the interface:
      const EventFormatElement* el = m_inputEventFormat.get(intName);
      ::TClass* cl = ::TClass::GetClass(el->className().c_str());
      if ((!cl) || (!cl->IsLoaded())) {
        ATH_MSG_WARNING("Couldn't find dictionary for type \""
                        << el->className() << "\"");
        continue;
      }

      // Get the dictionary for the DataVector base class:
      static const std::type_info &baseTi = typeid(SG::AuxVectorBase);
      static const std::string baseName = SG::normalizedTypeinfoName(baseTi);
      static ::TClass* const baseCl = ::TClass::GetClass(baseName.c_str());
      if (!baseCl) {
        ATH_MSG_ERROR("Couldn't get dictionary for type \"" << baseName
                                                            << "\"");
        return StatusCode::FAILURE;
      }

      // The type of the auxiliary store is finally deduced from the
      // inheritance of the interface container.
      const TAuxStore::EStructMode mode =
          (cl->InheritsFrom(baseCl) ? TAuxStore::EStructMode::kContainerStore
                                    : TAuxStore::EStructMode::kObjectStore);

      // Scan the branches using a temporary TAuxStore instance:
      static constexpr bool TOP_STORE = true;
      TAuxStore temp(branchName, TOP_STORE, mode);
      static constexpr bool PRINT_WARNINGS = false;
      ATH_CHECK(temp.readFrom(*m_inTree, PRINT_WARNINGS));

      // Conveninence variable:
      ReadStats &stats = IOStats::instance().stats();

      // Teach the cache about all the branches:
      for (SG::auxid_t id : temp.getAuxIDs()) {
        stats.branch(branchName, id);
      }

      // Increment the number of known branches:
      stats.setBranchNum(stats.branchNum() + temp.getAuxIDs().size());
    }
    // If it's an interface container:
    else {
      // Try to access the branch:
      const ::TBranch* container = m_inTree->GetBranch(branchName.c_str());
      // If it exists, let's remember it:
      if (container) {
        IOStats::instance().stats().container(branchName);
      }
    }
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function is used internally when copying an object with its
/// auxiliary store from the input file, and branch access mode is
/// activated for the event object.
///
/// The assumption is that the store object already knows what prefix
/// it should be using. The key parameter only specifies under what
/// ID the object should be handled in the output object list.
///
/// @param store The store object to connect to the output
/// @param key The "key" with which to record the object
/// @returns The usual @c StatusCode tyoes
///
StatusCode TEvent::record(std::unique_ptr<TAuxStore> store,
                          const std::string &key) {

  // Check if we have an output tree:
  if (!m_outTree) {
    ATH_MSG_ERROR(
        "No output tree defined. Did you forget to call writeTo(...)?");
    return StatusCode::FAILURE;
  }

  // Check if we have a filtering rule for this key:
  const std::set<std::string>* filter = 0;
  auto filter_itr = m_auxItemList.find(key);
  if (filter_itr != m_auxItemList.end()) {
    filter = &(filter_itr->second);
  }

  // Check if we need to add it to the event record:
  Object_t::iterator vitr = m_outputObjects.find(key);
  if (vitr == m_outputObjects.end()) {

    // Configure the object for variable filtering:
    if (filter) {
      store->selectAux(*filter);
    }
    // Tell the object where to write its contents:
    ATH_CHECK(store->writeTo(*m_outTree));
    // Record it to the output list:
    static constexpr bool OWNS_STORE = true;
    m_outputObjects[key] =
        std::make_unique<TAuxManager>(store.release(), OWNS_STORE);

    // We're done:
    return StatusCode::SUCCESS;
  }

  // Check if the output has the right store:
  if (vitr->second->object() == store.get()) {
    // We're done already:
    return StatusCode::SUCCESS;
  }

  // If not, update the output manager. This can happen when we copy
  // objects from the input to the output files, and we process
  // multiple input files.

  // Check if the output manager is of the right type:
  TAuxManager* mgr = dynamic_cast<TAuxManager* >(vitr->second.get());
  if (mgr == nullptr) {
    ATH_MSG_ERROR("Output object with key \""
                  << key << "\" already exists, and is not of type TAuxStore");
    return StatusCode::FAILURE;
  }

  // Configure the object for variable filtering:
  if (filter) {
    store->selectAux(*filter);
  }

  // Connect the auxiliary store to the output tree:
  ATH_CHECK(store->writeTo(*m_outTree));

  // Update the manager:
  mgr->setObject(store.release());

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function is used by connectBranch(...) and connectMetaBranch(...)
/// to set up auxiliary store type objects correctly for accessing dynamic
/// variables from the input file.
///
/// @param mgr  The object manager of the auxiliary store object
/// @param tree The tree to read dynamic variables from
/// @returns The usual @c StatusCode types
///
StatusCode TEvent::setUpDynamicStore(TObjectManager &mgr, ::TTree* tree) {

  // Check if we can call setName(...) on the object:
  ::TMethodCall setNameCall;
  // Don't use this code in Athena access mode. And just accept that access
  // monitoring is disabled in this case...
  if (m_auxMode != kAthenaAccess) {
    setNameCall.InitWithPrototype(mgr.holder()->getClass(), "setName",
                                  "const char*");
    if (setNameCall.IsValid()) {
      // Yes, there is such a function. Let's call it with the branch
      // name:
      const ::TString params =
          ::TString::Format("\"%s\"", mgr.branch()->GetName());
      const char* charParams = params.Data();
      setNameCall.Execute(mgr.holder()->get(), charParams);
    } else {
      // This is weird. What sort of auxiliary container is this? :-/
      ATH_MSG_WARNING("Couldn't find setName(...) function for container \""
                      << mgr.branch()->GetName() << "\"  (type: "
                      << mgr.holder()->getClass()->GetName() << ")");
    }
  }

  // Check if we can switch out the internal store of this object:
  static const TClass* const holderClass =
      TClass::GetClass(typeid(SG::IAuxStoreHolder));
  if (!mgr.holder()->getClass()->InheritsFrom(holderClass)) {
    // Nope... So let's just end the journey here.
    return StatusCode::SUCCESS;
  }

  // Try to get the object as an IAuxStoreHolder:
  SG::IAuxStoreHolder* storeHolder = reinterpret_cast<SG::IAuxStoreHolder* >(
      mgr.holder()->getAs(typeid(SG::IAuxStoreHolder)));
  if (!storeHolder) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // Create a TAuxStore instance that will read the dynamic variables
  // of this container. Notice that the TAuxManager doesn't own the
  // TAuxStore object. It will be owned by the SG::IAuxStoreHolder
  // object.
  static constexpr bool TOP_STORE = false;
  auto store = std::make_unique<TAuxStore>(
      mgr.branch()->GetName(), TOP_STORE,
      (storeHolder->getStoreType() == SG::IAuxStoreHolder::AST_ObjectStore
           ? TAuxStore::EStructMode::kObjectStore
           : TAuxStore::EStructMode::kContainerStore));
  // This object is used to read data from the input, it needs to be
  // locked:
  store->lock();

  // Set it up to read from the input RNTuple.
  ATH_CHECK(store->readFrom(*tree));

  // Set it up to read from the input TTree.
  ATH_CHECK(store->readFrom(*tree));
  // Tell the auxiliary store which entry to use. This is essential for
  // metadata objects, and non-important for event data objects, which will
  // get a possibly different entry loaded in setAuxStore(...).
  store->getEntry(0);

  // Set up a manager for it.
  static constexpr bool SHARED_OWNER = false;
  m_inputObjects[std::string(mgr.branch()->GetName()) + "Dynamic"] =
      std::make_unique<TAuxManager>(store.get(), SHARED_OWNER);

  // Give this object to the store holder:
  storeHolder->setStore(store.release());

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function is used internally to set up the writing of the auxiliary
/// store variables that were dynamically created on an object. (And not
/// statically defined to be part of that object.)
///
/// @param outTree The TTree to put the auxiliary branches into
/// @param mgr The object manager of the output object
/// @param metadata Flag specifying whether the info written is metadata or
///                 not
/// @returns The usual @c StatusCode types
///
StatusCode TEvent::putAux(::TTree &outTree, TVirtualManager &vmgr,
                          bool metadata) {

  // A little sanity check:
  assert(m_outputEventFormat != 0);

  // Do the conversion:
  TObjectManager* mgr = dynamic_cast<TObjectManager* >(&vmgr);
  if (!mgr) {
    // It's not an error any more when we don't get a TObjectManager.
    return StatusCode::SUCCESS;
  }

  // Check if we need to do anything here:
  if (!mgr->holder()->getClass()->InheritsFrom("SG::IAuxStoreIO")) {
    return StatusCode::SUCCESS;
  }

  // Get a pointer to the auxiliary store I/O interface:
  SG::IAuxStoreIO* aux = reinterpret_cast<SG::IAuxStoreIO* >(
      mgr->holder()->getAs(typeid(SG::IAuxStoreIO)));
  if (!aux) {
    ATH_MSG_FATAL("There is a logic error in the code!");
    return StatusCode::FAILURE;
  }

  // Check if we have rules defined for which auxiliary properties
  // to write out:
  xAOD::AuxSelection sel;
  if (!metadata) {
    auto item_itr = m_auxItemList.find(mgr->branch()->GetName());
    if (item_itr != m_auxItemList.end()) {
      sel.selectAux(item_itr->second);
    }
  }

  // Get the dynamic auxiliary variables held by this object, which
  // were selected to be written:
  const SG::auxid_set_t auxids =
      sel.getSelectedAuxIDs(aux->getSelectedAuxIDs());

  // If there are no dynamic auxiliary variables in the object, return
  // right away:
  if (auxids.empty()) {
    return StatusCode::SUCCESS;
  }

  // Decide what should be the prefix of all the dynamic branches:
  const std::string dynNamePrefix =
      Utils::dynBranchPrefix(mgr->branch()->GetName());

  // Select which container to add the variables to:
  Object_t &objects = (metadata ? m_outputMetaObjects : m_outputObjects);

  // This iteration will determine the ordering of branches within
  // the tree, so sort auxids by name.
  const SG::AuxTypeRegistry &r = SG::AuxTypeRegistry::instance();
  typedef std::pair<std::string, SG::auxid_t> AuxVarSort_t;
  std::vector<AuxVarSort_t> varsort;
  varsort.reserve(auxids.size());
  for (SG::auxid_t id : auxids) {
    varsort.emplace_back(r.getName(id), id);
  }
  std::sort(varsort.begin(), varsort.end());

  // Extract all the dynamic variables from the object:
  for (const auto &p : varsort) {

    // The auxiliary ID:
    const SG::auxid_t id = p.second;

    // Construct a name for the branch that we will write:
    const std::string brName = dynNamePrefix + p.first;

    // Try to find the branch:
    Object_t::iterator bmgr = objects.find(brName);

    // Check if we already know about this variable:
    if (bmgr == objects.end()) {

      // Construct the full type name of the variable:
      const std::type_info* brType = aux->getIOType(id);
      if (!brType) {
        ATH_MSG_ERROR("No I/O type found for variable " << brName);
        return StatusCode::FAILURE;
      }
      const std::string brTypeName = Utils::getTypeName(*brType);
      std::string brProperTypeName = "<unknown>";

      // The branch that will hopefully be created:
      ::TBranch* br = 0;

      // Check if it's a primitive type or not:
      if (strlen(brType->name()) == 1) {

        // Making the "proper" type name is simple in this case:
        brProperTypeName = brTypeName;

        // Get the character describing this type for ROOT:
        const char rootType = Utils::rootType(brType->name()[0]);
        if (rootType == '\0') {
          ATH_MSG_ERROR("Type not known for variable \""
                        << brName << "\" of type \"" << brTypeName << "\"");
          return StatusCode::FAILURE;
        }

        // Create the full description of the variable for ROOT:
        std::ostringstream leaflist;
        leaflist << brName << "/" << rootType;

        // Let's create a holder for this property:
        static constexpr bool IS_OWNER = false;
        auto auxmgr = std::make_unique<TPrimitiveAuxBranchManager>(
            id, nullptr, new THolder(aux->getIOData(id), nullptr, IS_OWNER));

        // ... and let's add it to the output TTree:
        static constexpr Int_t BASKET_SIZE = 32000;
        *(auxmgr->branchPtr()) =
            outTree.Branch(brName.c_str(), auxmgr->holder()->get(),
                           leaflist.str().c_str(), BASKET_SIZE);
        if (!auxmgr->branch()) {
          ATH_MSG_ERROR("Failed to create branch \""
                        << brName << "\" out of type \"" << brProperTypeName
                        << "\"");
          // Clean up:
          *(auxmgr->holder()->getPtr()) = 0;
          return StatusCode::FAILURE;
        }
        br = auxmgr->branch();

        // Store it in the output list.
        objects[brName] = std::move(auxmgr);

      } else {

        // Check if we have a dictionary for this type:
        static constexpr Bool_t LOAD_IF_NOT_FOUND = kTRUE;
        static constexpr Bool_t SILENT = kTRUE;
        TClass* cl = TClass::GetClass(*brType, LOAD_IF_NOT_FOUND, SILENT);
        if (cl == nullptr) {
          // The dictionary needs to be loaded now. This could be an
          // issue. But let's hope for the best...
          cl = TClass::GetClass(brTypeName.c_str());
          // If still not found...
          if (cl == nullptr) {
            ATH_MSG_ERROR("Dictionary not available for variable \""
                          << brName << "\" of type \"" << brTypeName << "\"");
            return StatusCode::FAILURE;
          }
        }

        // The proper type name comes from the dictionary in this case:
        brProperTypeName = cl->GetName();

        // Let's create a holder for this property:
        static constexpr bool IS_OWNER = false;
        auto auxmgr = std::make_unique<TAuxBranchManager>(
            id, nullptr, new THolder(aux->getIOData(id), cl, IS_OWNER));

        // ... and let's add it to the output TTree.
        static constexpr Int_t BASKET_SIZE = 32000;
        static constexpr Int_t SPLIT_LEVEL = 0;
        *(auxmgr->branchPtr()) = outTree.Branch(brName.c_str(), cl->GetName(),
                                                auxmgr->holder()->getPtr(),
                                                BASKET_SIZE, SPLIT_LEVEL);
        if (!auxmgr->branch()) {
          ATH_MSG_ERROR("Failed to create branch \""
                        << brName << "\" out of type \"" << brProperTypeName
                        << "\"");
          // Clean up:
          *(auxmgr->holder()->getPtr()) = 0;
          return StatusCode::FAILURE;
        }
        br = auxmgr->branch();

        // Store it in the output list.
        objects[brName] = std::move(auxmgr);
      }

      // If this is not the first event, fill up the already filled
      // events with (empty) content:
      if (outTree.GetEntries()) {
        void* ptr = br->GetAddress();
        br->SetAddress(0);
        for (::Long64_t i = 0; i < outTree.GetEntries(); ++i) {
          br->Fill();
        }
        br->SetAddress(ptr);
      }

      // If all went fine, let's add this branch to the event format
      // metadata:
      if (!m_outputEventFormat->exists(brName)) {
        m_outputEventFormat->add(EventFormatElement(brName, brProperTypeName,
                                                    mgr->branch()->GetName(),
                                                    getHash(brName)));
      }

      // We don't need to do the rest:
      continue;
    }

    // Access the object manager:
    bmgr = objects.find(brName);
    if (bmgr == objects.end()) {
      ATH_MSG_FATAL("There is an internal logic error in the code...");
      return StatusCode::FAILURE;
    }

    // Replace the managed object:
    void* nc_data ATLAS_THREAD_SAFE = // we hold non-const pointers but check
                                      // on retrieve
        const_cast<void* >(static_cast<const void* >(aux->getIOData(id)));
    bmgr->second->setObject(nc_data);
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode TEvent::recordAux(TAuxStore* store, const std::string &key) {

  // Check if we have an output tree:
  if (hasOutput() == false) {
    ATH_MSG_ERROR("No output tree set up.");
    return StatusCode::FAILURE;
  }

  // Check if we have a filtering rule for this key:
  const std::set<std::string>* filter = 0;
  auto filter_itr = m_auxItemList.find(key);
  if (filter_itr != m_auxItemList.end()) {
    filter = &(filter_itr->second);
  }

  // Check if we need to add it to the event record:
  Object_t::iterator vitr = m_outputObjects.find(key);
  if (vitr == m_outputObjects.end()) {

    // Configure the object for variable filtering:
    if (filter) {
      store->selectAux(*filter);
    }
    // Tell the object where to write its contents:
    ATH_CHECK(store->writeTo(*m_outTree));
    // Record it to the output list.
    static constexpr bool OWNS_STORE = false;
    m_outputObjects[key] = std::make_unique<TAuxManager>(store, OWNS_STORE);

    // We're done:
    return StatusCode::SUCCESS;
  }

  // Check if the output has the right store:
  if (vitr->second->object() == store) {
    // We're done already:
    return StatusCode::SUCCESS;
  }

  // If not, update the output manager. This can happen when we copy
  // objects from the input to the output files, and we process
  // multiple input files.

  // Check if the output manager is of the right type:
  TAuxManager* mgr = dynamic_cast<TAuxManager* >(vitr->second.get());
  if (mgr == nullptr) {
    ATH_MSG_ERROR("Output object with key \""
                  << key << "\" already exists, and is not of type TAuxStore");
    return StatusCode::FAILURE;
  }

  // Configure the object for variable filtering:
  if (filter) {
    store->selectAux(*filter);
  }

  // Connect the auxiliary store to the output tree:
  ATH_CHECK(store->writeTo(*m_outTree));

  // Update the manager:
  mgr->setObject(store);

  // Return gracefully:
  return StatusCode::SUCCESS;
}

} // namespace xAOD