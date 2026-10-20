/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MeasTruthAssocMapProvider.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

namespace MuonR4{
    StatusCode MeasTruthAssocMapProvider::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_linkKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_truthKey.initialize());
        ATH_CHECK(m_segLinkKey.initialize());
        ATH_CHECK(m_segLinkKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode MeasTruthAssocMapProvider::execute(const EventContext& ctx) const {
        const xAOD::MuonMeasurementContainer* measurements{};
        ATH_CHECK(SG::get(measurements, m_readKey, ctx));
        auto assocMap = std::make_unique<ActsTrk::MeasurementToTruthParticleAssociation>(measurements->size());
        assocMap->setSourceContainer(*measurements, ctx);
        for (const xAOD::MuonMeasurement* meas : *measurements) {
            const xAOD::MuonSimHit* assocHit = getTruthMatchedHit(*meas);
            if (!assocHit) {
                continue;
            }
            const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*assocHit);
            if (!truthPart) {
                continue;
            }
            (*assocMap)[meas->index()]= {xAOD::TruthHelpers::getTruthParticle(*truthPart)};
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(assocMap)));
        return StatusCode::SUCCESS;
    }
}