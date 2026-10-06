/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// IOVDbCrestTag.cxx - CREST-backed conditions source

#include "IOVDbCrestTag.h"

#include <chai/EnumConverters.h>
#include <chai/Errors.h>
#include <chai/VectorContainer.h>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "TStopwatch.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/AthenaAttrListAddress.h"
#include "CoralUtilities/ChaiCoralConverter.h"
#include "AthenaPoolUtilities/CondAttrListCollAddress.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/CondAttrListVec.h"
#include "AthenaPoolUtilities/CondAttrListVecAddress.h"

#include "IOVDbMetaDataTools/IIOVDbMetaDataTool.h"

#include "FolderAddressResolver.h"
#include "IOVDbConn.h"
#include "IOVDbCoolFunctions.h"
#include "IOVDbJsonStringFunctions.h"
#include "IOVDbStringFunctions.h"
#include "ReadFromFileMetaData.h"

#include <sstream>

using namespace IOVDbNamespace;

IOVDbCrestTag::IOVDbCrestTag(IOVDbConn* conn, const IOVDbParser& folderprop, MsgStream& msg,
                             IClassIDSvc* clidsvc, IIOVDbMetaDataTool* metadatatool,
                             chai::Database& db, const std::string& crestTagName):
  AthMessaging("IOVDbCrestTag"),
  p_clidSvc(clidsvc),
  p_metaDataTool(metadatatool),
  m_conn(conn),
  m_db(&db),
  m_resolvedTagName(crestTagName)
{
  setLevel(msg.level());
  m_foldername = folderprop.folderName();
  m_key = folderprop.key();
  m_jokey = folderprop.hasKey();
  m_eventstore = folderprop.eventStoreName();
  m_notagoverride = folderprop.noTagOverride();
  ATH_MSG_DEBUG("Created CREST folder " << m_foldername << " tag " << m_resolvedTagName);
  if (m_notagoverride) {
    ATH_MSG_INFO("Inputfile tag override disabled for " << m_foldername);
  }

  // <tag> is not used to resolve the CREST tag but is still recorded into
  // m_jotag for fillTagInfo(). <ctag> is the fallback when there is no <tag>.
  if (!folderprop.tag().empty()) {
    ATH_MSG_INFO("<tag> is ignored on the CREST path for folder " << m_foldername
                 << ". Use <ctag> to override the CREST tag name instead");
    m_jotag = IOVDbNamespace::spaceStrip(folderprop.tag());
  } else {
    std::string ctag;
    if (folderprop.getKey("ctag","",ctag) && !ctag.empty()) {
      m_jotag = IOVDbNamespace::spaceStrip(ctag);
    }
  }

  // No CHAI equivalent of COOL's server-side channelSelection. Parsed here
  // and applied client-side by channelInSelection().
  std::string chanspec;
  if (folderprop.getKey("channelSelection","",chanspec) && !chanspec.empty()) {
    m_chanrange = IOVDbNamespace::parseChannelSpec<cool::ChannelId>(chanspec);
  }

  if (folderprop.overridesIov(msg)) {
    m_iovoverridden = true;
    m_iovoverride = folderprop.iovOverrideValue(msg);
  }

  m_fromMetaDataOnly = folderprop.onlyReadMetadata();
  if (m_fromMetaDataOnly) {
    ATH_MSG_INFO("Read from meta data only for folder " << m_foldername);
  }

  m_extensible = folderprop.extensible();
  if (m_extensible) {
    ATH_MSG_INFO("Extensible folder " << m_foldername);
  }
}

IOVDbCrestTag::~IOVDbCrestTag() {
  if (m_cachespec != nullptr) {
    m_cachespec->release();
  }
}

void IOVDbCrestTag::useFileMetaData() {
  m_useFileMetaData = true;
  if (m_conn != nullptr) {
    m_conn->decUsage();
    m_conn = nullptr;
  }
}

