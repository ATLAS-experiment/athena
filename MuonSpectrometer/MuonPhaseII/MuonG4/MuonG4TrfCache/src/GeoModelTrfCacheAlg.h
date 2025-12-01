/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonG4TrfCache_GeoModelTrfCacheAlg_h
#define MuonG4TrfCache_GeoModelTrfCacheAlg_h

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/WriteCondHandleKey.h"

namespace MuonG4{
    class GeoModelTrfCacheAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            SG::WriteCondHandleKey<ActsTrk::DetectorAlignStore>  m_writeKey{this, "writeKey", ""};
            /// Flag determining the subdetector. Needs to be static castable to DetectorType
            Gaudi::Property<int> m_detType{this, "DetectorType", static_cast<int>(ActsTrk::DetectorType::UnDefined)};
            /// 
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            ActsTrk::DetectorType m_type{ActsTrk::DetectorType::UnDefined};

    };
}
#endif
