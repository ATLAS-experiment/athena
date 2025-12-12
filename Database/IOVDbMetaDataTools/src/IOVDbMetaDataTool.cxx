/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   IOVDbMetaDataTool.cxx
 * 
 * @brief This is a tool used to manage the IOV Meta Data for a given
 * object into the Meta Data Store.
 * 
 * @author Antoine Perus <perus@lal.in2p3.fr>
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 *
 */

#include "IOVDbMetaDataTool.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventIDRange.h"

#include "StoreGate/StoreGateSvc.h"
#include "IOVDbDataModel/IOVMetaDataContainer.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaKernel/IOVTime.h"
#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/CondContMaker.h"
#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/IRCUSvc.h"
#include "CoralBase/AttributeListSpecification.h"
#include "nlohmann/json.hpp"


IOVDbMetaDataTool::IOVDbMetaDataTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface*  parent)
  : base_class(type, name, parent)
  , m_metaDataStore ("StoreGateSvc/MetaDataStore",      name)
  , m_inputStore    ("StoreGateSvc/InputMetaDataStore", name)
  , m_condStore     ("StoreGateSvc/ConditionStore",     name)
  , m_overrideRunNumber(false)
  , m_overrideMinMaxRunNumber(false)
  , m_newRunNumber(0)
  , m_oldRunNumber(0)
  , m_minRunNumber(0)
  , m_maxRunNumber(0)
  , m_modifyFolders(false)
{
}

//--------------------------------------------------------------------------
IOVDbMetaDataTool::~IOVDbMetaDataTool()
{}


//--------------------------------------------------------------------------

StatusCode IOVDbMetaDataTool::initialize()
{
  ATH_MSG_DEBUG("in initialize()");

  // Set to be listener for FirstInputFile
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", this->name());
  ATH_CHECK(incSvc.retrieve());
  incSvc->addListener(this, "FirstInputFile", 60); // pri has to be < 100 to be after MetaDataSvc.

  // locate the meta data stores
  ATH_CHECK(m_metaDataStore.retrieve());
  ATH_CHECK(m_inputStore.retrieve());

  // Check whether folders need to be modified
  m_modifyFolders = (m_foldersToBeModified.value().size()>0);
  ATH_MSG_DEBUG("initialize(): " << (m_modifyFolders ? "" : "No ") << "need to modify folders");

  // Process direct payloads if configured
  if (!m_payloads.value().empty()) {
    // Group by folder: parse "folder:key" format from flat map
    std::map<std::string, std::map<std::string, std::string>> folderPayloads;

    for (const auto& [key, value] : m_payloads.value()) {
      // Split key on ':' to get folder and parameter name
      size_t colonPos = key.find(':');
      if (colonPos == std::string::npos) {
        ATH_MSG_ERROR("Invalid payload key format: " << key << " (expected 'folder:key')");
        return StatusCode::FAILURE;
      }

      std::string folderName = key.substr(0, colonPos);
      std::string paramName = key.substr(colonPos + 1);
      folderPayloads[folderName][paramName] = value;
    }

    ATH_MSG_DEBUG("Processing " << folderPayloads.size() << " folder(s) for direct payload registration");

    // Store payloads for ConditionStore registration (after MetaDataStore registration)
    std::map<std::string, std::pair<std::unique_ptr<CondAttrListCollection>, EventIDRange>> payloadsForCondStore;

    for (const auto& [folderName, parameters] : folderPayloads) {
      // Extract beginRun and endRun from parameters
      if (!parameters.contains("beginRun") || !parameters.contains("endRun")) {
        ATH_MSG_ERROR("Payload for folder " << folderName << " missing beginRun or endRun");
        return StatusCode::FAILURE;
      }

      unsigned int beginRun = std::stoul(parameters.at("beginRun"));
      unsigned int endRun = std::stoul(parameters.at("endRun"));

      // Create filtered parameters map without beginRun/endRun
      std::map<std::string, std::string> filteredParams;
      std::ranges::copy_if(parameters, std::inserter(filteredParams, filteredParams.end()),
                           [](const auto& p) { return p.first != "beginRun" && p.first != "endRun"; });

      ATH_MSG_DEBUG("Registering folder " << folderName << " with " << filteredParams.size()
                    << " parameters, IOV [" << beginRun << ", " << endRun << "]");

      ATH_CHECK(registerFolder(folderName));

      // Build payload for MetaDataStore
      // Note: AttributeListSpecification has protected destructor, must use new
      // The AttributeList constructor with 'true' takes ownership and will call release()
      coral::AttributeListSpecification* spec = new coral::AttributeListSpecification();
      for (const auto& [key, value] : filteredParams) {
        spec->extend(key, "string");
      }

      coral::AttributeList attrList(*spec, true);
      for (const auto& [key, value] : filteredParams) {
        attrList[key].setValue(value);
      }

      auto payload = std::make_unique<CondAttrListCollection>(true);
      payload->addNewStart(IOVTime(beginRun, 0));
      payload->addNewStop(IOVTime(endRun, IOVTime::MAXEVENT));
      payload->add(0, attrList);

      ATH_MSG_DEBUG("Created payload with IOV [" << beginRun << ", " << endRun << "]");

      // Store payload copy for ConditionStore registration
      EventIDRange iovRange(EventIDBase(beginRun, EventIDBase::UNDEFEVT, 0, 0, 0),
                            EventIDBase(endRun, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM));
      payloadsForCondStore[folderName] = std::make_pair(std::make_unique<CondAttrListCollection>(*payload), iovRange);

      // Add to MetaDataStore
      ATH_CHECK(addPayload(folderName, payload.release()));
    }

    // Now populate ConditionStore with all payloads
    if (!payloadsForCondStore.empty()) {
      ATH_MSG_INFO("Registering " << payloadsForCondStore.size() << " payload(s) to ConditionStore");

      ATH_CHECK(m_condStore.retrieve());

      ServiceHandle<Athena::IRCUSvc> rcuSvc("Athena::RCUSvc", name());
      ATH_CHECK(rcuSvc.retrieve());

      for (auto& [folderName, payloadPair] : payloadsForCondStore) {
        auto& [payload, iovRange] = payloadPair;

        // Check if CondCont<AthenaAttributeList> already exists using contains() to avoid warnings
        CondCont<AthenaAttributeList>* cc = nullptr;
        if (!m_condStore->contains<CondCont<AthenaAttributeList>>(folderName)) {
          ATH_MSG_INFO("Creating CondCont<AthenaAttributeList> for " << folderName);

          SG::DataObjectSharedPtr<DataObject> cb =
            CondContainer::CondContFactory::Instance().Create(*rcuSvc,
                                                               ClassID_traits<AthenaAttributeList>::ID(),
                                                               folderName);
          if (!cb) {
            ATH_MSG_ERROR("Failed to create CondCont for " << folderName);
            return StatusCode::FAILURE;
          }

          if (m_condStore->recordObject(cb, folderName, true, false) == nullptr) {
            ATH_MSG_ERROR("Failed to record CondCont for " << folderName);
            return StatusCode::FAILURE;
          }
        }

        ATH_CHECK(m_condStore->retrieve(cc, folderName));

        // Extract AthenaAttributeList from CondAttrListCollection
        if (payload->size() != 1) {
          ATH_MSG_ERROR("Expected single-entry CondAttrListCollection for " << folderName << ", got " << payload->size() << " entries");
          return StatusCode::FAILURE;
        }

        auto itr = payload->begin();
        const coral::AttributeList& attrList = itr->second;
        auto athAttrList = std::make_unique<AthenaAttributeList>(attrList);

        ATH_CHECK(cc->insert(iovRange, std::move(athAttrList), Gaudi::Hive::currentContext()));
        ATH_MSG_INFO("Inserted payload into CondCont for " << folderName << " with IOV " << iovRange);
      }
    }
  }

  return(StatusCode::SUCCESS);
}