void IOVDbCrestTag::setTagOverride(const std::string& tag, const bool setFlag) {
  // Never reached for CREST tags (IOVDbSvc::processTagInfo skips them).
  // CREST tags come from the GlobalTag mapping or <ctag>, not TagInfo.
  if (setFlag) {
    m_tagoverride = true;
  }
  m_jotag = IOVDbNamespace::spaceStrip(tag);
}

void IOVDbCrestTag::setWriteMeta() {
  m_writemeta = true;
}

void IOVDbCrestTag::setIOVOverride(const unsigned int run, const unsigned int lumiblock, const unsigned int time) {
  if (m_iovoverridden) {
    return;
  }
  if (m_timestamp) {
    if (time != 0) {
      m_iovoverride = IOVDbNamespace::iovTimeFromSeconds(time);
      ATH_MSG_INFO("Override timestamp to " << m_iovoverride << " for folder " << m_foldername);
      m_iovoverridden = true;
    }
  } else {
    if (run != 0 || lumiblock != 0) {
      m_iovoverride = IOVDbNamespace::iovTimeFromRunLumi(run, lumiblock);
      ATH_MSG_INFO("Override run/LB number to [" << run << ":" << lumiblock << "] for folder " << m_foldername);
      m_iovoverridden = true;
    }
  }
}

cool::ValidityKey IOVDbCrestTag::iovTime(const IOVTime& reftime) const {
  if (m_iovoverridden) {
    return m_iovoverride;
  }
  return (m_timestamp ? reftime.timestamp() : reftime.re_time());
}

void IOVDbCrestTag::reset() {
  m_resident.reset();
}

IOVDbNamespace::FolderType IOVDbCrestTag::determineFolderType() const {
  // Check for CoraCool (not supported in CREST)
  if (m_folderDescription.find("<coracool>") != std::string::npos) {
    ATH_MSG_FATAL("Folder " << m_foldername << " is CoraCool and is not representable with CREST");
    return IOVDbNamespace::UNKNOWN;
  }

  const std::string typeName = IOVDbNamespace::parseTypename(m_folderDescription);

  // If the type is CondAttrListVec, and is not CoraCool, it must be a CoolVector
  if (typeName == "CondAttrListVec") {
    return IOVDbNamespace::CoolVector;
  }

  // A string type "PoolRef" column in field 0 indicates a POOL-compatible tag
  const auto& fields = m_tag->getFieldSpec();
  const bool poolCompatible = ((fields.size() > 0) && (fields[0].name == "PoolRef") &&
                               (fields[0].type == chai::String));
  if (poolCompatible) {
    const auto& channels = m_tag->getChannelSpec().channels();
    const bool onlyChannelZero = (channels.size() == 1 && channels[0].id == 0);
    return onlyChannelZero ? IOVDbNamespace::PoolRef : IOVDbNamespace::PoolRefColl;
  }

  // CondAttrListCollection -> AttrListColl
  if (typeName == "CondAttrListCollection") {
    return IOVDbNamespace::AttrListColl;
  }

  // Otherwise  AttrList
  return IOVDbNamespace::AttrList;
}

std::unique_ptr<SG::TransientAddress>
IOVDbCrestTag::createTransientAddress(const std::vector<std::string>& symlinks) {
  auto tad = std::make_unique<SG::TransientAddress>(m_clid, m_key);
  for (const auto& linkname : symlinks) {
    if (not linkname.empty()) {
      CLID sclid = 0;
      if (StatusCode::SUCCESS == p_clidSvc->getIDOfTypeName(linkname, sclid)) {
        tad->setTransientID(sclid);
        ATH_MSG_DEBUG("Setup symlink " << linkname << " CLID " << sclid << " for folder " << m_foldername);
      } else {
        ATH_MSG_ERROR("Could not get clid for symlink: " << linkname);
        return nullptr;
      }
    }
  }
  return tad;
}

