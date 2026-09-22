/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// IOVDbConditionsSource.h
// Backend-neutral abstract interface through which IOVDbSvc drives one
// conditions folder, implemented by IOVDbFolder (COOL) and IOVDbCrestTag (CREST/CHAI)

#ifndef IOVDBSVC_IOVDBCONDITIONSSOURCE_H
#define IOVDBSVC_IOVDBCONDITIONSSOURCE_H

#include <memory>
#include <string>

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IIOVSvc.h"
#include "AthenaKernel/IOVRange.h"
#include "AthenaKernel/IOVTime.h"

#include "CoolKernel/IDatabase.h"
#include "CoolKernel/ValidityKey.h"
#include "SGTools/TransientAddress.h"

#include "FolderTypes.h"

class IOVDbConn;
class IOpaqueAddress;
class IAddressCreator;
class ITagInfoMgr;

class IOVDbConditionsSource {
public:
  // Backend a folder reads its conditions from.
  enum class source_t {
    COOLDB=0,
    CRESTDB,
  };

  virtual ~IOVDbConditionsSource() = default;
  // Fill in object details from cache
  // Set poolPayloadRequested flag if a POOL file was referenced
  virtual bool getAddress(const cool::ValidityKey reftime,IAddressCreator* persSvc,
                  const unsigned int poolSvcContext,
                  std::unique_ptr<IOpaqueAddress>& address,
                  IOVRange& range,bool& poolPayloadRequested) = 0;

  // Preload address to Storegate (does folder initialization from the backend)
  virtual std::unique_ptr<SG::TransientAddress>
  preload(ITagInfoMgr *tagInfoMgr,
                const unsigned int cacheRun,
                const unsigned int cacheTime) = 0;
  // Print out cache
  virtual void printState() = 0;
  // Reset cache to empty
  virtual void reset() = 0;
  // Make summary of usage
  virtual void summary() =0;

  // Access methods to various internal information
  const std::string& folderName() const {return m_foldername;};
  virtual const std::string& key() const =0;
  virtual IOVDbConn* conn() = 0;
  virtual bool multiVersion() const = 0;
  virtual bool timeStamp() const = 0;
  virtual bool tagOverride() const = 0;
  virtual bool retrieved() const = 0;
  virtual bool noOverride() const = 0;
  virtual IOVDbNamespace::FolderType folderType() const = 0;
  virtual bool readMeta() const = 0;
  virtual bool writeMeta() const = 0;
  // Read from meta data only, otherwise ignore folder
  virtual bool fromMetaDataOnly() const = 0;
  // If true, then the end time for an open-ended range will be set to just past
  // the current event. The end time will be automatically updated on
  // accesses in subsequent events.
  virtual bool extensible() const = 0;
  virtual bool dropped() const = 0;
  virtual bool iovOverridden() const = 0;
  // Tag from job options, if any. Used for TagInfo output
  virtual const std::string& joTag() const = 0;
  virtual const std::string& resolvedTag() const = 0;
  virtual const std::string& eventStore() const = 0;
  virtual const source_t& source() const = 0;
  virtual CLID clid() const = 0;
  virtual unsigned long long bytesRead() const = 0;
  virtual float readTime() const = 0;
  virtual const IOVRange& currentRange() const = 0;
  // Mark this folder as using metadata from an input file
  virtual void useFileMetaData() = 0;
  // Set folder description
  virtual void setFolderDescription(const std::string& description) = 0;
  // Set tag override, set override flag as well if setFlag is true
  // Override flag prevents reading of FLMD for this folder if present
  virtual void setTagOverride(const std::string& tag,const bool setFlag) = 0;
  // Set writeMeta flag
  virtual void setWriteMeta() = 0;
  // Set IOV overrides
  virtual void setIOVOverride(const unsigned int run,const unsigned int lumiblock,
                      const unsigned int time) = 0;
  // Mark object as dropped from Storegate
  virtual void setDropped(const bool dropped) = 0;

  // Get validityKey for folder, given current time, accounting for overrides
  virtual cool::ValidityKey iovTime(const IOVTime& reftime) const = 0;

  // Check cache is valid (resident) for current time
  virtual bool isResident(const cool::ValidityKey reftime) const = 0;
  // Load cache for given validitykey; job-level tuning constants
  // (cache alignment, global tag, online mode) are fixed at construction
  virtual bool loadAt(const cool::ValidityKey vkey) = 0;
  // Reload cache in online mode if ValidityKey returns a new object
  // with start > previously used start. COOL-only. The CREST implementation
  // is a no-op that returns true.
  virtual bool loadCacheIfDbChanged(const cool::ValidityKey vkey,
                            const cool::IDatabasePtr& dbPtr,
                            const ServiceHandle<IIOVSvc>& iovSvc) = 0;


protected:

  std::string m_foldername;       ///< Folder name, as it appears in COOL or as a CREST tag label
  std::string m_key;              ///< SG key where data is loaded (unique)

};
#endif
