/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IInDetConditionsTool.h
 * @author shaun.roe@cern.ch
**/
#ifndef SIMULATIONBASE
#ifndef IInDetConditionsTool_h
#define IInDetConditionsTool_h
//Gaudi Includes
#include "GaudiKernel/IInterface.h"

//Local includes
#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"

//forward declarations
class Identifier;
class IdentifierHash;
/**
 * @class IInDetConditionsTool
 * Interface class for service providing summary of status of a detector element
**/
class IInDetConditionsTool: virtual public IInterface, virtual public IAlgTool{
public:
  virtual ~IInDetConditionsTool() = default;
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(IInDetConditionsTool, 1, 0);
 
  struct IDCCacheEntry {
    EventContext::ContextEvt_t eventId = EventContext::INVALID_CONTEXT_EVT; // invalid event ID for the start
    const IDCInDetBSErrContainer_Cache* IDCCache = nullptr;

    void reset( EventContext::ContextEvt_t evtId, const IDCInDetBSErrContainer_Cache* cache) {
      eventId = evtId;
      IDCCache   = cache;
    }

    bool needsUpdate( const EventContext& ctx) const {
      return eventId != ctx.evt() or eventId == EventContext::INVALID_CONTEXT_EVT;
    }

  };

  virtual bool isActive(const Identifier& elementId, const InDetConditions::Hierarchy h, const EventContext& ctx) const =0;
  virtual bool isActive(const IdentifierHash& elementHash, const EventContext& ctx) const =0;
  virtual bool isActive(const IdentifierHash& elementHash, const Identifier& elementId, const EventContext& ctx) const =0;
  virtual double activeFraction(const IdentifierHash& elementHash, const Identifier& idStart, const Identifier& idEnd, const EventContext& ctx) const =0;  

  virtual bool isGood(const Identifier& elementId, const InDetConditions::Hierarchy h, const EventContext& ctx) const =0;
  virtual bool isGood(const IdentifierHash& elementHash, const EventContext& ctx) const =0;
  virtual bool isGood(const IdentifierHash& elementHash, const Identifier& elementId, const EventContext& ctx, const IDCCacheEntry* cacheEntry=nullptr) const =0;
  virtual double goodFraction(const IdentifierHash& elementHash, const Identifier& idStart, const Identifier& idEnd, const EventContext& ctx) const =0;

  virtual bool hasBSError(const IdentifierHash& elementHash, const EventContext& ctx, const IDCCacheEntry* cacheEntry=nullptr) const =0;
  virtual bool hasBSError(const IdentifierHash& elementHash, Identifier elementId, const EventContext& ctx, const IDCCacheEntry* cacheEntry=nullptr) const =0;
  virtual uint64_t getBSErrorWord(const IdentifierHash& elementHash, const EventContext& ctx, const IDCCacheEntry* cacheEntry=nullptr) const =0;
  virtual uint64_t getBSErrorWord(const IdentifierHash& elementHash, const int index, const EventContext& ctx, const IDCCacheEntry* cacheEntry=nullptr) const =0;

  virtual IDCCacheEntry* getCacheEntryOut(const EventContext& ctx) const = 0;
};

#endif
#endif