std::unique_ptr<SG::TransientAddress>
IOVDbCrestTag::preload(ITagInfoMgr* /*tagInfoMgr*/, const unsigned int /*cacheRun*/, const unsigned int /*cacheTime*/) {
  if (not m_useFileMetaData) {
    if (m_resolvedTagName.empty()) {
      ATH_MSG_FATAL("Folder " << m_foldername << " has no CREST tag and is not read from file metadata");
      return nullptr;
    }

    // Attempt to resolve the tag and set the folder description
    try {
      m_tag = m_db->getTag(m_resolvedTagName);
    } catch (const std::exception& e) {
      ATH_MSG_FATAL("Failed to resolve CHAI tag '" << m_resolvedTagName
                    << "' for folder " << m_foldername << ": " << e.what());
      return nullptr;
    }
    m_folderDescription = m_tag->getNodeDescription();

    // A description without <timeStamp> takes the timebase from the tag's IovType.
    // An existing element is used as is.
    if (chai::extractNodeDescTimeStampToken(m_folderDescription).empty()) {
      const std::string token = chai::iovTypeToNodeDescTimeStampToken(m_tag->getIovType());
      ATH_MSG_INFO("Folder " << m_foldername << "'s CREST description for tag "
                   << m_tag->getName() << " has no <timeStamp> element; inserting '" << token
                   << "' from the tag's IovType");
      m_folderDescription = "<timeStamp>" + token + "</timeStamp>" + m_folderDescription;
    }
  }

  // Otherwise the folder description was already set from file metadata
  ATH_MSG_DEBUG("Folder description for " << m_foldername << ": " << m_folderDescription);

  if (m_writemeta) {
    if (StatusCode::SUCCESS != p_metaDataTool->registerFolder(m_foldername, m_folderDescription)) {
      ATH_MSG_ERROR("Failed to register folder " << m_foldername << " for meta-data write");
      return nullptr;
    }
  }

  // Read timestamp-indexing straight from the description string so this
  // works even when no chai::Tag is resolved (a metadata-only folder).
  IOVDbParser folderpar(m_folderDescription, msg());
  m_timestamp = folderpar.timebaseIs_nsOfEpoch();

  // Reported here rather than in the constructor because the timebase
  // decides how the override key reads
  if (m_iovoverridden) {
    if (m_timestamp) {
      ATH_MSG_INFO("Override timestamp to " << m_iovoverride << " for folder " << m_foldername);
    } else {
      const auto [run,lumi] = IOVDbNamespace::runLumiFromIovTime(m_iovoverride);
      ATH_MSG_INFO("Override run/LB number to [" << run << ":" << lumi << "] for folder " << m_foldername);
    }
  }

  IOVDbNamespace::FolderAddressSpec resolved;
  if (!IOVDbNamespace::resolveFolderAddress(msg(), folderpar, m_foldername, m_jokey, m_key, p_clidSvc, resolved)) {
    return nullptr;
  }

  m_key = std::move(resolved.key);
  m_named = resolved.named;
  m_addrheader = std::move(resolved.addrheader);
  m_typename = std::move(resolved.typeName);
  m_clid = resolved.clid;

  if (not m_useFileMetaData) {
    // Get all channel IDs and names from the tag's channel spec
    const auto& channels = m_tag->getChannelSpec().channels();
    m_channums.clear();
    m_channames.clear();
    for (const auto& ch : channels) {
      m_channums.push_back(static_cast<cool::ChannelId>(ch.id));
      m_channames.push_back(ch.name);
    }

    m_foldertype = determineFolderType();
    if (m_foldertype == IOVDbNamespace::UNKNOWN) {
      return nullptr;
    }
  }

  // For folders read from metadata, folder-type identification is
  // done in getAddress(). Channel number/name info is never read.
  m_nchan = m_channums.size();
  ATH_MSG_DEBUG("Folder identified as type " << m_foldertype);

  // Get the transient address while handling any symlinks
  const auto& linknameVector = folderpar.symLinks();
  auto tad{createTransientAddress(linknameVector)};
  if (not tad) {
    ATH_MSG_WARNING("Transient address is null in " << __func__);
    return nullptr;
  }
  return tad;
}

