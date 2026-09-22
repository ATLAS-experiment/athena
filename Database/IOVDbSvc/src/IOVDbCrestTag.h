/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// IOVDbCrestTag.h
// CREST-backed implementation of IOVDbConditionsSource

#ifndef IOVDBSVC_IOVDBCRESTTAG_H
#define IOVDBSVC_IOVDBCRESTTAG_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>

#include <chai/Database.h>
#include <chai/Tag.h>
#include <chai/VectorContainer.h>

#include "GaudiKernel/IClassIDSvc.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/IOVTime.h"
#include "AthenaKernel/IOVRange.h"
#include "AthenaKernel/IIOVSvc.h"

#include "CoolKernel/ValidityKey.h"
#include "SGTools/TransientAddress.h"
#include "IOVDbParser.h"

#include "IOVDbConditionsSource.h"
#include "FolderTypes.h"

class MsgStream;
class IOVDbConn;
class IOpaqueAddress;
class IAddressCreator;
class IIOVDbMetaDataTool;
class CondAttrListCollection;
class ITagInfoMgr;

namespace coral {
  class AttributeListSpecification;
  class AttributeList;
}

// CHAI/CREST-backed conditions source. Resolves one CHAI tag by name against
// the shared chai::Database owned at the IOVDbSvc/connection level. Not
// thread-safe: every call runs under Athena::DBLock or from IOVDbSvc's own
// single-threaded init paths (see the m_crestDatabases comment in IOVDbSvc.h).
class IOVDbCrestTag: public AthMessaging, public IOVDbConditionsSource {
public:
  // Construct from the shared chai::Database and the resolved CREST tag
  // name. The tag itself is resolved in preload(), so an empty name (a
  // metadata-only folder such as /TagInfo) is allowed here.
  IOVDbCrestTag(IOVDbConn* conn, const IOVDbParser& folderprop, MsgStream& msg,
                IClassIDSvc* clidsvc, IIOVDbMetaDataTool* metadatatool,
                chai::Database& db, const std::string& crestTagName);
  ~IOVDbCrestTag();

  // Access methods to various internal information
  const std::string& key() const override;
  const source_t& source() const override;

  IOVDbConn* conn() override;
  bool multiVersion() const override;
  bool timeStamp() const override;
  bool tagOverride() const override;
  bool retrieved() const override;
  bool noOverride() const override;
  IOVDbNamespace::FolderType folderType() const override;
  bool readMeta() const override;
  bool writeMeta() const override;
  bool fromMetaDataOnly() const override;
  bool extensible() const override;
  bool dropped() const override;
  bool iovOverridden() const override;
  const std::string& joTag() const override;
  const std::string& resolvedTag() const override;
  const std::string& eventStore() const override;
  CLID clid() const override;
  unsigned long long bytesRead() const override;
  float readTime() const override;
  const IOVRange& currentRange() const override;

  // Set methods, used after folder creation to set properties externally
  void useFileMetaData() override;
  void setFolderDescription(const std::string& description) override;
  void setTagOverride(const std::string& tag,const bool setFlag) override;
  void setWriteMeta() override;
  void setIOVOverride(const unsigned int run,const unsigned int lumiblock, const unsigned int time) override;
  void setDropped(const bool dropped) override;

  // Get validityKey for folder, given current time (accounting for overrides)
  cool::ValidityKey iovTime(const IOVTime& reftime) const override;

  // Check cache is valid (resident) for current time
  bool isResident(const cool::ValidityKey reftime) const override;

  // Load cache for given validitykey: fetches the one IOV covering the key,
  // with no COOL-like windowed prefetch or caching. See Resident for how
  // it is held.
  bool loadAt(const cool::ValidityKey vkey) override;

  // Reset cache to empty
  void reset() override;

  // Fill in object details from cache
  bool getAddress(const cool::ValidityKey reftime,IAddressCreator* persSvc,
                  const unsigned int poolSvcContext,
                  std::unique_ptr<IOpaqueAddress>& address,
                  IOVRange& range,bool& poolPayloadRequested) override;

  // Make summary of usage
  void summary() override;

  // Preload address to Storegate (does folder initialization from CHAI)
  std::unique_ptr<SG::TransientAddress>
  preload(ITagInfoMgr *tagInfoMgr, const unsigned int cacheRun, const unsigned int cacheTime) override;

  // Dump the resident payload's channels covering reftime as a JSON array,
  // empty if reftime is not resident. Uses the same serializer as
  // IOVDbFolder::dumpChannelsAsJson, so cool_crest_compare sees identical
  // output from both backends. Debug/tooling only.
  std::string dumpChannelsAsJson(cool::ValidityKey reftime) const;

  // Print out cache
  void printState() override;

  // No-op on the CREST path. Always returns true.
  bool loadCacheIfDbChanged(const cool::ValidityKey vkey,
                            const cool::IDatabasePtr& dbPtr,
                            const ServiceHandle<IIOVSvc>& iovSvc) override;

private:
  // Resolve the CHAI tag's folder type from its node description + payload spec.
  IOVDbNamespace::FolderType determineFolderType() const;

  // Create transient address, processing symlinks if given
  std::unique_ptr<SG::TransientAddress>
  createTransientAddress(const std::vector<std::string> & symlinks);

  // Call metadata writing tool for given list and range (same as IOVDbFolder's)
  bool addMetaAttrList(const coral::AttributeList& atrlist, const IOVRange& range);
  bool addMetaAttrListColl(const CondAttrListCollection* coll);

  // True if channel falls in an (empty means unrestricted) <channelSelection>
  // range list. CHAI's getPayloadAt()/getVectorPayloadAt() return every
  // channel with no server-side filter, so getAddress() filters client-side.
  bool channelInSelection(cool::ChannelId chan) const;

