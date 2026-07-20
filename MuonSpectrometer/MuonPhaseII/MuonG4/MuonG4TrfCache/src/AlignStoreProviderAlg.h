/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonG4TrfCache_AlignStoreProviderAlg_h
#define MuonG4TrfCache_AlignStoreProviderAlg_h

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"

namespace MuonG4{
    class AlignStoreProviderAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            SG::ReadCondHandleKey<ActsTrk::DetectorAlignStore> m_readKey{this, "readKey", ""};
            SG::WriteHandleKey<ActsTrk::DetectorAlignStore>  m_writeKey{this, "writeKey", ""};
    };
}

#endif