bool IOVDbCrestTag::loadAt(const cool::ValidityKey requestedVkey) {
  TStopwatch cachetimer;
  // A bad EventInfo timestamp can produce vkey==0xFFFF... which a floor
  // lookup would resolve to the last IOV. Reset to 0 with a warning.
  cool::ValidityKey vkey = requestedVkey;
  if (vkey > cool::ValidityKeyMax) {
    ATH_MSG_WARNING("Requested validity key " << vkey << " is out of range, reset to 0");
    vkey = 0;
  }
  ATH_MSG_DEBUG("Load cache for folder " << m_foldername << " validitykey " << vkey);

  m_resident.reset();

  // An IOVTime above cool::ValidityKeyMax is invalid so clamp at the max
  const auto clampedKey = [this](uint64_t key, const char* which) {
    if (key > cool::ValidityKeyMax) {
      ATH_MSG_WARNING("CREST " << which << " " << key << " for folder " << m_foldername
                      << " exceeds cool::ValidityKeyMax; clamped");
      return cool::ValidityKeyMax;
    }
    return static_cast<cool::ValidityKey>(key);
  };

  unsigned int nchan = 0;
  try {
    // Caches the payload and returns the channel count
    auto storeResident = [this, &clampedKey](auto&& payloadResult) {
      if (m_cachespec == nullptr) {
        m_cachespec = ChaiCoralConverter::toCoralSpec(payloadResult.payload.fieldSpec());
      }
      const unsigned int channelCount = payloadResult.payload.channelIds().size();
      m_nbytesread += ChaiCoralConverter::payloadSize(payloadResult.payload);
      const cool::ValidityKey sinceKey = clampedKey(payloadResult.since, "since");
      const cool::ValidityKey untilKey = payloadResult.until.has_value()
        ? clampedKey(*payloadResult.until, "until") : cool::ValidityKeyMax;
      m_resident = Resident{sinceKey, untilKey, std::move(payloadResult)};
      return channelCount;
    };

    // CHAI needs to know if it's a vector or scalar payload to call the correct getXPayloadAt method.
    // This is what actually loads the payload at the desired time and caches it.
    nchan = (m_foldertype == IOVDbNamespace::CoolVector) ? storeResident(m_tag->getVectorPayloadAt(vkey))
                                                         : storeResident(m_tag->getPayloadAt(vkey));
  } catch (const chai::NotFoundError& e) {
    // IOVDbFolder returns an empty collection when a collection-type folder has
    // zero matching objects and when the query time precedes the first IOV.
    // Any other NotFoundError is a real error.
    const bool isCollectionType = (m_foldertype == AttrListColl || m_foldertype == PoolRefColl || m_foldertype == CoolVector);

    if (isCollectionType) {
      try {
        // getIovs() is lazy, so begin() fetches only the first page and returns the first element
        auto iovs = m_tag->getIovs(0);
        auto firstIt = iovs.begin();
        if (firstIt != iovs.end() && firstIt->getSince() > vkey) {
          const cool::ValidityKey firstSince = clampedKey(firstIt->getSince(), "first since");
          ATH_MSG_WARNING("No covering IOV for folder " << m_foldername << " validityKey " << vkey
                          << " (tag " << m_resolvedTagName << "'s first IOV starts at "
                          << firstSince << "); treating the folder as empty");
          m_resident = Resident{cool::ValidityKeyMin, firstSince, std::monostate{}};
          ++m_ndbread;
          const float timeinc = cachetimer.RealTime();
          m_readtime += timeinc;
          ATH_MSG_DEBUG("Cache retrieve done for " << m_foldername
                        << " with 0 channels (no coverage) " << "in " << std::fixed << std::setw(8)
                        << std::setprecision(2) << timeinc << " s");
          return true;
        }
      } catch (const std::exception& probeErr) {
        // A failure here is not the empty-collection case. Fall through to the real error below.
        ATH_MSG_DEBUG("Coverage probe for " << m_foldername << " failed: " << probeErr.what());
      }
    }
    ATH_MSG_ERROR("No covering IOV for folder " << m_foldername << " validityKey " << vkey << ": " << e.what());
    reset();
    return false;
  } catch (const chai::Error& e) {
    ATH_MSG_ERROR("CHAI error for folder " << m_foldername << ": " << e.what());
    reset();
    return false;
  } catch (const std::exception& e) {
    ATH_MSG_ERROR("Failed for folder " << m_foldername << ": " << e.what());
    reset();
    return false;
  }

  ++m_ndbread;
  m_nobjread += nchan;
  const float timeinc = cachetimer.RealTime();
  m_readtime += timeinc;
  ATH_MSG_DEBUG("Cache retrieve done for " << m_foldername << " with " << nchan
                << " channels stored in " << std::fixed << std::setw(8) << std::setprecision(2)
                << timeinc << " s");
  return true;
}