  // The one CHAI payload resident for this tag: whatever loadAt() last
  // fetched, held raw until getAddress() converts it.
  struct Resident {
    cool::ValidityKey since{0};  ///< Start of the covered range
    cool::ValidityKey until{0};  ///< End of the covered range, or ValidityKeyMax if open-ended
    // monostate means no payload: the query time precedes the tag's first IOV
    // on a collection-type folder. since/until still bound the gap so a later
    // call reloads once coverage starts.
    std::variant<std::monostate,
                 chai::PayloadResult<chai::Container>,
                 chai::PayloadResult<chai::VectorContainer>> payload;
  };

  IClassIDSvc*         p_clidSvc{nullptr};
  IIOVDbMetaDataTool*  p_metaDataTool{nullptr};
  IOVDbConn*           m_conn{nullptr};           ///< Unused for CREST I/O; kept for per-connection usage counting
  chai::Database*      m_db{nullptr};             ///< Non-owning pointer to the shared chai::Database
  chai::TagPtr         m_tag;                     ///< Resolved in preload(). Stays null for a metadata-only folder

  std::string m_folderDescription;
  bool m_timestamp{false};                        ///< Indexed by timestamp (else runLB), from tag->getIovType()

  bool m_tagoverride{false};    ///< Unused on the CREST path. Present to satisfy IOVDbConditionsSource
  bool m_notagoverride{false};  ///< Unused on the CREST path. Present to satisfy IOVDbConditionsSource

  bool m_writemeta{false};
  bool m_useFileMetaData{false};
  bool m_fromMetaDataOnly{false};
  bool m_extensible{false};
  bool m_named{false};
  bool m_iovoverridden{false};
  bool m_jokey{false};
  bool m_dropped{false};
  cool::ValidityKey m_iovoverride{0};
  IOVDbNamespace::FolderType m_foldertype{IOVDbNamespace::UNKNOWN};

  std::string m_jotag;              ///< Explicit <tag>/<ctag> from job options. Used only for /TagInfo
  std::string m_resolvedTagName;    ///< The CREST tag name actually used for lookup
  std::string m_eventstore;
  std::string m_addrheader;
  std::string m_typename;
  CLID m_clid{0};

  unsigned int m_ncacheread{0};
  unsigned int m_ndbread{0};
  unsigned int m_nobjread{0};
  unsigned long long m_nbytesread{0};
  float m_readtime{0};

  unsigned int m_nchan{0};
  std::vector<cool::ChannelId> m_channums;
  std::vector<std::string> m_channames;

  // Parsed <channelSelection> ranges. Empty means unrestricted.
  // See channelInSelection().
  std::vector<std::pair<cool::ChannelId, cool::ChannelId>> m_chanrange;

  bool m_retrieved{false};
  IOVRange m_currange;

  // Shared AttributeListSpecification, built once per tag from the first
  // resident payload's PayloadSpec and reused by every conversion after.
  coral::AttributeListSpecification* m_cachespec{nullptr};

  std::optional<Resident> m_resident;
};

inline const std::string& IOVDbCrestTag::key() const { return m_key; }
inline const IOVDbCrestTag::source_t& IOVDbCrestTag::source() const {
  static constexpr source_t s_source{source_t::CRESTDB};
  return s_source;
}

inline IOVDbConn* IOVDbCrestTag::conn() { return m_conn; }

inline bool IOVDbCrestTag::multiVersion() const { return false; }

inline bool IOVDbCrestTag::timeStamp() const { return m_timestamp; }

inline bool IOVDbCrestTag::tagOverride() const { return m_tagoverride; }

inline bool IOVDbCrestTag::noOverride() const { return m_notagoverride; }

inline bool IOVDbCrestTag::retrieved() const { return m_retrieved; }

inline IOVDbNamespace::FolderType IOVDbCrestTag::folderType() const { return m_foldertype; }

inline void IOVDbCrestTag::setFolderDescription(const std::string& description) {
  m_folderDescription = description;
}

inline bool IOVDbCrestTag::readMeta() const { return m_useFileMetaData; }

inline bool IOVDbCrestTag::writeMeta() const { return m_writemeta; }

inline bool IOVDbCrestTag::fromMetaDataOnly() const { return m_fromMetaDataOnly; }

inline bool IOVDbCrestTag::extensible() const { return m_extensible; }

inline bool IOVDbCrestTag::dropped() const { return m_dropped; }

inline bool IOVDbCrestTag::iovOverridden() const { return m_iovoverridden; }

inline const std::string& IOVDbCrestTag::joTag() const { return m_jotag; }

inline const std::string& IOVDbCrestTag::resolvedTag() const { return m_resolvedTagName; }

inline const std::string& IOVDbCrestTag::eventStore() const { return m_eventstore; }

inline CLID IOVDbCrestTag::clid() const { return m_clid; }

inline unsigned long long IOVDbCrestTag::bytesRead() const { return m_nbytesread; }

inline float IOVDbCrestTag::readTime() const { return m_readtime; }

inline const IOVRange& IOVDbCrestTag::currentRange() const { return m_currange; }

inline bool IOVDbCrestTag::isResident(const cool::ValidityKey reftime) const {
  // Unlike IOVDbFolder::isResident's strict ">", the bounds here are the IOV's
  // own [since, until), so reftime landing on since counts as resident.
  // Otherwise every such vkey forces a redundant CHAI refetch.
  return m_resident && reftime>=m_resident->since && reftime<m_resident->until;
}

inline void IOVDbCrestTag::setDropped(const bool dropped) { m_dropped=dropped; }

#endif //  IOVDBSVC_IOVDBCRESTTAG_H