//--------------------------------------------------------------------------

StatusCode 
IOVDbMetaDataTool::finalize()
{
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

void IOVDbMetaDataTool::handle(const Incident& inc)
{
  const FileIncident* fileInc  = dynamic_cast<const FileIncident*>(&inc);
  if(!fileInc) throw std::runtime_error("Unable to get FileName from FirstInputFile incident");

  const std::string fileName = fileInc->fileName();
  ATH_MSG_DEBUG("handle() " << inc.type() << " for " << fileName);

  // Check if we need to override run number - only needed for simulation
  checkOverrideRunNumber();

  StatusCode sc = processInputFileMetaData(fileName);
  if(!sc.isSuccess()) throw std::runtime_error("Could not process input file meta data");
  m_filesProcessed.insert(std::move(fileName));
}

StatusCode IOVDbMetaDataTool::beginInputFile(const SG::SourceID& sid)
{
  if (!m_filesProcessed.contains(sid)) {
    ATH_CHECK(processInputFileMetaData(sid));
    m_filesProcessed.insert(sid);
  }
  return StatusCode::SUCCESS;
}

StatusCode IOVDbMetaDataTool::endInputFile(const SG::SourceID&)
{
  return StatusCode::SUCCESS;
}

StatusCode IOVDbMetaDataTool::serializeIOVMetadataToBSMetadata()
{
  // Check if we have folders to serialize to ByteStream metadata
  if (m_foldersToSerializeToBSMetadata.value().empty()) {
    ATH_MSG_DEBUG("No folders configured for serialization");
    return StatusCode::SUCCESS;
  }

  // Collect IOV metadata strings first
  std::vector<std::string> iovMetaStrings;
  for (const std::string& folderName : m_foldersToSerializeToBSMetadata.value()) {
    IOVMetaDataContainer* container = findMetaDataContainer(folderName);
    if (!container) {
      ATH_MSG_WARNING("Could not find IOVMetaDataContainer for folder " << folderName << ", skipping");
      continue;
    }

    std::string jsonStr = serializeContainerToJSON(container);
    if (!jsonStr.empty()) {
      iovMetaStrings.push_back("IOVMeta." + folderName + "=" + jsonStr);
      ATH_MSG_DEBUG("Serialized folder " << folderName << " (" << jsonStr.size() << " bytes JSON)");
    }
  }

  if (iovMetaStrings.empty()) {
    ATH_MSG_DEBUG("No IOV metadata serialized");
    return StatusCode::SUCCESS;
  }

  // Store the IOV metadata strings in MetaDataStore for ByteStreamCnvSvc to retrieve
  // Check if the object already exists and update it, or create a new one
  if (m_metaDataStore->contains<std::vector<std::string>>("IOVMetaDataStrings")) {
    // Retrieve existing and append
    std::vector<std::string>* existingStrings = nullptr;
    ATH_CHECK(m_metaDataStore->retrieve(existingStrings, "IOVMetaDataStrings"));
    existingStrings->insert(existingStrings->end(), iovMetaStrings.begin(), iovMetaStrings.end());
    ATH_MSG_DEBUG("Appended " << iovMetaStrings.size() << " IOV metadata strings to existing collection");
  } else {
    // Create new
    auto iovMetaData = std::make_unique<std::vector<std::string>>(std::move(iovMetaStrings));
    ATH_CHECK(m_metaDataStore->record(std::move(iovMetaData), "IOVMetaDataStrings"));
    ATH_MSG_DEBUG("Stored " << iovMetaStrings.size() << " IOV metadata strings in MetaDataStore");
  }

  return StatusCode::SUCCESS;
}

StatusCode IOVDbMetaDataTool::metaDataStop()
{
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

void
IOVDbMetaDataTool::checkOverrideRunNumber() 
{
    ATH_MSG_DEBUG("begin checkOverrideRunNumber");

    // Check if override run numbers have been set by properties or by
    // the EventSelector

    if (m_minMaxRunNumbers.value().size() > 0) {
        m_overrideMinMaxRunNumber = true;
        m_minRunNumber = m_minMaxRunNumbers.value()[0];
        if (m_minMaxRunNumbers.value().size() > 1) m_maxRunNumber = m_minMaxRunNumbers.value()[1];
        else m_maxRunNumber = m_minRunNumber;

        if (m_maxRunNumber > IOVTime::MAXRUN) m_maxRunNumber = IOVTime::MAXRUN;
        
        ATH_MSG_INFO("checkOverrideRunNumber: overriding IOV for range - min: " << m_minRunNumber 
                      << " max: " << m_maxRunNumber);
        return;
    }

    ATH_MSG_DEBUG("checkOverrideRunNumber: check if tag is set in jobOpts");

    // Get name of event selector from the application manager to
    // make sure we get the one for MC signal events
    SmartIF<IProperty> appMgr{serviceLocator()->service("ApplicationMgr")};
    if (!appMgr) {
        ATH_MSG_ERROR("checkOverrideRunNumber: Cannot get ApplicationMgr "); 
        return;
    }
    StringProperty property("EvtSel", "");
    StatusCode sc = appMgr->getProperty(&property);
    if (!sc.isSuccess()) {
        ATH_MSG_ERROR("checkOverrideRunNumber: unable to get EvtSel: found " << property.value());
        return;
    }
    // Get EventSelector for ApplicationMgr
    const std::string eventSelector = property.value();
    SmartIF<IProperty> evtSel{serviceLocator()->service(eventSelector)};
    if (!evtSel) {
        ATH_MSG_ERROR("checkOverrideRunNumber: Cannot get EventSelector " << eventSelector); 
        return;
    }

    // Is flag set to override the run number? 
    BooleanProperty overrideRunNumber("OverrideRunNumberFromInput", false);
    sc = evtSel->getProperty(&overrideRunNumber);
    if (!sc.isSuccess()) {
        // Not all EventSelectors have this property, so we must be tolerant
        ATH_MSG_DEBUG("resetRunNumber: unable to get OverrideRunNumberFromInput property from EventSelector ");
        return;
    }
    m_overrideRunNumber = overrideRunNumber.value();
    if (m_overrideRunNumber) {
        // New run number
        IntegerProperty runNumber("RunNumber", 0);
        sc = evtSel->getProperty(&runNumber);
        if (!sc.isSuccess()) {
            ATH_MSG_ERROR("checkOverrideRunNumber: unable to get RunNumber from EventSelector: found "
                          << runNumber.value());
            return;
        }
        m_newRunNumber = runNumber.value();
        // Old run number
        IntegerProperty oldRunNumber("OldRunNumber", 0);
        sc = evtSel->getProperty(&oldRunNumber);
        if (!sc.isSuccess()) {
            ATH_MSG_ERROR("checkOverrideRunNumber: unable to get OldRunNumber from EventSelector: found "
                          << oldRunNumber.value());
            return;
        }
        m_oldRunNumber = oldRunNumber.value();

        ATH_MSG_DEBUG("checkOverrideRunNumber: Changing old to new run number:  " << m_oldRunNumber
                      << " " << m_newRunNumber << " obtained from " << eventSelector);
    }
    else ATH_MSG_DEBUG("checkOverrideRunNumber: OverrideRunNumberFromInput not set for " << eventSelector);
}

//--------------------------------------------------------------------------

StatusCode
IOVDbMetaDataTool::registerFolder(const std::string& folderName) const
{
    // Set the default folder description for a CondAttrListCollection
    // which will be read back via IOVDbSvc
    std::string folderDescr = "<timeStamp>run-event</timeStamp><addrHeader><address_header service_type=\"256\" clid=\"1238547719\" /> </addrHeader><typeName>CondAttrListCollection</typeName>" ;

    return registerFolder(folderName, folderDescr);
}

//--------------------------------------------------------------------------

StatusCode
IOVDbMetaDataTool::registerFolder(const std::string& folderName, 
                                  const std::string& folderDescription) const
{
    // lock the tool before getMetaDataContainer() call
    std::scoped_lock  guard( m_mutex );

    ATH_MSG_DEBUG("begin registerFolder ");

    if( ! getMetaDataContainer(folderName, folderDescription) ) {
        ATH_MSG_ERROR("Unable to register folder " << folderName);
        return(StatusCode::FAILURE);
    } 
    else {
        ATH_MSG_DEBUG("IOVMetaDataContainer  for folder " << folderName << " has been registered ");
    }

    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

StatusCode IOVDbMetaDataTool::addPayload (const std::string& folderName
					  , CondAttrListCollection* payload) const
{
  // lock the tool while it is modifying the folder
  std::scoped_lock  guard( m_mutex );

  ATH_MSG_DEBUG("begin addPayload ");
  
  // Check if the  folder has already been found
  IOVMetaDataContainer* cont = m_metaDataStore->tryRetrieve<IOVMetaDataContainer>(folderName);
  if(cont) {
    ATH_MSG_DEBUG("Retrieved IOVMetaDataContainer from MetaDataStore for folder " 
		  << folderName);
  }
  else {
    ATH_MSG_ERROR("addPayload: Could not find IOVMetaDataContainer in MetaDataStore for folder " 
		   << folderName 
		   << ". One must have previously called registerFolder. ");
     return StatusCode::FAILURE;
  }

  // Override run number if requested
  if (m_overrideRunNumber || m_overrideMinMaxRunNumber) {
    ATH_CHECK( overrideIOV(payload) );
  }

  // Add payload to container
  bool success = cont->merge(payload);
  if (success) {
    ATH_MSG_DEBUG("Added new payload for folder " << folderName);
  }
  else {
    ATH_MSG_DEBUG("Could not add new payload for folder " 
		  << folderName 
		  << " (may be duplicate payload).");

    // To Do: the function implicitly assumes ownership on the payload pointer
    delete payload;
    payload = nullptr;
  }

  // Debug printout
  if(payload && msgLvl(MSG::DEBUG)) {
    std::ostringstream stream;
    payload->dump(stream);
    ATH_MSG_DEBUG(stream.str());
  }

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

StatusCode
IOVDbMetaDataTool::modifyPayload (const std::string& folderName, 
                                  CondAttrListCollection*& coll) const
{
    // protected by lock in processInputFileMetaData()

    /// Modify a Payload for a particular folder - replaces one of the
    /// internal attributes
    ATH_MSG_DEBUG("begin modifyPayload for folder " << folderName);

    // check if this folder needs to be modified
    bool modifyAttr = false;
    std::string attributeName;
    const std::vector<std::string>& folders = m_foldersToBeModified.value();
    const std::vector<std::string>& attrs   = m_attributesToBeRemoved.value();
    for (unsigned int i = 0; i < folders.size(); ++i) {
        if (folderName == folders[i]) {
            if (attrs.size() > i) {
                attributeName = attrs[i];
                modifyAttr    = true;
                ATH_MSG_DEBUG("modifyPayload: remove attribute " << attributeName);
                break;
            }
        }
    }

    if (!modifyAttr) {
        ATH_MSG_DEBUG("modifyPayload: folder " << folderName << " OK ");
        return StatusCode::SUCCESS;
    }

    bool iovSizeIsZero = coll->iov_size() == 0;
    IOVRange testIOV = coll->minRange();
    IOVTime  start   = testIOV.start();
    IOVTime  stop    = testIOV.stop();
    // Set the IOV
    CondAttrListCollection* coll1 = new CondAttrListCollection(true);
    if (iovSizeIsZero) { 
        // Only add in overall range if channels do not have
        // IOVs - otherwise this is automatically calculated
        coll1->addNewStart(start);
        coll1->addNewStop (stop);
    }
    // Add in channels
    unsigned int nchans = coll->size();
    bool hasChanNames = (coll->name_size() == nchans);
    for (unsigned int ichan = 0; ichan < nchans; ++ichan) {
        CondAttrListCollection::ChanNum chan = coll->chanNum(ichan);
        // Now filter out the unwanted attribute
        CondAttrListCollection::AttributeList  newAttrList;
        const CondAttrListCollection::AttributeList& oldAttrList = coll->attributeList(chan);
        for (unsigned int iatt = 0; iatt < oldAttrList.size(); ++iatt) {
            // skip the unwanted attribute
            if (attributeName == oldAttrList[iatt].specification().name()) {
                ATH_MSG_DEBUG("modifyPayload: skipping attribute name " << oldAttrList[iatt].specification().name());
                continue;
            }
                
            // copy the rest
            newAttrList.extend(oldAttrList[iatt].specification().name(),
                               oldAttrList[iatt].specification().type());
            const coral::Attribute&  oldAttr = oldAttrList[iatt];
            coral::Attribute&        newAttr = newAttrList[oldAttrList[iatt].specification().name()];
            newAttr = oldAttr;
            // newAttr.setValue(oldAttr.data());
            ATH_MSG_DEBUG("modifyPayload: copying attr name " 
                          << oldAttrList[iatt].specification().name() << " " 
                          /*<< newAttr*/);
        }
        coll1->add(chan, newAttrList);
        if (!iovSizeIsZero) coll1->add(chan, coll->iovRange(chan));
        if(hasChanNames)coll1->add(chan, coll->chanName(chan));
        ATH_MSG_DEBUG("modifyPayload: copied attribute list for channel " << chan);
    }
    delete coll;
    coll = coll1;
    if (msgLvl(MSG::DEBUG)) {
      std::ostringstream stream;
      coll->dump(stream);
      ATH_MSG_DEBUG(stream.str());
    }

    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

IOVMetaDataContainer*
IOVDbMetaDataTool::findMetaDataContainer(const std::string& folderName) const
{
  // lock the tool before this call
  // Return the folder if it is in the meta data store
  return m_metaDataStore->tryRetrieve<IOVMetaDataContainer>(folderName);
}


IOVMetaDataContainer* 
IOVDbMetaDataTool::getMetaDataContainer(const std::string& folderName
                                        , const std::string& folderDescription) const
{
  // protected by locks in addPayload() and registerFolder()
  ATH_MSG_DEBUG("begin getMetaDataContainer ");

  IOVMetaDataContainer* cont{nullptr};
  // See if it is in the meta data store
  if (!m_metaDataStore->contains<IOVMetaDataContainer>(folderName)) {
    // Container not found, add in new one
    cont = new IOVMetaDataContainer(folderName, folderDescription);
    ATH_MSG_DEBUG("No IOVMetaDataContainer in MetaDataStore for folder " << folderName
		  << ". Created a new instance");
    StatusCode sc = m_metaDataStore->record(cont, folderName);
    if (!sc.isSuccess()) {
      ATH_MSG_ERROR("Could not record IOVMetaDataContainer in MetaDataStore for folder " << folderName);
      delete cont;
      cont = nullptr;
    }
  } 
  else {
    ATH_MSG_DEBUG("IOVMetaDataContainer already in MetaDataStore for folder " << folderName);
    StatusCode sc = m_metaDataStore->retrieve(cont, folderName);
    if (!sc.isSuccess()) {
      ATH_MSG_ERROR("Could not retrieve IOVMetaDataContainer in MetaDataStore for folder " << folderName);
      cont = nullptr;
    }
  }
  return cont;
}

//--------------------------------------------------------------------------

StatusCode IOVDbMetaDataTool::processInputFileMetaData(const std::string& fileName)
{
  // lock the tool while it is processing input metadata 
  std::scoped_lock  guard( m_mutex );

  ATH_MSG_DEBUG("processInputFileMetaData: file name " << fileName);

  // Retrieve all meta data containers from InputMetaDataStore
  SG::ConstIterator<IOVMetaDataContainer> cont;
  SG::ConstIterator<IOVMetaDataContainer> contEnd;

  StatusCode sc = m_inputStore->retrieve(cont, contEnd);
  if (!sc.isSuccess()) {
    ATH_MSG_DEBUG("processInputFileMetaData: Could not retrieve IOVMetaDataContainer objects from InputMetaDataStore - cannot process input file meta data");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("processInputFileMetaData: Retrieved from IOVMetaDataContainer(s) from InputMetaDataStore");

  // For each container, merge its contents into the MetaDataStore 
  unsigned int ncolls    = 0;
  unsigned int ndupColls = 0;
  for (; cont != contEnd; ++cont) {
    IOVMetaDataContainer* contMaster = getMetaDataContainer(cont->folderName()
							    , cont->folderDescription());

    // We assume that the folder is the same for all versions, and
    // now we loop over versions for the payloads
    std::list<SG::ObjectWithVersion<IOVMetaDataContainer> > allVersions;
    sc = m_inputStore->retrieveAllVersions(allVersions, cont.key());
    if (!sc.isSuccess()) {
      ATH_MSG_ERROR("Could not retrieve all versions for " << cont.key());
      return sc;
    }

    for (SG::ObjectWithVersion<IOVMetaDataContainer>& obj : allVersions) {
      const IOVPayloadContainer*  payload = obj.dataObject->payloadContainer();

      ATH_MSG_DEBUG("processInputFileMetaData: New container: payload size " << payload->size() << " version key " << obj.versionedKey);

      // detailed printout before merge
      if (msgLvl(MSG::VERBOSE)) {
	const IOVPayloadContainer*  payloadMaster = contMaster->payloadContainer();
	ATH_MSG_VERBOSE("Before merge, payload minRange for folder " << cont->folderName());
	if (payloadMaster && payloadMaster->size()) {
	  // Loop over AttrColls and print out minRange
	  IOVPayloadContainer::const_iterator itColl    = payloadMaster->begin();
	  IOVPayloadContainer::const_iterator itCollEnd = payloadMaster->end();
	  unsigned int iPayload = 0;
	  for (; itColl != itCollEnd; ++itColl, ++iPayload) {
	    ATH_MSG_VERBOSE(iPayload << " " << (*itColl)->minRange() << " " 
			    << (*itColl)->size());
	  }
	}
	else { 
	  ATH_MSG_VERBOSE("  no payloads yet!"); 
	}
      }
    }
        
    // Detailed printout
    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("processInputFileMetaData: Current payload before merge " << contMaster->folderName());
      IOVPayloadContainer::const_iterator itColl1    = contMaster->payloadContainer()->begin();
      IOVPayloadContainer::const_iterator itCollEnd1 = contMaster->payloadContainer()->end();
      std::ostringstream stream;
      for (; itColl1 != itCollEnd1; ++itColl1) (*itColl1)->dump(stream);
      ATH_MSG_DEBUG(stream.str());
    }

    //
    // Loop over CondAttrListCollections and do merge
    //
    for (SG::ObjectWithVersion<IOVMetaDataContainer>& obj : allVersions) {
      const IOVPayloadContainer*  payload = obj.dataObject->payloadContainer();
      IOVPayloadContainer::const_iterator itColl    = payload->begin();
      IOVPayloadContainer::const_iterator itCollEnd = payload->end();
      for (; itColl != itCollEnd; ++itColl) {

	// Make a copy of the collection and merge it into
	// master container in meta data store 
	CondAttrListCollection* coll = new CondAttrListCollection(**itColl);
	// Override run number if requested
	if (m_overrideRunNumber || m_overrideMinMaxRunNumber) {
          ATH_CHECK( overrideIOV(coll) );
        }

	// first check if we need to modify the incoming payload
	if (!modifyPayload (contMaster->folderName(), coll).isSuccess()) {
          ATH_MSG_ERROR("processInputFileMetaData: Could not modify the payload for folder " << contMaster->folderName());
          return StatusCode::FAILURE;
        }

	ATH_MSG_VERBOSE("processInputFileMetaData: merge minRange: " << coll->minRange());
	if (!contMaster->merge(coll)) {
	  // Did not merge it in - was a duplicate, so we need to delete it 
	  delete coll;
	  ++ndupColls;
	  ATH_MSG_VERBOSE(" => not merged ");
	}
	else {
	  ++ncolls;
	  ATH_MSG_VERBOSE(" => merged ");
	}

      }
      ATH_MSG_DEBUG("processInputFileMetaData: Merged together containers for folder " << cont->folderName() << " ncoll/ndup " 
		    << ncolls << " " << ndupColls);

      // Check for consistency after merge
      const IOVPayloadContainer*  payloadMaster = contMaster->payloadContainer();
      if (payloadMaster && payloadMaster->size()) {
	// Loop over AttrColls and print out minRange
	IOVPayloadContainer::const_iterator itColl    = payloadMaster->begin();
	IOVPayloadContainer::const_iterator itCollEnd = payloadMaster->end();
	IOVTime lastStop;
	if ((*itColl)->minRange().start().isTimestamp()) lastStop = IOVTime(0);
	else lastStop = IOVTime(0,0);
	bool hasError = false;
	for (; itColl != itCollEnd; ++itColl) {
	  if ((*itColl)->minRange().start() < lastStop) hasError = true;
	  lastStop = (*itColl)->minRange().stop();
	}
	if (hasError) {
	  ATH_MSG_ERROR("processInputFileMetaData: error after merge of file meta data. " );
	  ATH_MSG_ERROR("processInputFileMetaData: Filename " << fileName);
	  ATH_MSG_ERROR("processInputFileMetaData: folder " << contMaster->folderName());
	  ATH_MSG_ERROR("processInputFileMetaData: MinRange for meta data folders ");
	  unsigned int iPayload = 0;
	  itColl    = payloadMaster->begin();
	  for (; itColl != itCollEnd; ++itColl, ++iPayload) {
	    ATH_MSG_ERROR(iPayload << " " << (*itColl)->minRange() << " " << (*itColl)->size());
	  }
	}
      }

      // detailed printout after merge
      if (msgLvl(MSG::VERBOSE)) {
	const IOVPayloadContainer*  payloadMaster = contMaster->payloadContainer();
	ATH_MSG_VERBOSE("processInputFileMetaData: After merge, payload minRange ");
	if (payloadMaster) {
	  // Loop over AttrColls and print out minRange
	  IOVPayloadContainer::const_iterator itColl    = payloadMaster->begin();
	  IOVPayloadContainer::const_iterator itCollEnd = payloadMaster->end();
	  unsigned int iPayload = 0;
	  for (; itColl != itCollEnd; ++itColl, ++iPayload) {
	    ATH_MSG_VERBOSE(iPayload << " " << (*itColl)->minRange() << " " 
			    << (*itColl)->size());
	  }
	}
	else { ATH_MSG_ERROR("  no payloads yet!"); }
      }
      
      // Detailed printout
      if (msgLvl(MSG::DEBUG)) {
	ATH_MSG_DEBUG("processInputFileMetaData: Input payload " << cont->folderName());
	std::ostringstream streamInp;
	itColl    = payload->begin();
	itCollEnd = payload->end();
	for (; itColl != itCollEnd; ++itColl) (*itColl)->dump(streamInp);
	ATH_MSG_DEBUG(streamInp.str());
	ATH_MSG_DEBUG("processInputFileMetaData: Output payload " << contMaster->folderName());
	std::ostringstream streamOut;
	itColl    = contMaster->payloadContainer()->begin();
	itCollEnd = contMaster->payloadContainer()->end();
	for (; itColl != itCollEnd; ++itColl) (*itColl)->dump(streamOut);
	ATH_MSG_DEBUG(streamOut.str());
      }
    }
  }

  ATH_MSG_DEBUG("processInputFileMetaData: Total number of attribute collections  merged together " << ncolls
		<< " Number of duplicate collections " << ndupColls);

  // Also populate ConditionStore for parameter folders when reading from file metadata
  // without intermediate sqlite files (direct in-file metadata mode).
  // Only do this if IOVDbSvc hasn't registered the folder (checked by CondCont existence).
  // Note: We're already holding m_mutex lock, so we populate ConditionStore directly
  // without calling addPayload() to avoid deadlock.
  //
  // This can only be done when we have a valid EventContext (i.e., during event processing),
  // not during file opening when this method is typically called.
  const EventContext& currentCtx = Gaudi::Hive::currentContext();
  if (currentCtx.valid()) {
    constexpr std::array folderNames{"/Digitization/Parameters", "/Simulation/Parameters"};
    for (std::string_view folderName : folderNames) {
      // Skip if folder doesn't exist in MetaDataStore (e.g., old files without these folders)
      if (!m_metaDataStore->contains<IOVMetaDataContainer>(std::string(folderName))) {
        continue;
      }

      // Skip if folder is in Payloads (write-only mode for overlay)
      // When Payloads contains entries for a folder, we're explicitly providing the data
      // and don't need to read from input file
      bool inPayloads = false;
      for (const auto& [key, value] : m_payloads) {
        if (key.find(std::string(folderName) + ":") == 0) {
          inPayloads = true;
          break;
        }
      }
      if (inPayloads) {
        ATH_MSG_DEBUG("Folder " << folderName << " is in Payloads, skipping auto-read from input");
        continue;
      }

      // Check MetaDataStore (merged view of metadata from all input files)
      IOVMetaDataContainer* contMaster = nullptr;
      if (m_metaDataStore->retrieve(contMaster, std::string(folderName)).isSuccess() && contMaster) {
        const IOVPayloadContainer* payloadMaster = contMaster->payloadContainer();
        if (payloadMaster && payloadMaster->size() > 0) {
          // Ensure ConditionStore is available
          if (!m_condStore.isValid()) {
            ATH_CHECK(m_condStore.retrieve());
          }

          // Check if CondCont already exists - if so, IOVDbSvc is managing it
          if (m_condStore->contains<CondCont<AthenaAttributeList>>(std::string(folderName))) {
            ATH_MSG_DEBUG("CondCont for " << folderName << " already exists, skipping");
            continue;
          }

          // Get the first payload (should only be one for parameter folders)
          const CondAttrListCollection* coll = dynamic_cast<const CondAttrListCollection*>(*(payloadMaster->begin()));
          if (coll) {
            // Create new CondCont using CondContFactory
            CondCont<AthenaAttributeList>* cc = nullptr;
            ServiceHandle<Athena::IRCUSvc> rcuSvc("Athena::RCUSvc", name());
            ATH_CHECK(rcuSvc.retrieve());

            SG::DataObjectSharedPtr<DataObject> cb =
              CondContainer::CondContFactory::Instance().Create(*rcuSvc,
                                                                 ClassID_traits<AthenaAttributeList>::ID(),
                                                                 std::string(folderName));
            if (!cb) {
              ATH_MSG_ERROR("Failed to create CondCont for " << folderName);
              return StatusCode::FAILURE;
            }

            if (m_condStore->recordObject(cb, std::string(folderName), true, false) == nullptr) {
              ATH_MSG_ERROR("Failed to record CondCont for " << folderName);
              return StatusCode::FAILURE;
            }

            // Retrieve the CondCont
            ATH_CHECK(m_condStore->retrieve(cc, std::string(folderName)));

            // Extract AthenaAttributeList from CondAttrListCollection and insert into CondCont
            auto itr = coll->begin();
            const coral::AttributeList& attrList = itr->second;
            auto athAttrList = std::make_unique<AthenaAttributeList>(attrList);

            // Create EventIDRange from the collection's IOV
            IOVRange iovRange = coll->minRange();
            EventIDBase start, stop;
            start.set_run_number(iovRange.start().run());
            start.set_lumi_block(iovRange.start().event());
            stop.set_run_number(iovRange.stop().run());
            stop.set_lumi_block(iovRange.stop().event());
            EventIDRange range(start, stop);

            // Insert into CondCont (we already checked that currentCtx is valid)
            ATH_CHECK(cc->insert(range, std::move(athAttrList), currentCtx));

            ATH_MSG_DEBUG("Populated ConditionStore for " << folderName << " from file metadata");
          }
        }
      }
    }
  }

  // Serialize requested IOV folders to ByteStream metadata
  ATH_CHECK(serializeIOVMetadataToBSMetadata());

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------

std::string
IOVDbMetaDataTool::serializeContainerToJSON(const IOVMetaDataContainer* container) const
{
  const IOVPayloadContainer* payloads = container->payloadContainer();
  if (!payloads || payloads->size() == 0) {
    ATH_MSG_WARNING("No payloads for folder " << container->folderName());
    return "";
  }

  using json = nlohmann::json;
  json jsonData;

  jsonData["folder"] = container->folderName();
  jsonData["description"] = container->folderDescription();
  jsonData["iovs"] = json::array();

  // Serialize each IOV payload
  for (const CondAttrListCollection* coll : *payloads) {
    json iov;

    // Get IOV range
    IOVRange range = coll->minRange();
    IOVTime start = range.start();
    IOVTime stop = range.stop();

    // IOV range
    if (start.isRunEvent()) {
      iov["range"]["start"] = {{"run", start.run()}, {"event", start.event()}};
      iov["range"]["stop"] = {{"run", stop.run()}, {"event", stop.event()}};
    } else {
      iov["range"]["start"] = {{"timestamp", start.timestamp()}};
      iov["range"]["stop"] = {{"timestamp", stop.timestamp()}};
    }

    // Attributes (serialize each channel)
    iov["attrs"] = json::object();
    for (const auto& chanAttrPair : *coll) {
      CondAttrListCollection::ChanNum chan = chanAttrPair.first;
      const coral::AttributeList& attrList = chanAttrPair.second;

      std::string chanKey = "chan" + std::to_string(chan);
      iov["attrs"][chanKey] = json::object();

      for (const auto& attr : attrList) {
        auto & thisAttribute = iov["attrs"][chanKey][attr.specification().name()];
        // Serialize attribute value based on type
        const std::type_info& type = attr.specification().type();
        if (type == typeid(std::string)) {
          thisAttribute = attr.data<std::string>();
        } else if (type == typeid(int)) {
          thisAttribute = attr.data<int>();
        } else if (type == typeid(unsigned int)) {
          thisAttribute = attr.data<unsigned int>();
        } else if (type == typeid(long)) {
          thisAttribute = attr.data<long>();
        } else if (type == typeid(unsigned long)) {
          thisAttribute = attr.data<unsigned long>();
        } else if (type == typeid(long long)) {
          thisAttribute = attr.data<long long>();
        } else if (type == typeid(unsigned long long)) {
          thisAttribute = attr.data<unsigned long long>();
        } else if (type == typeid(float)) {
          thisAttribute = attr.data<float>();
        } else if (type == typeid(double)) {
          thisAttribute = attr.data<double>();
        } else if (type == typeid(bool)) {
          thisAttribute = attr.data<bool>();
        } else {
          // For other types, convert to string representation
          std::ostringstream oss;
          attr.toOutputStream(oss);
          thisAttribute = oss.str();
          ATH_MSG_DEBUG("Attribute " << attr.specification().name() << " has unsupported type, converted to string: " << oss.str());
        }
      }
    }

    jsonData["iovs"].push_back(iov);
  }

  return jsonData.dump();
}

//--------------------------------------------------------------------------

StatusCode
IOVDbMetaDataTool::overrideIOV (CondAttrListCollection*& coll) const
{
    ATH_MSG_DEBUG("overrideIOV ");

    // Override the IOV for run/event IOVs

    //   (ONLY TRUE FOR OVERRIDE COMING IN VIA EVENTSELECTOR:)
    //   NOTE: we require that the old run number falls within the
    //   IOVRange of the incoming collection. We override ALL IOVs for
    //   ALL channels forcing the IOVRange to be (newRunNumber,1) to
    //   (newRunNumber+1,1)

    bool iovSizeIsZero = coll->iov_size() == 0;
    IOVRange testIOV = coll->minRange();
    IOVTime  start   = testIOV.start();
    IOVTime  stop    = testIOV.stop();
    IOVTime  oldRun(m_oldRunNumber, 0);
    if (start.isRunEvent() && stop.isRunEvent()) { // only for run/event
        IOVRange newRange;
        // Two ways of resetting 
        if (m_overrideMinMaxRunNumber) newRange = IOVRange(IOVTime(m_minRunNumber, 0), IOVTime(m_maxRunNumber, IOVTime::MAXEVENT));
        else if (m_overrideRunNumber)  newRange = IOVRange(IOVTime(m_newRunNumber, 0), IOVTime(m_newRunNumber + 1, 0));

        if (m_overrideRunNumber && !testIOV.isInRange(oldRun)) { 
            // old run must be in the range
            ATH_MSG_ERROR("overrideIOV: old run number does not match. Old run number " << m_oldRunNumber << " IOVRange: " << testIOV);
            return StatusCode::SUCCESS;
        }

        ATH_MSG_DEBUG("overrideIOV: overrideMinMaxRunNumber: " << (int)m_overrideMinMaxRunNumber
                      << " overrideRunNumber " << (int)m_overrideRunNumber
                      << " iovSizeIsZero: " << (int)iovSizeIsZero
                      << " newRange " << newRange);
        
        // Now over ride IOVs - two cases: 1) single IOV for full collection, 2) IOVs for individual channels.
        // Must treat the reset of collection IOV differently
        if (iovSizeIsZero) {
            // Only add in overall range if channels do not have
            // IOVs - otherwise this is automatically calculated
            coll->resetMinRange(); // must first reset to 'full range' and then reduce the IOVRange accordingly
            coll->addNewStart(newRange.start());
            coll->addNewStop (newRange.stop());
        }
        else {
            // Add in channels
            unsigned int nchans = coll->size();
            ATH_MSG_DEBUG("overrideIOV: nchans " << nchans);
            for (unsigned int ichan = 0; ichan < nchans; ++ichan) {
                // FIXME: O(N^2)!
                CondAttrListCollection::ChanNum chan = coll->chanNum(ichan);
                coll->add(chan, newRange);
                ATH_MSG_DEBUG("overrideIOV: overriding the IOV of collection chan " << chan);
            }
            // must reset the collection range AFTER the channels, because the collection range will be
            // 'narrowed' to that of the channels
            coll->resetMinRange();
        }
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("overrideIOV: after  overriding the IOV of collection");
            std::ostringstream stream;
            coll->dump(stream);
            ATH_MSG_DEBUG(stream.str());
        }
    }
    else ATH_MSG_DEBUG("overrideIOV: IOV is not run/event ");

    return StatusCode::SUCCESS;
}


//--------------------------------------------------------------------------
