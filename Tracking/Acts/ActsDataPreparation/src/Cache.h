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

#include <set>
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "IRegionSelector/IRegSelTool.h"
#include "AthContainers/DataVector.h"

namespace ActsTrk::Cache{
    template<typename OT>
    class CacheEntry: public DataObject{
        public:
        CacheEntry(){}
        CacheEntry(DataVector<OT>* dv, unsigned int s, unsigned int e): container(dv), range_start(s), range_end(e){}
        DataVector<OT>* container;
        unsigned int range_start;
        unsigned int range_end;
    };

    template<typename OT>
    class Handles{
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

    template<typename CT>
    class ViewFillerAlg: public AthReentrantAlgorithm{
        public:
        using ObjectType = typename CT::base_value_type;
        using CacheReadHandleKey = typename Handles<ObjectType>::ReadHandleKey;
        using CacheReadHandle = typename Handles<ObjectType>::ReadHandle;

        ViewFillerAlg(const std::string& name, ISvcLocator* pSvcLocator): AthReentrantAlgorithm(name, pSvcLocator) {}
        virtual ~ViewFillerAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute (const EventContext& ctx) const override;

        //read handle for the IDC we want to fill
        CacheReadHandleKey m_inputIDC{this, "InputIDC","", "The input IDC container"};
        //write handle for the plain old xaod container we want to create
        SG::WriteHandleKey<ConstDataVector<CT>> m_outputKey{this,"Output","", "The key of the output container"};

        //handle for the rois we need to fill for
        SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey {this, "RoIs", "", "RoIs to read in"};
        ToolHandle<IRegSelTool> m_regionSelector{this,"RegSelTool","","Region selector tool"};
    };

    template<typename OT>
    class Helper{
    public:
        using IDCWriteHandle = typename IdentifiableContainer<CacheEntry<OT>>::IDC_WriteHandle;

        static StatusCode insert(IDCWriteHandle& wh, DataVector<OT>* dv, unsigned int range_start, unsigned int range_end);
    };
}

#include "Cache.icc"
#endif