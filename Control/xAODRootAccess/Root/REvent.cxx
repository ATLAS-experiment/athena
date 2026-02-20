// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/REvent.h"

#include "IOUtils.h"
#include "xAODRootAccess/RAuxStore.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TVirtualIncidentListener.h"
#include "xAODRootAccess/tools/RAuxManager.h"
#include "xAODRootAccess/tools/RObjectManager.h"
#include "xAODRootAccess/tools/RAuxFieldManager.h"
#include "xAODRootAccess/tools/ROutObjManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/TIncident.h"
#include "xAODRootAccess/tools/Utils.h"
#include "xAODRootAccess/tools/TEventFormatRegistry.h"

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

std::string  getFieldNameFromKey( const std::string& key ) {

  // build RNTuple field name from the key of the output object
  // RNTuple field names require replacing '.' with ':' for <cont>Aux. or <cont>AuxDyn.<var>
  std::string fieldName = key;
  if (fieldName.rfind("Aux.") != std::string::npos || fieldName.rfind("AuxDyn.") != std::string::npos) { 
    std::replace(fieldName.begin(), fieldName.end(), '.', ':');
  }
  return fieldName;
}


StatusCode getInfoForFieldCreation( const std::string& key, const xAOD::TVirtualManager& mgr, 
                                    std::string& fieldName, std::string& className ) {

  // Get field name from the key
  fieldName = getFieldNameFromKey(key);

  // Get class name from holder 
  // There are two managers to consider: 
  //   ROutObjManager   - standard objects
  //   RAuxFieldManager - aux either simple type (isPrimitive) or - aux non-simple type
  // Check for ROutObjManager
  const xAOD::Experimental::ROutObjManager* omgr = dynamic_cast< const xAOD::Experimental::ROutObjManager* >( &mgr );


  // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - found outObjMgr for key " << key << ", " << omgr);


  if( omgr ) {

    // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - get className ");

    className = omgr->holder()->getClass()->GetName();

    // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - get className " << className);

  }
  else {
    // Check for RAuxFieldManager

    // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - is rauxFieldMgr for key " << key);

    const xAOD::Experimental::RAuxFieldManager* auxmgr = dynamic_cast< const xAOD::Experimental::RAuxFieldManager* >( &mgr );

    // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - found rauxFieldMgr for key " << key << ", " << auxmgr);


    if( auxmgr ) {
      if ( auxmgr->isPrimitive() ) {

        // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - get className for primative ");

        className = xAOD::Utils::getTypeName( *auxmgr->holder()->getTypeInfo() );
      }
      else {

        // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - get className for non-primative ");

        className = auxmgr->holder()->getClass()->GetName();
      }
    }
  }

  // ATH_MSG_DEBUG("REvent::getInfoForFieldCreation - fieldName " << fieldName << " className " << className);

  return StatusCode::SUCCESS;
}


}  // namespace