bool IOVDbCrestTag::loadCacheIfDbChanged(const cool::ValidityKey /*vkey*/,
                                         const cool::IDatabasePtr& /*dbPtr*/,
                                         const ServiceHandle<IIOVSvc>& /*iovSvc*/) {
  // COOL-specific online-reload path. No-op for CREST.
  return true;
}

bool IOVDbCrestTag::addMetaAttrList(const coral::AttributeList& atrlist, const IOVRange& range) {
  CondAttrListCollection tmpColl(!m_timestamp);
  tmpColl.add(0xFFFF, atrlist);
  tmpColl.add(0xFFFF, range);
  return addMetaAttrListColl(&tmpColl);
}

bool IOVDbCrestTag::addMetaAttrListColl(const CondAttrListCollection* coll) {
  if (!coll) {
    return false;
  }
  CondAttrListCollection* flmdColl = new CondAttrListCollection(*coll);
  if (StatusCode::SUCCESS != p_metaDataTool->addPayload(m_foldername, flmdColl)) {
    ATH_MSG_ERROR("addMetaAttrList: Failed to write metadata for folder " << m_foldername);
    return false;
  }
  ATH_MSG_DEBUG("addMetaAttrList: write metadata for folder " << m_foldername);
  return true;
}

bool IOVDbCrestTag::channelInSelection(cool::ChannelId chan) const {
  if (m_chanrange.empty()) {
    return true;
  }
  for (const auto& range : m_chanrange) {
    if (chan >= range.first && chan <= range.second) {
      return true;
    }
  }
  return false;
}

