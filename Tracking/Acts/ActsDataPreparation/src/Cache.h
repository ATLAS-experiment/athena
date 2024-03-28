/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKING_ACTS_CACHE_H
#define TRACKING_ACTS_CACHE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/ConstDataVector.h"
#include "GaudiKernel/DataObject.h"

#include "EventContainers/IdentifiableContainer.h"
#include "EventContainers/IdentifiableCache.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandle.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"

#include "StoreGate/UpdateHandle.h"
#include "StoreGate/UpdateHandleKey.h"

namespace ActsTrk::Cache {

  template<typename OT>
    class CacheEntry : public DataObject {
        public:
        CacheEntry(){}
        CacheEntry(DataVector<OT>* dv, unsigned int s, unsigned int e): container(dv), range_start(s), range_end(e){}
        DataVector<OT>* container;
        unsigned int range_start;
        unsigned int range_end;
    };

    template<typename OT>
    class Handles {
        public:
        using IDCBackend = typename EventContainers::IdentifiableCache<CacheEntry<OT>>;
        using IDC = IdentifiableContainer<CacheEntry<OT>>;

        using BackendUpdateHandleKey = SG::UpdateHandleKey<IDCBackend>;
        using BackendUpdateHandle = SG::UpdateHandle<IDCBackend>;
        
        using WriteHandleKey = SG::WriteHandleKey<IDC>;
        using WriteHandle = SG::WriteHandle<IDC>;

        using ReadHandleKey = SG::ReadHandleKey<IDC>;
        using ReadHandle = SG::ReadHandle<IDC>;
    };

    template<typename OT>
    class Helper {
    public:
        using IDCWriteHandle = typename IdentifiableContainer<CacheEntry<OT>>::IDC_WriteHandle;

        static StatusCode insert(IDCWriteHandle& wh, DataVector<OT>* dv, unsigned int range_start, unsigned int range_end);
    };

} // namespace

#include "Cache.icc"

#endif