namespace xAOD::Experimental {

REvent::REvent() : Event("xAOD::Experimental::REvent") {}

REvent::~REvent() {

  // Clear the input and output objects before the input/output files would be
  // closed. Otherwise we can be left with RNTuple view objects pointing
  // nowhere.
  m_inputObjects.clear();
  m_outputObjects.clear();
}

/// Implementation of interface method taking a TFile object. Forwards to the method with a string view below.
StatusCode REvent::readFrom(TFile& inFile) {
  ATH_CHECK(readFrom(inFile.GetName()));
  return StatusCode::SUCCESS;
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


  ATH_MSG_DEBUG("Create RNTupleReader for \"" << METADATA_OBJECT_NAME
                                               << "\" in file: " << fileName);

  // Set up a reader for the metadata ntuple.
  // Since some types are non-xAOD types and so not 'visible' when running in AnalysisBase
  //   we need to protect for unknown types with SetEmulateUnknownTypes(true)
  ROOT::RNTupleDescriptor::RCreateModelOptions opts;
  opts.SetEmulateUnknownTypes(true);
  m_metaReader = ROOT::RNTupleReader::Open(opts, METADATA_OBJECT_NAME, fileName);
  if (!m_metaReader) {
    ATH_MSG_ERROR("Couldn't find \"" << METADATA_OBJECT_NAME
                                     << "\" tree in input file: " << fileName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created RNTupleReader for \"" << METADATA_OBJECT_NAME
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
  const StatusCode sc = readEventFormatMetadata(METADATA_OBJECT_NAME);
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
      if ((keyName != METADATA_OBJECT_NAME) &&
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
  m_eventReader = ROOT::RNTupleReader::Open(EVENT_RNTUPLE_NAME, fileName);
  if (!m_eventReader) {
    ATH_MSG_ERROR("Couldn't access RNTuple \"" << EVENT_RNTUPLE_NAME
                                               << "\" in file: " << fileName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created RNTupleReader for \"" << EVENT_RNTUPLE_NAME
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



/// This function should be called to create a file for writing and 
/// setup the output RNTuple and metadata trees
///
/// @param file the TFile to which the output is writter
///
StatusCode REvent::writeTo(TFile& file) {

  // Save filefor writing
  m_outputFile = &file;
  
  ATH_MSG_DEBUG("REvent::writeTo - opened output file " << m_outputFile->GetName());

  // Access the EventFormat object associated with this file:
  m_outputEventFormat =
      &( TEventFormatRegistry::instance().getEventFormat( m_outputFile ) );

  ATH_MSG_DEBUG("REvent::writeTo - creating RNTupleModel ");

  // Create new model for this file
  m_model = ROOT::RNTupleModel::Create();

  // Set output RNTuple name as model description
  const char*  rnTupleName = EVENT_RNTUPLE_NAME;
  m_model->SetDescription(rnTupleName);

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function needs to be called when the user is done writing events
/// to a file, before (s)he would close the file itself.
///
/// @param file The file that the event data is written to
/// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
///
StatusCode REvent::finishWritingTo(TFile& file) {

  // A small sanity check for meta data writing - need output file as long as MetaData is in a rntuple:
  if( m_outputFile != &file ) {
      ATH_MSG_FATAL("File given to finishWritingTo does not match the file given in writeTo file!");
      return StatusCode::FAILURE;
  }

  // Notify the listeners that they should write out their metadata, if they
  // have any.
  const TIncident incident( IncidentType::MetaDataStop );
  for (auto& listener : m_listeners) {
    listener->handle(incident);
  }

  // Now go to the output file:
  m_outputFile->cd();

  const char*  outRNTupleName = METADATA_OBJECT_NAME;

  // Check if there's already a metadata tree in the output, if so we can return:
  if( m_outputFile->Get( outRNTupleName ) ) {

      // Let's assume that the metadata is complete in the file already.
      ATH_MSG_INFO( "Metadata tree already exists, returning" );

      return StatusCode::SUCCESS;
  }

  // Write out meta data

  // Create new model for this file
  auto model = ROOT::RNTupleModel::Create();

  // Set output RNTuple name as model description
  model->SetDescription(outRNTupleName);


  // Create field explicitly for EventFormat

  // Check if we have a dictionary: for this object:
  std::string typeName = SG::normalizedTypeinfoName( typeid( xAOD::EventFormat ) );
  TClass* cl = TClass::GetClass( typeName.c_str() );
  if( ! cl ) {
      ATH_MSG_ERROR( "Didn't find dictionary for type: " << typeName );
      return StatusCode::FAILURE;
  }
  std::string efName = cl->GetName();

  ATH_MSG_DEBUG("finishWriting");

  model->AddField( ROOT::RFieldBase::Create( "EventFormat", efName ).Unwrap());

  // Loop over output meta data object managers and create the corresponding fields for the output model

  // Make sure that any dynamic auxiliary variables that
  // were added to the object after it was put into the event,
  // get added to the output

  // Must copy m_outputMetaObjects because it may be augmented in putAux
  std::vector<std::pair<std::string, TVirtualManager*>> outputMetaObjects;
  outputMetaObjects.reserve(m_outputMetaObjects.size());
  for (const auto& [key, mgr] : m_outputMetaObjects) {
    TVirtualManager* objMgr = dynamic_cast<TVirtualManager*>(mgr.get());
    if (objMgr == nullptr) {
      ATH_MSG_FATAL("Internal logic error detected");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("finishWriting: save metadata key " << key);

    outputMetaObjects.emplace_back(key, objMgr);
  }

  // Set up the saving of all the dynamic auxiliary properties
  // of the object if it has any:
  for( auto& itr : outputMetaObjects ) {
    static constexpr bool IS_METADATA         = true;
    ATH_CHECK( putAux( *(itr.second), IS_METADATA ) );
  }

  // Now create fields for each output metadata object
  for (auto &[key, mgr] : m_outputMetaObjects) {

    // Get field name and class name to create a RFieldBase
    std::string fieldName;
    std::string className;
    if (getInfoForFieldCreation( key, *mgr, fieldName, className ).isFailure()) return StatusCode::FAILURE;

    // should have found class name
    if ( className.empty() ) {
      ATH_MSG_ERROR( "could not find className!" );
      return StatusCode::FAILURE;
    }

    // Add RFieldBase to model
    model->AddField( ROOT::RFieldBase::Create( fieldName, className ).Unwrap());

  }

  //  Create RNTuple write for meta data with model
  auto metaDataWriter = ROOT::RNTupleWriter::Append(std::move(model), outRNTupleName, *m_outputFile);


  // Get entry for writing
  auto rnEntry = metaDataWriter->GetModel().CreateBareEntry();

  // Now loop over all object managers and bind the output object pointers 
  // to the those in the output metadata RNTuple
  for (auto &[key, mgr] : m_outputMetaObjects) {

      // Get field name from the key
      auto fieldName = getFieldNameFromKey(key);

      // Save value, if exist, otherwise save default value
      if (mgr.get()) {
        rnEntry->BindRawPtr(fieldName, mgr->object());
      } else {
        rnEntry->EmplaceNewValue(fieldName);
      }
  }

  // Add EventFormat to the RNEntry
  rnEntry->BindRawPtr( "EventFormat", m_outputEventFormat );

  // Write the entry, and check the return value:
  const ::Int_t ret = metaDataWriter->Fill(*rnEntry);
  if( ret <= 0 ) {
      ATH_MSG_FATAL( "Output rntuple filling failed with return value: " << ret );
  }
  else {
      ATH_MSG_INFO( "Output meta data rntuple filled. nbytes = " << ret );
  }

  // Now clean up:
  
  // reset output EventFormat
  m_outputEventFormat = 0;
  m_outputObjects.clear();
  m_outputMetaObjects.clear();

  rnEntry.reset();

  /// reset writer (metaDataWriter will be reset when going out of scope)
  m_eventWriter.reset();

  metaDataWriter.reset();
  
  // Return gracefully:
  return StatusCode::SUCCESS;
} // finishWriting


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
/// @param entry The entry from the input RNtuple to load
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
  return (m_outputFile != nullptr);
}

/// This function needs to be called by the user at the end of processing
/// each event that is meant to be written out.
///
/// @returns The number of bytes written if successful, a negative number
///          if not
///
::Int_t REvent::fill() {

  // Make sure that all objects have been read in. The 99 as the value
  // has a special meaning for RAuxStore. With this value it doesn't
  // delete its transient (decoration) variables. Otherwise it does.
  // (As it's supposed to, when moving to a new event.)
  if( m_eventReader ) {
    if (getEntry( m_entry, 99 ) < 0) {
      ATH_MSG_ERROR( "getEntry failed!" );
      return 0;
    }
  }

  // Prepare the objects for writing. Note that we need to iterate over a
  // copy of the m_outputObjects container. Since the putAux(...) function
  // called inside the loop may itself add elements to the m_outputObject
  // container.
  std::string unsetObjects;
  std::vector<std::pair<std::string, TVirtualManager*>> outputObjectsCopy;
  outputObjectsCopy.reserve(m_outputObjects.size());
  for (const auto& [key, mgr] : m_outputObjects) {
    TVirtualManager* objMgr = dynamic_cast<TVirtualManager*>(mgr.get());
    if (objMgr == nullptr) {
      ATH_MSG_ERROR("Internal logic error detected");
      return 0;
    }
    outputObjectsCopy.emplace_back(key, objMgr);
  }
  for (auto &[key, mgr] : outputObjectsCopy) {

    ATH_MSG_DEBUG("REvent::fill - checking and adding aux for " << key << ", " << mgr << " isSet " << mgr->isSet());

    // Check that a new object was provided in the event - skip dynamic variables:
    if ((key.find("AuxDyn") == std::string::npos) && !mgr->isSet()) {

      ATH_MSG_DEBUG("REvent::fill - not set " << key);

      // We are now going to fail. But let's collect the names of
      // all the unset objects:
      if (unsetObjects.size()) {
        unsetObjects += ", ";
      }
      unsetObjects.append("\"" + key + "\"");
      continue;
    }

    // Make sure that any dynamic auxiliary variables that
    // were added to the object after it was put into the event
    // are also added to the output.
    if( putAux( *mgr ).isFailure() ) {
      ATH_MSG_ERROR( "Failed to put dynamic auxiliary variables in the output for object \"" << key << "\"" );
      return 0;
    }
  }

  // Check if there were any unset objects:
  if (unsetObjects.size()) {
    ATH_MSG_ERROR("The following objects were not set in the current event: "
                  << unsetObjects);
    return 0;
  }

  // For the first call to fill, we create an RNTupleWriter with the RNTuple model already accumulated
  // - there may be extensions for Aux containers which are empty for the first few events
  ATH_MSG_DEBUG( "REvent::fill - has event writer " << (m_eventWriter.get() != nullptr) );

  if( m_eventWriter.get() == nullptr ) {

    // Set output RNTuple name as model description
    const char*  rnTupleName = EVENT_RNTUPLE_NAME;

    //  Create RNTuple write with model
    m_eventWriter = ROOT::RNTupleWriter::Append(std::move(m_model), rnTupleName, *m_outputFile);

    // Reset the RNTupleModel so that subsequent additions of new fields will be done with model extensions
    m_model.reset();

    ATH_MSG_DEBUG( "REvent::fill - created writer and reset model " );

  }

  // Get entry for writing
  auto rnEntry = m_eventWriter->GetModel().CreateBareEntry();

  // Now loop over all object managers and bind the output object pointers 
  // to the those in the output RNTuple
  ::Int_t nbytes = 0;
  for (auto &[key, mgr] : m_outputObjects) {

    // Get field name from the key
    auto fieldName = getFieldNameFromKey(key);

    // Check if this is a container, top-level or aux, rather than a aux variable (AuxDyn):
    ROutObjManager* omgr = dynamic_cast< ROutObjManager* >( mgr.get() );
    bool isContainer = (omgr != nullptr);

    ATH_MSG_DEBUG("REvent::fill - bind ptr " << key << " mgr " << mgr << " is set " << mgr->isSet()
                  << ", is container " << isContainer);

    // Save value, if an object has been set, otherwise save default value
    // Dynamic attributes don't exist for empty containers, e.g. no electrons in an event
    if (mgr->isSet()) {
      rnEntry->BindRawPtr(fieldName, mgr->object());
    } else {

      ATH_MSG_DEBUG("REvent::fill - bind unSet ptr " << key << " mgr " << mgr << " is set " << mgr->isSet()
                    << ", is container " << isContainer);

      rnEntry->EmplaceNewValue(fieldName);
    }
  }

  ATH_MSG_DEBUG("REvent::fill - writer fill ");

  // Write the entry, and check the return value:
  const ::Int_t ret = m_eventWriter->Fill(*rnEntry);
  if( ret <= 0 ) {
    ATH_MSG_ERROR( "Output RNTuple filling failed with return value: " << ret );
  }
  nbytes += ret;

  ATH_MSG_DEBUG("REvent::fill - writer filled ");

  // Reset the object managers:
  for (auto &[key, mgr] : m_outputObjects) {
    mgr->reset();

    ATH_MSG_DEBUG("REvent::fill - after reset " << key << " is set " << mgr->isSet());

  }

  // Return the number of bytes written:
  return nbytes;
}


StatusCode REvent::getNames(const std::string& targetClassName,
                         std::vector<std::string>& vkeys,
                         bool metadata) const {

  // The results go in here
  std::set<std::string> keys;

  // Get list of fields from
  // the input metadata tree or input tree
  
  ROOT::RNTupleReader* reader = (metadata) ? m_metaReader.get() : m_eventReader.get();
  if (reader == nullptr) {
    ATH_MSG_ERROR("No input file is connected");
    return StatusCode::FAILURE;
  }
  if (metadata) ATH_MSG_DEBUG("scanning input objects for metadata for type name " << targetClassName);
  else          ATH_MSG_DEBUG("scanning input objects for event data for type name " << targetClassName);

  // add in names for all top level fields
  for (const auto &topLevelField : reader->GetDescriptor().GetTopLevelFields()) {
      std::string objClassName = topLevelField.GetTypeName();
      std::string key = topLevelField.GetFieldName();
      if (objClassName == targetClassName) {
        ATH_MSG_VERBOSE("Matched " << targetClassName << " to key " << key);
        keys.insert(std::move(key));
      }
  }

  // check output objects 
  if (m_eventWriter && !metadata){
      ATH_MSG_DEBUG("scanning output objects for type name " << targetClassName);
      // add in names for all top level fields
      for (const auto& topLevelFieldName : m_eventWriter->GetModel().GetRegisteredSubfieldNames()) {
        auto& topLevelField = m_eventWriter->GetModel().GetConstField(topLevelFieldName);
        std::string objClassName = topLevelField.GetTypeName();
        std::string key = topLevelField.GetFieldName();
        ATH_MSG_VERBOSE("Inspecting " << objClassName << "/" << key);
      if (objClassName == targetClassName) {
          ATH_MSG_VERBOSE("Matched " << targetClassName << " to key " << key);
            keys.insert(std::move(key));
        }
      }
  }

  const Object_t& outAux = ( metadata ?
                              m_outputMetaObjects : m_outputObjects );

  // Search though EventFormat for entries where class matches the provided
  // typeName
  
  ATH_MSG_DEBUG("scanning output Aux objects for type name " << targetClassName);

  for( const auto& object : outAux ) {
      // All metadata objects should be held by ROutObjManager objects.
      // Anything else is an error.
      std::string objClassName;
      if ( metadata ) {
        ROutObjManager* mgr = dynamic_cast< ROutObjManager* >( object.second.get() );
        if ( ! mgr ) continue;
        objClassName = mgr->holder()->getClass()->GetName();
      }
      const std::string& key = object.first;
      ATH_MSG_VERBOSE("Inspecting " << objClassName << "/" << key);
      if (objClassName == targetClassName) {
        ATH_MSG_VERBOSE("Matched " << targetClassName << " to key " << key);
        keys.insert(std::move(key));
      }
  }

  vkeys.insert(vkeys.end(), keys.begin(), keys.end());

  return StatusCode::SUCCESS;
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
  ATH_MSG_DEBUG("Connecting to field \"" << key << "\"");

  // Check if we have metadata about this branch.
  const xAOD::EventFormatElement* ef = nullptr;

  // RNTuples store fields with an "Aux:" postfix instead of "Aux.".
  std::string fieldName = key;
  if (key.ends_with("Aux.")) {
    fieldName.replace(fieldName.size() - 1, 1, ":");
  }

  if (m_inputEventFormat.exists(key) == false) {
    if (!silent) {
      ATH_MSG_WARNING("No metadata available for object: " << key);
    }
  } else {
    ef = m_inputEventFormat.get(key);
  }

  // Check if the field exists in our input RNTuple.
  if (m_eventReader->GetDescriptor().FindFieldId(fieldName.c_str()) ==
      ROOT::kInvalidDescriptorId) {
    // Field doesn't exist
    if (!silent) {
      ATH_MSG_WARNING("Field \"" << fieldName << "\" not available on input");
    }
    m_inputMissingObjects.insert(key);
    return StatusCode::RECOVERABLE;
  }

  // RDS: may need some logic here to get type from inputEventFormat rather than
  // the view to read in with automatic schema evolution

  // Get class name from the field
  ROOT::RNTupleView<void> view =
      m_eventReader->GetView<void>(fieldName.c_str(), nullptr);
  std::string className = view.GetField().GetTypeName();
  if (className == "") {
    if (ef) {
      // This is a fairly weird situation, but let's fall back to taking
      // the class name from the metadata object in this case.
      className = ef->className();
    } else {
      ATH_MSG_ERROR(
          "Couldn't find an appropriate type with a dictionary for field \""
          << fieldName << "\"");
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
        << fieldName << "\"");
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
  Object_t::const_iterator out_itr = m_outputObjects.find( key );
  if( out_itr != m_outputObjects.end() ) {
    // It needs to be an object manager...
    RObjectManager* mgr =
    dynamic_cast< RObjectManager* >( out_itr->second.get() );
    if( ! mgr ) {
      ATH_MSG_ERROR("Couldn't access output manager for: " << key );
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

  // Create the new manager object that will hold this EDM object.
  auto mgr = std::make_unique<RObjectManager>(
      m_eventReader->GetView(fieldName, ptr, className), m_entry,
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
} // connectObject

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
  std::string fieldName = key;
  if (key.ends_with("Aux.")) {
    fieldName.replace(fieldName.size() - 1, 1, ":");
  }

  // Check if the field exists in our input RNTuple.
  if (m_metaReader->GetDescriptor().FindFieldId(fieldName.c_str()) ==
      ROOT::kInvalidDescriptorId) {
    // Field doesn't exist
    if (!silent) {
      ATH_MSG_WARNING("Field \"" << fieldName << "\" not available on input");
    }
    return StatusCode::RECOVERABLE;
  }

  // RDS: may need some logic here to get type from inputEventFormat rather than
  // the view to read in with automatic schema evolution

  // Get class name from the field
  ROOT::RNTupleView<void> view =
      m_metaReader->GetView<void>(fieldName.c_str(), nullptr);
  std::string className = view.GetField().GetTypeName();
  if (className == "") {
    ATH_MSG_ERROR(
        "Couldn't find an appropriate type with a dictionary for field \""
        << fieldName << "\"");
    return StatusCode::FAILURE;
  }
  ::TClass* realClass = ::TClass::GetClass(className.c_str());
  if ((!realClass) || (!realClass->IsLoaded())) {
    // Now we're in trouble...
    ATH_MSG_ERROR(
        "Couldn't find an appropriate type with a dictionary for field \""
        << fieldName << "\"");
    return StatusCode::FAILURE;
  }

  // Create the object and the manager(s) around it.
  void* ptr = realClass->New();
  static const ::Long64_t FIRST_ENTRY = 0;
  auto mgr = std::make_unique<RObjectManager>(
      m_metaReader->GetView(fieldName, ptr, className), FIRST_ENTRY,
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
/// the input. This uses the EDM object that was used to write the auxiliary
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
  static const TClass* const holderClass = TClass::GetClass(typeid(SG::IAuxStoreHolder));
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
/// to the input. This uses the EDM object that was used to write the auxiliary
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


/// This is the function doing the heavy lifting when recording a new
/// object into the output RNTuple. It makes sure that the object is
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
/// @returns <code>kTRUE</code> if the operation was successful, or
///          <code>kFALSE</code> if it was not
///
StatusCode REvent::record( void* obj, 
                           const std::string& typeName,
                           const std::string& key,
                           bool overwrite, 
                           bool metadata,
                           bool isOwner ) {


  // Check if we have an output file when writing an event:
  if ( !hasOutput() ) {
    ATH_MSG_FATAL("No output output file defined. Did you forget to call writeTo(...)?" );
    return StatusCode::FAILURE;
  }
  assert( m_outputEventFormat != 0 );

  ATH_MSG_DEBUG("REvent::record - key, type " << key << ", " << typeName);


  // If this is metadata, just take ownership of it. The object will only
  // be recorded into the output file when calling finishWriting(...).
  if( metadata ) {
    // Check whether we already have such an object:
    if( ( ! overwrite ) &&
        ( m_outputMetaObjects.find( key ) !=
          m_outputMetaObjects.end() ) ) {
      ATH_MSG_FATAL("Meta-object " << typeName << "/" << key << " already recorded" );
      return StatusCode::FAILURE;
    }
    // Check if we have a dictionary for this object:
    TClass* cl = TClass::GetClass( typeName.c_str() );
    if( ! cl ) {
      ATH_MSG_ERROR( "Didn't find dictionary for type: " << typeName );
      return StatusCode::FAILURE;
    }
    // Create output object manager with a holder for the object:
    auto outmgr = std::make_unique<ROutObjManager>( key, std::make_unique<THolder>(obj, cl, isOwner));
    m_outputMetaObjects[key]       = std::move(outmgr);
    // We're done. The rest will be done later on.
    return StatusCode::SUCCESS;
  }

  // Check if we accessed this object on the input. If yes, then this
  // key may not be used for recording.
  if( ( ! overwrite ) &&
    ( m_inputObjects.find( key ) != m_inputObjects.end() ) ) {
    ATH_MSG_FATAL( "Object " << typeName << "/" << key << 
                   " already accessed from the input, can't be overwritten in memory" );
    return StatusCode::FAILURE;
  }

  // Check if we need to add it to the event record:
  Object_t::iterator vitr = m_outputObjects.find( key );
  if( vitr == m_outputObjects.end() ) {

    // Check if we have a dictionary for this object:
    TClass* cl = TClass::GetClass( typeName.c_str() );
    if( ! cl ) {
      ATH_MSG_ERROR( "Didn't find dictionary for type: " << typeName );
      return StatusCode::FAILURE;
    }
    // Check if this is a new object "type" or not:
    if( ! m_outputEventFormat->exists( key ) ) {
      m_outputEventFormat->add( EventFormatElement( key, cl->GetName(),
                                                    "", getHash( key ) ) );
    }
    // Create output object manager with a holder for the object:
    auto outmgr = std::make_unique<ROutObjManager>( key, std::make_unique<THolder>(obj, cl, isOwner));
    ROutObjManager* outmgrPtr = outmgr.get();
    m_outputObjects[ key ] = std::move(outmgr);

    ATH_MSG_DEBUG("REvent::record - save outObjMgr for key, type " << key << ", " << typeName);

    // Set up the saving of all the dynamic auxiliary properties
    // of the object if it has any:
    ATH_CHECK( putAux( *outmgrPtr ) );

    /// Add field to RNTuple model 
    ATH_CHECK( addField(key, *outmgrPtr) );

    // Return at this point, as we don't want to run the rest of
    // the function's code:
    return StatusCode::SUCCESS;
  }

  // Access the object manager:
  ROutObjManager* omgr = dynamic_cast< ROutObjManager* >( vitr->second.get() );
  if( ! omgr ) {
    ATH_MSG_FATAL( "Manager object of the wrong type encountered" );
    return StatusCode::FAILURE;
  }

  // RDS: Is the following still needed?
  // // Check that the type of the object matches that of the previous
  // // object:
  // if( typeName != omgr->holder()->getClass()->GetName() ) {
  //    // This may still be, when the ROOT dictionary name differs from the
  //    // "simple type name" known to C++. So let's get the ROOT name of the
  //    // new type:
  //    TClass* cl = TClass::GetClass( typeName.c_str() );
  //    if( ( ! cl ) || ::strcmp( cl->GetName(),
  //                              omgr->holder()->getClass()->GetName() ) ) {
  //       ATH_MSG_FATAL(
  //                XAOD_MESSAGE( "For output key \"%s\" the previous type "
  //                              "was \"%s\", the newly requested type is "
  //                              "\"%s\"" ),
  //                key.c_str(), omgr->holder()->getClass()->GetName(),
  //                typeName.c_str() );
  //       return StatusCode::FAILURE;
  //    }
  // }

  // Replace the managed object:
  omgr->setObject( obj );

  // Replace the auxiliary objects:
  return putAux( *omgr );
} // record

StatusCode REvent::recordAux(TVirtualManager& mgr, const std::string& key,
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

  // Apparently we didn't recorgnize the auxiliary store type.
  ATH_MSG_ERROR("Unknown auxiliary store manager type encountered");
  return StatusCode::FAILURE;
}


/// This function is used internally to set up the writing of the auxiliary
/// store variables that were dynamically created on an object. (And not
/// statically defined to be part of that object.)
/// The first time an aux store variable is encountered, an RAuxFieldManager
/// is created to manage it. And for each call, the manager holder is set
/// to point to the aux variable.
/// Note: this may be called when recording an aux container, but new aux 
///       variables may be created up to a call to fill
/// One also needs to keep track of empty aux containers on the first event
/// to allow to add in the aux fields to the RNTuple model for the next event
/// which has a non-empty container
///
/// @param mgr              The object manager of the output object
/// @param metadata         Flag specifying whether the info written is metadata or not
/// @returns <code>kTRUE</code> if the setup was successful, or
///         <code>kFALSE</code> if it was not
///
StatusCode REvent::putAux( TVirtualManager& vmgr, ::Bool_t metadata ) {

  // A little sanity check:
  assert( m_outputEventFormat != 0 );

  // Do the conversion:
  ROutObjManager* mgr = dynamic_cast< ROutObjManager* >( &vmgr );

  ATH_MSG_DEBUG("REvent::putAux -  vmgr " << mgr);

  if( ! mgr ) {
      // It's not an error any more when we don't get a ROutObjManager.
      return StatusCode::SUCCESS;
  }

  // Check if this class has an auxiliary store, if not nothing needs to be done:
  if( ! mgr->holder()->getClass()->InheritsFrom( "SG::IAuxStoreIO" ) ) {

    ATH_MSG_DEBUG("REvent::putAux -  no Aux store ");

    return StatusCode::SUCCESS;
  }

  // Get a pointer to the auxiliary store I/O interface:
  SG::IAuxStoreIO* aux = reinterpret_cast< SG::IAuxStoreIO* >(mgr->holder()->getAs( typeid( SG::IAuxStoreIO ) ) );
  if( ! aux ) {
    ATH_MSG_FATAL( "There is a logic error in the code!" );
  }

  // Check if we have rules defined for which auxiliary properties
  // to write out:
  xAOD::AuxSelection sel;
  if( ! metadata ) {
    auto item_itr = m_auxItemList.find( mgr->key() );
    if( item_itr != m_auxItemList.end() ) {
      sel.selectAux( item_itr->second );
    }
  }

  // Get the dynamic auxiliary variables held by this object, which
  // were selected to be written:
  const SG::auxid_set_t auxids = sel.getSelectedAuxIDs (aux->getSelectedAuxIDs());

  // Decide what should be the prefix of all the dynamic branches:
  const std::string dynNamePrefix = Utils::dynBranchPrefix( mgr->key() );

  ATH_MSG_DEBUG("REvent::putAux -  dynNamePrefix " << dynNamePrefix);


  // Select which container to add the variables to:
  Object_t& objects = ( metadata ? m_outputMetaObjects : m_outputObjects );

  // RDS: is the following needed in an RNTuple?
  // This iteration will determine the ordering of branches within
  // the tree, so sort auxids by name.
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  typedef std::pair< std::string, SG::auxid_t > AuxVarSort_t;
  std::vector< AuxVarSort_t > varsort;
  varsort.reserve( auxids.size() );
  for( SG::auxid_t id : auxids ) {
      varsort.emplace_back( r.getName( id ), id );
  }
  std::sort( varsort.begin(), varsort.end() );

  // Extract all the dynamic variables from the object:
  for( const auto& p : varsort ) {

    // The auxiliary ID:
    const SG::auxid_t id = p.second;

    // Construct a dynamic key name for the field that we will write:
    const std::string dynKey = dynNamePrefix + p.first;

    ATH_MSG_DEBUG("REvent::putAux -  id, dynKey " << id << ", " << dynKey);


    // Try to find the object manager:
    Object_t::iterator bmgr = objects.find( dynKey );

    // Check if we already know about this variable:
    if( bmgr == objects.end() ) {
      // Construct the full type name of the variable:
      const std::type_info* brType = aux->getIOType( id );
      if( ! brType ) {
        ATH_MSG_FATAL( "No I/O type found for variable " << dynKey );
        return StatusCode::FAILURE;
      }
      const std::string brTypeName = Utils::getTypeName( *brType );
      std::string brProperTypeName = "<unknown>";

      // Check if it's a primitive type or not:
      bool isPrimitive = false;
      TVirtualManager* outmgrPtr = nullptr;

      if( strlen( brType->name() ) == 1 ) {
        isPrimitive = true;

        ATH_MSG_DEBUG("REvent::putAux -  primitive " << id << ", " << brTypeName << " brType name " << brType->name());


        // Making the "proper" type name is simple in this case:
        brProperTypeName = std::move(brTypeName);

        // Let's create an RAuxFieldManager for this property:
        static constexpr bool IS_OWNER = false;
        auto auxmgr = std::make_unique<RAuxFieldManager>( 
          std::make_unique<THolder>(aux->getIOData( id ), *brType, IS_OWNER), isPrimitive );
        outmgrPtr = auxmgr.get();

        objects[ dynKey ] = std::move(auxmgr);

      } else {

        ATH_MSG_DEBUG("REvent::putAux -  not primitive " << id << ", " << brTypeName);

        // Check if we have a dictionary for this type:
        TClass* cl = TClass::GetClass( *brType, kTRUE, kTRUE );
        if( ! cl ) {
          // The dictionary needs to be loaded now. This could be an
          // issue. But let's hope for the best...
          cl = TClass::GetClass( brTypeName.c_str() );
          // If still not found...
          if( ! cl ) {
            ATH_MSG_FATAL( "Dictionary not available for variable \"" << dynKey 
                            << "\" of type \"" << brTypeName << "\"" );
            return StatusCode::FAILURE;
          }
        }

        // The proper type name comes from the dictionary in this case:
        brProperTypeName = cl->GetName();

        // Let's create an RAuxFieldManager for this property - not a primitive:
        static constexpr bool IS_OWNER = false;
        auto auxmgr = std::make_unique<RAuxFieldManager>( 
          std::make_unique<THolder>(aux->getIOData( id ), cl, IS_OWNER), isPrimitive );
        outmgrPtr = auxmgr.get();
        objects[ dynKey ] = std::move(auxmgr);
      }

      // For event data, add in the the new fields to the RNTuple model and event format metadata
      if (!metadata) {

        /// Add field to RNTuple model 
        ATH_CHECK( addField(dynKey, *outmgrPtr) );

        // If all went fine, let's add this branch to the event format
        // metadata (but not for regular metadata):
        if( ! m_outputEventFormat->exists( dynKey ) ) {
            m_outputEventFormat->add(
                  EventFormatElement( dynKey,
                                      brProperTypeName,
                                      mgr->key(),
                                      getHash( dynKey ) ) );
        }
      }

      // We don't need to do the rest:
      continue;
    } 

    ATH_MSG_DEBUG("REvent::putAux -  setObj " << dynKey);

    // Access the object manager:
    bmgr = objects.find( dynKey );
    if( bmgr == objects.end() ) {
      ATH_MSG_FATAL( "There is an internal logic error in the code..." );
    }
    // Replace the managed object:
    void* nc_data ATLAS_THREAD_SAFE = // we hold non-const pointers but check on retrieve
      const_cast< void* >( static_cast< const void* >( aux->getIOData( id ) ) );
    bmgr->second->setObject( nc_data );
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}


/// Add field to RNTuple model given the StoreGate key and output object manager
StatusCode REvent::addField(const std::string& key, const TVirtualManager& mgr) {

  // Add to the RNTuple model -
  //   This is done by either adding the field to  the initial model before the first event is written, or
  //   after the first event has been written a new field is added via an extension of the output RNTuple model.
  //   This can occurs when an AuxStore is empty for the initial events

  // Get field name and class name to create a RFieldBase
  std::string fieldName;
  std::string className;
  if (getInfoForFieldCreation( key, mgr, fieldName, className ).isFailure()) return StatusCode::FAILURE;
  // should have found class name
  if ( className.empty() ) {
    ATH_MSG_ERROR( "could not find className!" );
    return StatusCode::FAILURE;
  }

  if (m_model) {
    // Continue to add to the initial RNTuple model
    m_model->AddField( ROOT::RFieldBase::Create( fieldName, className ).Unwrap());
  }
  else {
    if (m_eventWriter.get() == nullptr) {
      ATH_MSG_ERROR("Internal logic error detected - no output found");
      return StatusCode::FAILURE;
    }
    // After the first event, one needs to extend the output RNTuple model
    auto field = ROOT::RFieldBase::Create(fieldName, className).Unwrap();
    auto updater = m_eventWriter->CreateModelUpdater();
    updater->BeginUpdate();
    updater->AddField(std::move(field));
    updater->CommitUpdate();
  }

  // Return gracefully:
  return StatusCode::SUCCESS;

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