bool IOVDbCrestTag::getAddress(const cool::ValidityKey reftime, IAddressCreator* persSvc,
                               const unsigned int poolSvcContext,
                               std::unique_ptr<IOpaqueAddress>& address,
                               IOVRange& range, bool& poolPayloadReq) {
  ++m_ncacheread;
  std::string strAddress;
  // Owned here until released into the address object below, to avoid leaking in case of failure
  std::unique_ptr<AthenaAttributeList> attrList;
  std::unique_ptr<CondAttrListCollection> attrListColl;
  std::unique_ptr<CondAttrListVec> attrListVec;

  // Read from metadata if flag is set
  if (m_useFileMetaData) {
    IOVDbNamespace::SafeReadFromFileMetaData readFromMetaData(m_foldername, p_metaDataTool, reftime, m_timestamp);
    if (not readFromMetaData.isValid()) {
      ATH_MSG_ERROR("read:Could not find IOVPayloadContainer for folder " << m_foldername);
      return false;
    }
    m_foldertype = readFromMetaData.folderType();
    m_nobjread += readFromMetaData.numberOfObjects();
    poolPayloadReq = readFromMetaData.poolPayloadRequested();
    strAddress = readFromMetaData.stringAddress();
    range = readFromMetaData.range();
    attrList.reset(readFromMetaData.attributeList());
    attrListColl.reset(readFromMetaData.attrListCollection());
    ATH_MSG_DEBUG("Read file metadata for folder " << m_foldername << " foldertype is " << m_foldertype);
  }

  // Otherwise convert the resident CHAI payload directly
  else {
    if (!m_resident) {
      ATH_MSG_ERROR("No resident payload for folder " << m_foldername << " at IOV " << reftime);
      return false;
    }
    range = IOVDbNamespace::makeRange(m_resident->since, m_resident->until, m_timestamp);

    unsigned int nchan = 0;
    std::optional<coral::AttributeList> singleAl;
    try {
      switch (m_foldertype) {
        case AttrList:
        case PoolRef: {
          // A single-channel folder with no payload is an error
          if (std::holds_alternative<std::monostate>(m_resident->payload)) {
            ATH_MSG_ERROR("Empty resident for single-channel folder " << m_foldername);
            return false;
          }
          const auto& payloadResult = std::get<chai::PayloadResult<chai::Container>>(m_resident->payload);
          // Only channel 0 is read, as on the COOL path. Any other populated channel is ignored
          if (!payloadResult.payload.hasChannel(0)) {
            ATH_MSG_ERROR("No channel 0 for single-channel folder " << m_foldername << " currentTime "
                          << reftime << " (" << payloadResult.payload.channelIds().size()
                          << " channels populated)");
            return false;
          }
          nchan = 1;
          // Convert CHAI payload to attribute list
          singleAl.emplace(ChaiCoralConverter::toAttributeList(*m_cachespec, payloadResult.payload[0]));
          if (m_foldertype == AttrList) {
            attrList = std::make_unique<AthenaAttributeList>(*singleAl);
            strAddress = "POOLContainer_AthenaAttributeList][CLID=x";
          } else {
            strAddress = (*singleAl)["PoolRef"].data<std::string>();
          }
          break;
        }
        case AttrListColl:
        case PoolRefColl: {
          attrListColl = std::make_unique<CondAttrListCollection>(!m_timestamp);
          // Empty resident means zero channels. Just leave it empty, matching IOVDbFolder.
          if (!std::holds_alternative<std::monostate>(m_resident->payload)) {
            const auto& payloadResult = std::get<chai::PayloadResult<chai::Container>>(m_resident->payload);
            const auto channelIds = payloadResult.payload.channelIds();
            for (const auto channelId : channelIds) {
              const auto chan = static_cast<cool::ChannelId>(channelId);
              if (!channelInSelection(chan)) {
                continue;
              }
              // Convert each row to an attribute list and add to collection
              attrListColl->addShared(chan, ChaiCoralConverter::toAttributeList(*m_cachespec, payloadResult.payload[channelId]));
              attrListColl->add(chan, range);
              ++nchan;
            }
          }
          if (m_named) {
            auto nitr = m_channames.begin();
            for (auto chitr = m_channums.begin(); chitr != m_channums.end(); ++chitr, ++nitr) {
              attrListColl->add(*chitr, *nitr);
            }
          }
          strAddress = "POOLContainer_CondAttrListCollection][CLID=x";
          break;
        }
        case CoolVector: {
          attrListVec = std::make_unique<CondAttrListVec>(!m_timestamp);
          // Empty resident means zero channels. Just leave it empty, matching IOVDbFolder.
          if (!std::holds_alternative<std::monostate>(m_resident->payload)) {
            const auto& payloadResult = std::get<chai::PayloadResult<chai::VectorContainer>>(m_resident->payload);
            const auto channelIds = payloadResult.payload.channelIds();
            for (const auto channelId : channelIds) {
              const auto chan = static_cast<cool::ChannelId>(channelId);
              if (!channelInSelection(chan)) {
                continue;
              }
              // Convert each set of rows (per channel ID) to an attribute list vec
              const auto rows = ChaiCoralConverter::toAttributeListVec(*m_cachespec, payloadResult.payload.rows(channelId));
              attrListVec->addSlice(range, chan, rows, 0, rows.size());
              ++nchan;
            }
          }
          strAddress = "POOLContainer_CondAttrListVec][CLID=x";
          break;
        }
        default:
          ATH_MSG_ERROR("Unhandled folder type " << m_foldertype << " for folder " << m_foldername);
          return false;
      }
    } catch (const std::exception& e) {
      ATH_MSG_ERROR("Conversion failed for folder " << m_foldername << ": " << e.what());
      return false;
    }

    if (m_writemeta && (m_foldertype == AttrList || m_foldertype == PoolRef)) {
      if (!addMetaAttrList(*singleAl, range)) {
        return false;
      }
    }

    // CoolVector folders cannot be written to file metadata
    if (m_writemeta && m_foldertype == CoolVector) {
      ATH_MSG_ERROR("Writing of CoolVector folders to file metadata not implemented");
      return false;
    }

    // Don't change wording here, getProblemFoldersFromLogs.py relies on this.
    ATH_MSG_DEBUG("Retrieved object: folder " << m_foldername << " at IOV " << reftime
                  << " channels " << nchan << " has range " << range);
  }

  m_currange = range;
  m_retrieved = true;

  if (m_writemeta && (m_foldertype == AttrListColl || m_foldertype == PoolRefColl)) {
    if (!addMetaAttrListColl(attrListColl.get())) {
      return false;
    }
  }

  strAddress = m_addrheader + strAddress;
  IOpaqueAddress* addrp = nullptr;
  if (StatusCode::SUCCESS != persSvc->createAddress(0, 0, strAddress, addrp)) {
    ATH_MSG_ERROR("Could not get IOpaqueAddress from string address " << strAddress);
    return false;
  }
  address = std::unique_ptr<IOpaqueAddress>(addrp);
  GenericAddress* gAddr = dynamic_cast<GenericAddress*>(address.get());
  if (!gAddr) {
    ATH_MSG_ERROR("Could not cast IOpaqueAddress to GenericAddress");
    return false;
  }
  if (m_foldertype == AttrListColl) {
    auto addr = std::make_unique<CondAttrListCollAddress>(*gAddr);
    addr->setAttrListColl(attrListColl.release());
    address = std::move(addr);
  } else if (m_foldertype == PoolRefColl || m_foldertype == PoolRef) {
    auto addr = std::make_unique<CondAttrListCollAddress>(gAddr->svcType(),
                                                          gAddr->clID(), gAddr->par()[0],
                                                          gAddr->par()[1], poolSvcContext,
                                                          gAddr->ipar()[1]);
    if (m_foldertype == PoolRefColl) {
      addr->setAttrListColl(attrListColl.release());
    }
    address = std::move(addr);
    poolPayloadReq = true;
  } else if (m_foldertype == AttrList) {
    auto addr = std::make_unique<AthenaAttrListAddress>(*gAddr);
    addr->setAttrList(attrList.release());
    address = std::move(addr);
  } else if (m_foldertype == CoolVector) {
    auto addr = std::make_unique<CondAttrListVecAddress>(*gAddr);
    addr->setAttrListVec(attrListVec.release());
    address = std::move(addr);
  }
  return true;
}

