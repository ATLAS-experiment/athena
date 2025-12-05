// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/REvent.h"

#include "IOUtils.h"
#include "xAODRootAccess/RAuxStore.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TVirtualIncidentListener.h"
#include "xAODRootAccess/tools/RAuxManager.h"
#include "xAODRootAccess/tools/RObjectManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/TIncident.h"
#include "xAODRootAccess/tools/Utils.h"

// Framework include(s).
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/AuxInfoBase.h"
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

// ROOT include(s).
#include <TFile.h>
#include <TKey.h>
#include <TMethodCall.h>

#include <ROOT/RNTuple.hxx>

// System include(s).
#include <regex>
#include <stdexcept>
#include <typeinfo>
#include <utility>

namespace {

/// This function is used to search for a field in an RNTuple that
/// contains a given substring. It returns the name of the first field
/// that contains the search term. If no such field is found, the function
/// returns the search term itself.
///
/// @param reader The RNTupleReader to search in
/// @param pre The search term
/// @returns The name of the first field that contains the search term
///
std::string getFirstFieldMatch(ROOT::RNTupleReader& reader,
                               const std::string& pre) {

  const std::regex pattern(".*" + pre + ".*");
  for (const auto& field : reader.GetDescriptor().GetTopLevelFields()) {

    if (std::regex_match(field.GetFieldName(), pattern)) {
      return field.GetFieldName();
    }
  }

  return pre;
}

}  // namespace