void IOVDbCrestTag::summary() {
  ATH_MSG_INFO("Folder " << m_foldername << " (" << IOVDbNamespace::folderTypeName(m_foldertype)
              << ") db-read " << m_ndbread << "/" << m_ncacheread << " objs/chan/bytes "
              << m_nobjread << "/" << m_nchan << "/" << m_nbytesread << " (( " << std::fixed
              << std::setw(8) << std::setprecision(2) << m_readtime << " ))s");
  if (m_ncacheread == 0 && m_ndbread > 0) {
    ATH_MSG_WARNING("Folder " << m_foldername << " is requested but no data retrieved");
  }
}

void IOVDbCrestTag::printState() {
  ATH_MSG_DEBUG("folder cache printout -------------------");
  if (m_resident) {
    unsigned int nchan = 0;
    if (!std::holds_alternative<std::monostate>(m_resident->payload)) {
      nchan = (m_foldertype == CoolVector)
        ? std::get<chai::PayloadResult<chai::VectorContainer>>(m_resident->payload).payload.channelIds().size()
        : std::get<chai::PayloadResult<chai::Container>>(m_resident->payload).payload.channelIds().size();
    }
    ATH_MSG_DEBUG(m_foldername << "\tsince: " << m_resident->since << "\tuntil: "
                  << m_resident->until << "\tchannels: " << nchan);
  } else {
    ATH_MSG_DEBUG(m_foldername << "\tno resident payload");
  }
  ATH_MSG_DEBUG("current range: " << m_currange);
  ATH_MSG_DEBUG("folder cache printout -------------------");
}