namespace xAOD::Experimental {

// Initialise some static data.
static const char* const EVENT_NTUPLE_NAME = "EventData";
static const char* const METADATA_NTUPLE_NAME = "MetaData";

REvent::REvent() : Event("xAOD::Experimental::REvent") {}

REvent::~REvent() {

  // Clear the input and output objects before the input/output files would be
  // closed. Otherwise we can be left with RNTuple view objects pointing
  // nowhere.
  m_inputObjects.clear();
  m_outputObjects.clear();
}

/// This function takes care of connecting the event object to a new input
/// file. It reads in the metadata of the input file needed for reading
/// the file.
///
/// @param fileName name of file needed for metadata access
///
StatusCode REvent::readFrom(std::string_view fileName) {

  ATH_MSG_INFO("REvent::readFrom:  fileName " << fileName);


  // Clear the cached input objects.
  m_inputObjects.clear();
  m_inputMissingObjects.clear();
  m_inputMetaObjects.clear();
  {
    upgrading_lock_t lock(m_branchesMutex);
    lock.upgrade();
    m_branches.clear();
  }
  m_eventReader.reset();

  // Reset the internal flags.
  m_entry = -1;
  m_inputNTupleIsMissing = false;

  // Clear out the current object.
  m_inputEventFormat = {};


  ATH_MSG_DEBUG("Create RNTupleReader for \"" << METADATA_NTUPLE_NAME
                                               << "\" in file: " << fileName);


  // Set up a reader for the metadata ntuple.
  // Since some types are non-xAOD types and so not 'visible' when running in AnalysisBase
  //   we need to protect for unknown types with SetEmulateUnknownTypes(true)
  ROOT::RNTupleDescriptor::RCreateModelOptions opts;
  opts.SetEmulateUnknownTypes(true);
  m_metaReader = ROOT::RNTupleReader::Open(opts, METADATA_NTUPLE_NAME, fileName);
  if (!m_metaReader) {
    ATH_MSG_ERROR("Couldn't find \"" << METADATA_NTUPLE_NAME
                                     << "\" tree in input file: " << fileName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created RNTupleReader for \"" << METADATA_NTUPLE_NAME
                                               << "\" in file: " << fileName);

  // Make sure that the xAOD::EventFormat dictonary is loaded.
  // This may not be the case if streamer information reading is turned
  // off.
  static const std::string eventFormatTypeName =
      Utils::getTypeName(typeid(EventFormat));
  if (::TClass::GetClass(eventFormatTypeName.c_str()) == nullptr) {
    ATH_MSG_WARNING("Couldn't load the EventFormat dictionary");
  }
  ATH_MSG_VERBOSE("Loaded the xAOD::EventFormat dictionary");

  // Helper lambda for collecting the event format metadata from an RNTuple
  // with a given name.
  auto readEventFormatMetadata = [&](std::string_view tupleName) -> StatusCode {
    // Set up a reader. This may technically be the same as m_metaReader, but
    // it's easier to write the code this way.
    auto metaReader = ROOT::RNTupleReader::Open(tupleName, fileName);
    if (!metaReader) {
      ATH_MSG_ERROR("Couldn't find \""
                    << tupleName << "\" ntuple in input file: " << fileName);
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Created temporary RNTupleReader for \""
                    << tupleName << "\" in file: " << fileName);

    // Try to read in the event format metadata.
    const std::string eventFormatFieldName =
        getFirstFieldMatch(*m_metaReader, "EventFormat");
    try {
      // Add its elements to m_inputEventFormat.
      auto ef = m_metaReader->GetView<xAOD::EventFormat>(eventFormatFieldName);
      for (const auto& [key, element] : ef(0u)) {
        m_inputEventFormat.add(element);
      }
    } catch (const ROOT::RException&) {
      ATH_MSG_WARNING("Input file provides no event or metadata");
      return StatusCode::RECOVERABLE;
    }
    ATH_MSG_VERBOSE("Merged event format metadata from \"" << tupleName
                                                           << "\" ntuple");

    // Return gracefully.
    return StatusCode::SUCCESS;
  };

  // Read in the metadata from the "main" metadata ntuple.
  const StatusCode sc = readEventFormatMetadata(METADATA_NTUPLE_NAME);
  if (sc.isRecoverable()) {
    m_inputNTupleIsMissing = true;
    return StatusCode::SUCCESS;
  }
  ATH_CHECK(sc);

  // List all the other Metadata RNTuples in the input file.
  // Having several metadata tuples can happen for augmented files
  // as one metadata tree per stream is produced.
  std::set<std::string> lOtherMetaTupleNames;
  {
    std::unique_ptr<TFile> inFile(TFile::Open(fileName.data(), "READ"));
    TList* lKeys = inFile->GetListOfKeys();
    if (lKeys == nullptr) {
      ATH_MSG_ERROR("Could not get list of keys for input file: " << fileName);
      return StatusCode::FAILURE;
    }
    for (const TObject* obj : *lKeys) {
      const std::string keyName = obj->GetName();
      // Make sure the key corresponds to a metadata ntuple but
      // do not add the current metadata tree in the list of other trees
      // and do not add the metadata tree handlers to the list
      if ((keyName != METADATA_NTUPLE_NAME) &&
          (keyName.find("MetaData") != std::string::npos) &&
          (keyName.find("MetaDataHdr") == std::string::npos)) {
        // Make sure key corresponds to an RNTuple
        const TKey* key = dynamic_cast<const TKey*>(obj);
        if (key == nullptr) {
          ATH_MSG_ERROR("Object describing \"" << keyName << "\" in file \""
                                               << fileName
                                               << "\" is not a TKey?");
          return StatusCode::FAILURE;
        }
        const char* className = key->GetClassName();
        static constexpr bool LOAD = kFALSE;
        static constexpr bool SILENT = kTRUE;
        ::TClass* cl = ::TClass::GetClass(className, LOAD, SILENT);
        if ((cl != nullptr) && cl->InheritsFrom(ROOT::RNTuple::Class())) {
          lOtherMetaTupleNames.insert(std::move(keyName));
        }
      }
    }
  }

  // Read in the metadata from any additonal NTuples found (if any).
  for (const std::string& tupleName : lOtherMetaTupleNames) {
    ATH_CHECK(readEventFormatMetadata(tupleName));
  }

  // Set up the main ntuple reader.
  m_eventReader = ROOT::RNTupleReader::Open(EVENT_NTUPLE_NAME, fileName);
  if (!m_eventReader) {
    ATH_MSG_ERROR("Couldn't access RNTuple \"" << EVENT_NTUPLE_NAME
                                               << "\" in file: " << fileName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created RNTupleReader for \"" << EVENT_NTUPLE_NAME
                                               << "\" in file: " << fileName);

  // Init the statistics collection.
  ATH_CHECK(initStats());
  // Update the event counter in the statistics object:
  xAOD::ReadStats& stats = IOStats::instance().stats();
  stats.setNEvents(stats.nEvents() + m_eventReader->GetNEntries());

  // Notify the listeners that a new file was opened:
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

  // The initialisation was successful:
  return StatusCode::SUCCESS;
}

/// @returns The number of events in the input file(s)
///
::Long64_t REvent::getEntries() const {

  if (m_eventReader) {
    return m_eventReader->GetNEntries();
  } else if (m_inputNTupleIsMissing) {
    return 0u;
  } else {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return 0u;
  }
}

/// This function is used to move to looking at a new entry from the input
/// ntuple. It doesn't do any I/O operation at this point, it just remembers
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
::Int_t REvent::getEntry(::Long64_t entry, ::Int_t getall) {

  // A little sanity check:
  if (!m_eventReader) {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return -1;
  }

  // Check if anything needs to be done.
  if (entry == m_entry) {
    ATH_MSG_DEBUG("Entry " << entry << " is already the active one");
    return 0;
  }

  // Set entry value
  m_entry = entry;

  // Stats counter needs to know it's the next event.
  IOStats::instance().stats().nextEvent();

  // Loop over all input object managers, and force them to load their
  // content. But only if getall was used.
  ::Int_t result = 0;
  if (getall) {
    for (auto& [key, inObj] : m_inputObjects) {
      result += inObj->getEntry(getall);
    }
  }

  // Notify the listeners that a new event was loaded.
  const TIncident incident(IncidentType::BeginEvent);
  for (TVirtualIncidentListener* listener : m_listeners) {
    listener->handle(incident);
  }

  // Return the number of bytes read.
  return result;
}

bool REvent::hasInput() const {

  return (m_eventReader || m_inputNTupleIsMissing);
}

bool REvent::hasOutput() const {

  return false;
}

StatusCode REvent::getNames(const std::string& /*targetClassName*/,
                            std::vector<std::string>& /*vkeys*/,
                            bool /*metadata*/) const {

  ATH_MSG_ERROR("xAOD::REvent::getNames not yet implemented");
  return StatusCode::FAILURE;
}

/// This is one of the more important functions of the class. It connects the
/// event object to a given field of the input RNTuple.
///
/// The type that is read in to memory is not actually determined by the type
/// written to the ntuple itself, but from the xAOD::EventFormat object. Which
/// is there to make it possible to possibly use ROOT read rules to read a
/// different type than what's on disk.
///
/// Everything else is fairly basic ROOT stuff, the code just has to make
/// sure that the memory management is set up correctly for the created
/// object.
///
/// @param key The name of the branch to connect to
/// @param silent Set to @c true to make the code fail silently
///               in case the field can't be connected to
/// @return The usual @c StatusCode values
///
StatusCode REvent::connectObject(const std::string& key, ::Bool_t silent) {

  // A little sanity check.
  if (!m_eventReader) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Increment the access counter on this container.
  IOStats::instance().stats().readContainer(key);

  // Check if the branch is already connected.
  if (m_inputObjects.contains(key)) {
    return StatusCode::SUCCESS;
  }
  // Check if it was already found to be missing.
  if (m_inputMissingObjects.contains(key)) {
    if (!silent) {
      ATH_MSG_WARNING("Field \"" << key << "\" not available on input");
    }
    return StatusCode::RECOVERABLE;
  }

  // Tell the user what's happening.
  ATH_MSG_DEBUG("Connecting to branch \"" << key << "\"");

  // Check if we have metadata about this branch.
  const xAOD::EventFormatElement* ef = nullptr;

  // RNTuples store fields with an "Aux:" postfix instead of "Aux.".
  std::string key_to_read = key;
  if (key.ends_with("Aux.")) {
    key_to_read.replace(key_to_read.size() - 1, 1, ":");
  }

  if (m_inputEventFormat.exists(key) == false) {
    if (!silent) {
      ATH_MSG_WARNING("No metadata available for object: " << key);
    }
  } else {
    ef = m_inputEventFormat.get(key);
  }

  // Check if the field exists in our input RNTuple.
  if (m_eventReader->GetDescriptor().FindFieldId(key_to_read.c_str()) ==
      ROOT::kInvalidDescriptorId) {
    // Field doesn't exist
    if (!silent) {
      ATH_MSG_WARNING("Field \"" << key_to_read << "\" not available on input");
    }
    m_inputMissingObjects.insert(key);
    return StatusCode::RECOVERABLE;
  }

  // RDS: may need some logic here to get type from inputEventFormat rather than
  // the view
  //      to read in with automatic schema evolution

  // Get class name from the field
  ROOT::RNTupleView<void> view =
      m_eventReader->GetView<void>(key_to_read.c_str(), nullptr);
  std::string className = view.GetField().GetTypeName();
  if (className == "") {
    if (ef) {
      // This is a fairly weird situation, but let's fall back to taking
      // the class name from the metadata object in this case.
      className = ef->className();
    } else {
      ATH_MSG_ERROR(
          "Couldn't find an appropriate type with a dictionary for field \""
          << key_to_read << "\"");
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
        "Couldn't find an appropriate type with a dictionary for field \""
        << key_to_read << "\"");
    return StatusCode::FAILURE;
  }

  // Make sure that the current object is the "active event":
  setActive();

  // Check if the output already has this object. If it does, let's
  // assume that we have been copying the object to the output. Which
  // means that we need to resume filling the same memory address that
  // the output holder points to.
  void* ptr = nullptr;

  // Handle the case where an output object with this key already exists.

  // If there is no output object, then let's create one ourselves.
  // This is the only way in which we can have the memory management of
  // THolder do the right thing with this object.
  if (ptr == nullptr) {
    ptr = realClass->New();
  }

  // Create the new manager object that will hold this EDM object.
  auto mgr = std::make_unique<RObjectManager>(
      m_eventReader->GetView(key_to_read, ptr, className), m_entry,
      std::make_unique<THolder>(ptr, realClass));
  RObjectManager* mgrPtr = mgr.get();
  m_inputObjects[key] = std::move(mgr);

  // If it's an auxiliary store object, set it up correctly.
  if (Details::isAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(setUpDynamicStore(*mgrPtr, *m_eventReader));
  }

  // If it (probably) has an associated auxiliary store, set it up as well.
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
/// @returns The usual StatusCode types
///
StatusCode REvent::connectMetaObject(const std::string& key, bool silent) {

  // A little sanity check.
  if (!m_metaReader) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Check if the branch is already connected.
  if (m_inputMetaObjects.contains(key)) {
    return StatusCode::SUCCESS;
  }

  // RNTuples store fields with an "Aux:" postfix instead of "Aux.".
  std::string key_to_read = key;
  if (key.ends_with("Aux.")) {
    key_to_read.replace(key_to_read.size() - 1, 1, ":");
  }

  // Check if the field exists in our input RNTuple.
  if (m_metaReader->GetDescriptor().FindFieldId(key_to_read.c_str()) ==
      ROOT::kInvalidDescriptorId) {
    // Field doesn't exist
    if (!silent) {
      ATH_MSG_WARNING("Field \"" << key_to_read << "\" not available on input");
    }
    return StatusCode::RECOVERABLE;
  }

  // RDS: may need some logic here to get type from inputEventFormat rather than
  // the view
  //      to read in with automatic schema evolution

  // Get class name from the field
  ROOT::RNTupleView<void> view =
      m_metaReader->GetView<void>(key_to_read.c_str(), nullptr);
  std::string className = view.GetField().GetTypeName();
  if (className == "") {
    ATH_MSG_ERROR(
        "Couldn't find an appropriate type with a dictionary for field \""
        << key_to_read << "\"");
    return StatusCode::FAILURE;
  }
  ::TClass* realClass = ::TClass::GetClass(className.c_str());
  if ((!realClass) || (!realClass->IsLoaded())) {
    // Now we're in trouble...
    ATH_MSG_ERROR(
        "Couldn't find an appropriate type with a dictionary for field \""
        << key_to_read << "\"");
    return StatusCode::FAILURE;
  }

  // Create the object and the manager(s) around it.
  void* ptr = realClass->New();
  static const ::Long64_t FIRST_ENTRY = 0;
  auto mgr = std::make_unique<RObjectManager>(
      m_metaReader->GetView(key_to_read, ptr, className), FIRST_ENTRY,
      std::make_unique<THolder>(ptr, realClass));
  // For metadata, we must read in the first entry - entry number already set by FIRST_ENTRY in constructor
  mgr->getEntry();
  RObjectManager* mgrPtr = mgr.get();
  m_inputMetaObjects[key] = std::move(mgr);

  // If it's an auxiliary store object, set it up correctly.
  if (Details::isAuxStore(*(mgrPtr->holder()->getClass()))) {
    // For reading in of the dynamic variables for metadata
    ATH_CHECK(setUpDynamicStore(*mgrPtr, *m_metaReader));
  }

  // If it (probably) has an associated auxiliary store, set it up as well.
  if (Details::hasAuxStore(*(mgrPtr->holder()->getClass()))) {
    ATH_CHECK(connectMetaAux(
        key + "Aux.", Details::isStandalone(*(mgrPtr->holder()->getClass()))));
    static constexpr bool METADATA = true;
    ATH_CHECK(setAuxStore(key, *mgrPtr, METADATA));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
} // connectMetaObject

/// This function is used internally to connect an auxiliary object to
/// the input. Based on the configuration of the object it will either
/// use RAuxStore, or the EDM object that was used to write the auxiliary
/// information in Athena.
///
/// @param prefix The prefix (main branch name) of the auxiliary data
/// @param standalone Type of the auxiliary store that should be created
/// @return <code>kTRUE</code> if the connection was successful, or
///         <code>kFALSE</code> if it was not
///
StatusCode REvent::connectAux(const std::string& prefix, ::Bool_t standalone) {

  // A little sanity check.
  if (!m_eventReader) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Check if the field is already connected.
  if (m_inputObjects.contains(prefix)) {
    return StatusCode::SUCCESS;
  }

  // Connect to the field as we would for any other.
  static constexpr bool SILENT = false;
  ATH_CHECK(connectObject(prefix, SILENT));

  // Access the object's manager.
  Object_t::const_iterator mgr_itr = m_inputObjects.find(prefix);
  if (mgr_itr == m_inputObjects.end()) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }
  const Details::IObjectManager* omgr =
      dynamic_cast<const RObjectManager*>(mgr_itr->second.get());
  if (omgr == nullptr) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // Check if we can switch out the internal store of this object.
  static const TClass* const holderClass =
      TClass::GetClass(typeid(SG::IAuxStoreHolder));
  if (omgr->holder()->getClass()->InheritsFrom(holderClass) == false) {
    // Nope... So let's just end the journey here.
    return StatusCode::SUCCESS;
  }

  // Try to get the object as an IAuxStoreHolder.
  SG::IAuxStoreHolder* storeHolder = reinterpret_cast<SG::IAuxStoreHolder*>(
      omgr->holder()->getAs(typeid(SG::IAuxStoreHolder)));
  if (!storeHolder) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // A sanity check to see whether the store's type is in sync with the
  // object's type that it will be connected to:
  if ((standalone &&
       (storeHolder->getStoreType() != SG::IAuxStoreHolder::AST_ObjectStore)) ||
      ((!standalone) && (storeHolder->getStoreType() !=
                         SG::IAuxStoreHolder::AST_ContainerStore))) {
    ATH_MSG_ERROR("Requested store types inconsistent for: " << prefix);
    ATH_MSG_ERROR("standalone = "
                  << standalone << ", getStoreType() = "
                  << static_cast<int>(storeHolder->getStoreType()));
    return StatusCode::FAILURE;
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// This function is used internally to connect an auxiliary metadata object
/// to the input. Based on the configuration of the object it will either
/// use TAuxStore, or the EDM object that was used to write the auxiliary
/// information in Athena.
///
/// @param prefix The prefix (main branch name) of the auxiliary data
/// @param standalone Type of the auxiliary store that should be created
/// @return The usual StatusCode types
///
StatusCode REvent::connectMetaAux(const std::string& prefix,
                                  ::Bool_t standalone) {

  // A little sanity check.
  if (!m_metaReader) {
    ATH_MSG_ERROR("Function called on un-initialised object");
    return StatusCode::FAILURE;
  }

  // Check if the field is already connected.
  if (m_inputMetaObjects.contains(prefix)) {
    return StatusCode::SUCCESS;
  }

  // Connect to the field as we would for any other.
  static constexpr bool SILENT = false;
  ATH_CHECK(connectMetaObject(prefix, SILENT));

  // Access the object's manager.
  Object_t::const_iterator mgr_itr = m_inputMetaObjects.find(prefix);
  if (mgr_itr == m_inputMetaObjects.end()) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }
  const Details::IObjectManager* omgr =
      dynamic_cast<const RObjectManager*>(mgr_itr->second.get());
  if (omgr == nullptr) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // Check if we can switch out the internal store of this object.
  static const TClass* const holderClass =
      TClass::GetClass(typeid(SG::IAuxStoreHolder));
  if (omgr->holder()->getClass()->InheritsFrom(holderClass) == false) {
    // Nope... So let's just end the journey here.
    return StatusCode::SUCCESS;
  }

  // Try to get the object as an IAuxStoreHolder.
  SG::IAuxStoreHolder* storeHolder = reinterpret_cast<SG::IAuxStoreHolder*>(
      omgr->holder()->getAs(typeid(SG::IAuxStoreHolder)));
  if (!storeHolder) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // A sanity check to see whether the store's type is in sync with the
  // object's type that it will be connected to:
  if ((standalone &&
       (storeHolder->getStoreType() != SG::IAuxStoreHolder::AST_ObjectStore)) ||
      ((!standalone) && (storeHolder->getStoreType() !=
                         SG::IAuxStoreHolder::AST_ContainerStore))) {
    ATH_MSG_ERROR("Requested store types inconsistent for: " << prefix);
    ATH_MSG_ERROR("standalone = "
                  << standalone << ", getStoreType() = "
                  << static_cast<int>(storeHolder->getStoreType()));
    return StatusCode::FAILURE;
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// Every time a DataVector is read in from the input for a new RNTuple
/// entry, one needs to re-connect it with its auxiliary store. This
/// function takes care of this.
///
/// @param key The key (field name) of the object whose auxiliary store
///            should be set up
/// @param mgr The manager object of the DV container
/// @param metadata Flag specifying whether we're dealing with a metadata
///                 or event data object
/// @return The usual @c StatusCode types
///
StatusCode REvent::setAuxStore(const std::string& key,
                               Details::IObjectManager& mgr, bool metadata) {

  // Check if we need to do anything:
  if ((Details::hasAuxStore(*(mgr.holder()->getClass())) == false) &&
      (Details::isAuxStore(*(mgr.holder()->getClass())) == false)) {
    return StatusCode::SUCCESS;
  }

  // Select which object container to use:
  Object_t& objects = (metadata ? m_inputMetaObjects : m_inputObjects);

  // Look up the auxiliary object's manager:
  TVirtualManager* auxMgr = nullptr;
  std::string auxKey;
  if (Details::isAuxStore(*(mgr.holder()->getClass()))) {
    auxMgr = &mgr;
    auxKey = key;
  } else {
    auto itr = objects.find(key + "Aux.");
    if (itr == objects.end()) {
      // Apparently there's no auxiliary object for this DV, so let's
      // give up.
      return StatusCode::SUCCESS;
    }
    auxMgr = itr->second.get();
    auxKey = key + "Aux:";
  }

  if (!metadata) {
    // Make sure the auxiliary object is up to date.
    ::Int_t readBytes = auxMgr->getEntry();

    // Check if there is a separate auxiliary object for the dynamic
    // variables, which would need to be updated.
    const std::string dynAuxKey = auxKey + "Dynamic";
    auto dynAuxMgr = objects.find(dynAuxKey);
    if ((dynAuxMgr != objects.end()) && (readBytes || (auxMgr == &mgr))) {

      // Tell the dynamic store object to switch to a new entry.
      dynAuxMgr->second->getEntry();
    }
  }

  // Stop here if we've set up an auxiliary store.
  if (auxMgr == &mgr) {
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

  // Get the concrete auxiliary manager:
  RObjectManager* omgr = dynamic_cast<RObjectManager*>(auxMgr);
  if (!omgr) {
    ATH_MSG_FATAL("Auxiliary manager for \"" << auxKey
                                             << "\" is not of the right type");
    return StatusCode::FAILURE;
  }
  void* p = omgr->holder()->getAs(typeid(SG::IConstAuxStore));
  const SG::IConstAuxStore* store =
      reinterpret_cast<const SG::IConstAuxStore*>(p);
  if (store == nullptr) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // Connect the two:
  if (vec) {
    vec->setStore(store);
  } else if (aux) {
    aux->setStore(store);
  } else {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // We succeeded:
  return StatusCode::SUCCESS;
}

StatusCode REvent::record(void*, const std::string&, const std::string&, bool,
                          bool, bool) {

  ATH_MSG_ERROR("xAOD::REvent::record not yet implemented");
  return StatusCode::FAILURE;
}

StatusCode REvent::recordAux(TVirtualManager&, const std::string&, bool) {

  ATH_MSG_ERROR("xAOD::REvent::recordAux not yet implemented");
  return StatusCode::FAILURE;
}

/// This function is used internally to initialise the reading of an input
/// file. It prepares the "monitoring information" in memory that gets filled
/// while the code is running, with information about xAOD I/O.
///
/// @returns The usual @c StatusCode values
///
StatusCode REvent::initStats() {

  // A little sanity check.
  if (!m_eventReader) {
    ATH_MSG_ERROR("Function called on an uninitialised object");
    return StatusCode::FAILURE;
  }

  // Reset the number of input branches information.
  IOStats::instance().stats().setBranchNum(0);

  // Loop over the EventFormat information.
  for (const auto& [key, format] : m_inputEventFormat) {

    ATH_MSG_DEBUG("Investigating object \"" << key << "\" of class \""
                                            << format.className() << "\"");

    // If it's an auxiliary container, scan it using RAuxStore.
    if (key.ends_with("Aux.")) {

      // But first decide whether it describes a container, or just
      // a single object. For that, first get the class that it was
      // written with.
      TClass* cl = TClass::GetClass(format.className().c_str());
      if ((cl == nullptr) || (cl->IsLoaded() == false)) {
        ATH_MSG_WARNING(
            "Couldn't find the dictionary for type: " << format.className());
        continue;
      }

      // And then check if it inherits from xAOD::AuxContainerBase or from
      // xAOD::AuxInfoBase.
      static TClass* const auxContCl = TClass::GetClass(
          SG::normalizedTypeinfoName(typeid(xAOD::AuxContainerBase)).c_str());
      static TClass* const auxInfoCl = TClass::GetClass(
          SG::normalizedTypeinfoName(typeid(xAOD::AuxInfoBase)).c_str());
      if ((auxContCl == nullptr) || (auxInfoCl == nullptr)) {
        ATH_MSG_ERROR(
            "Couldn't get dictionary for xAOD::AuxContainerBase or "
            "xAOD::AuxInfoBase");
        return StatusCode::FAILURE;
      }
      const bool isContainer = cl->InheritsFrom(auxContCl);
      const bool isInfo = cl->InheritsFrom(auxInfoCl);
      if ((isContainer == false) && (isInfo == false)) {
        ATH_MSG_WARNING("Auxiliary store \""
                        << key
                        << "\" is of an unknown type: " << format.className());
        continue;
      }
      ATH_MSG_VERBOSE("isContainer = " << isContainer
                                       << ", isInfo = " << isInfo);

      // Scan the branches using a temporary RAuxStore instance.
      const std::string fieldName = key.substr(0, key.size() - 1) + ":";
      const RAuxStore::EStructMode mode =
          (isContainer ? RAuxStore::EStructMode::kContainerStore
                       : RAuxStore::EStructMode::kObjectStore);
      static constexpr bool TOP_STORE = true;
      RAuxStore temp(fieldName, TOP_STORE, mode);
      ATH_CHECK(temp.readFrom(*m_eventReader));

      // Add all the auxids to the statistics object:
      ReadStats& stats = IOStats::instance().stats();
      for (SG::auxid_t id : temp.getAuxIDs()) {
        stats.branch(fieldName, id);
      }

      // Increment the number of known branches.
      stats.setBranchNum(stats.branchNum() + temp.getAuxIDs().size());
    }
    // If it's an interface container.
    else {
      // Check if it's part of the input ntuple.
      if (m_eventReader->GetDescriptor().FindFieldId(key) !=
          ROOT::kInvalidDescriptorId) {
        IOStats::instance().stats().container(key);
      }
    }
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function is used by @c connectObject(...) and @c connectMetaObject(...)
/// to set up auxiliary store type objects correctly for accessing dynamic
/// variables from the input file.
///
/// @param mgr  The object manager of the auxiliary store object
/// @param reader The RNTuple reader to read dynamic variables from
/// @returns The usual @c StatusCode types
///
StatusCode REvent::setUpDynamicStore(RObjectManager& mgr,
                                     ROOT::RNTupleReader& reader) {

  // The name of the field that the object is connected to.
  const std::string fieldName = mgr.field().GetField().GetFieldName();

  // Check if we can call setName(...) on the object.
  ::TMethodCall setNameCall;
  setNameCall.InitWithPrototype(mgr.holder()->getClass(), "setName",
                                "const char*");
  if (setNameCall.IsValid()) {
    // Yes, there is such a function. Let's call it with the field
    // name.
    const ::TString params = ::TString::Format("\"%s\"", fieldName.c_str());
    const char* charParams = params.Data();
    setNameCall.Execute(mgr.holder()->get(), charParams);
  } else {
    // This is weird. What sort of auxiliary container is this? :-/
    ATH_MSG_WARNING("Couldn't find setName(...) function for container \""
                    << fieldName << "\"  (type: "
                    << mgr.holder()->getClass()->GetName() << ")");
  }

  // Check if we can switch out the internal store of this object:
  static const TClass* const holderClass =
      TClass::GetClass(typeid(SG::IAuxStoreHolder));
  if (!mgr.holder()->getClass()->InheritsFrom(holderClass)) {
    // Nope... So let's just end the journey here.
    return StatusCode::SUCCESS;
  }

  // Try to get the object as an IAuxStoreHolder:
  SG::IAuxStoreHolder* storeHolder = reinterpret_cast<SG::IAuxStoreHolder*>(
      mgr.holder()->getAs(typeid(SG::IAuxStoreHolder)));
  if (!storeHolder) {
    ATH_MSG_FATAL("There's a logic error in the code");
    return StatusCode::FAILURE;
  }

  // Create an RAuxStore instance that will read the dynamic variables
  // of this container. Notice that the RAuxManager doesn't own the
  // RAuxStore object. It will be owned by the SG::IAuxStoreHolder
  // object.
  static constexpr bool TOP_STORE = false;
  auto store = std::make_unique<RAuxStore>(
      fieldName, TOP_STORE,
      (storeHolder->getStoreType() == SG::IAuxStoreHolder::AST_ObjectStore
           ? RAuxStore::EStructMode::kObjectStore
           : RAuxStore::EStructMode::kContainerStore));
  // This object is used to read data from the input, it needs to be
  // locked:
  store->lock();

  // Set it up to read from the input RNTuple.
  ATH_CHECK(store->readFrom(reader));
  // Tell the auxiliary store which entry to use. This is essential for
  // metadata objects, and non-important for event data objects, which will
  // get a possibly different entry loaded in setAuxStore(...).
  ATH_CHECK(store->getEntry(0));

  // Set up a manager for it.
  static constexpr bool SHARED_OWNER = false;
  m_inputObjects[fieldName + "Dynamic"] =
      std::make_unique<RAuxManager>(store.get(), m_entry, SHARED_OWNER);

  // Give this object to the store holder.
  storeHolder->setStore(store.release());

  // Return gracefully:
  return StatusCode::SUCCESS;
}

}  // namespace xAOD::Experimental