std::string IOVDbCrestTag::dumpChannelsAsJson(cool::ValidityKey reftime) const {
  std::ostringstream os;
  os << "[";
  std::string sep;
  if (isResident(reftime) && !std::holds_alternative<std::monostate>(m_resident->payload)) {
    const cool::ValidityKey since = m_resident->since;
    const cool::ValidityKey until = m_resident->until;
    if (m_foldertype == CoolVector) {
      const auto& payloadResult = std::get<chai::PayloadResult<chai::VectorContainer>>(m_resident->payload);
      for (const auto channelId : payloadResult.payload.channelIds()) {
        const auto& rows = payloadResult.payload.rows(channelId);
        if (rows.empty()) {
          continue;
        }
        std::ostringstream payload;
        payload << "[";
        std::string rowSep;
        for (const auto& row : rows) {
          const coral::AttributeList al = ChaiCoralConverter::toAttributeList(*m_cachespec, *row);
          payload << rowSep << IOVDbNamespace::jsonAttributeList(al);
          rowSep = IOVDbNamespace::s_delimiterJson;
        }
        payload << "]";
        os << sep << IOVDbNamespace::s_openJson << "\"" << channelId << "\" : "
           << IOVDbNamespace::s_openJson
           << "\"since\" : " << since << IOVDbNamespace::s_delimiterJson
           << "\"until\" : " << until << IOVDbNamespace::s_delimiterJson
           << "\"payload\" : " << payload.str()
           << IOVDbNamespace::s_closeJson << IOVDbNamespace::s_closeJson;
        sep = IOVDbNamespace::s_delimiterJson;
      }
    } else {
      const auto& payloadResult = std::get<chai::PayloadResult<chai::Container>>(m_resident->payload);
      for (const auto channelId : payloadResult.payload.channelIds()) {
        const coral::AttributeList al =
          ChaiCoralConverter::toAttributeList(*m_cachespec, payloadResult.payload[channelId]);
        os << sep << IOVDbNamespace::s_openJson << "\"" << channelId << "\" : "
           << IOVDbNamespace::s_openJson
           << "\"since\" : " << since << IOVDbNamespace::s_delimiterJson
           << "\"until\" : " << until << IOVDbNamespace::s_delimiterJson
           << "\"payload\" : [" << IOVDbNamespace::jsonAttributeList(al) << "]"
           << IOVDbNamespace::s_closeJson << IOVDbNamespace::s_closeJson;
        sep = IOVDbNamespace::s_delimiterJson;
      }
    }
  }
  os << "]";
  return os.str();
}
