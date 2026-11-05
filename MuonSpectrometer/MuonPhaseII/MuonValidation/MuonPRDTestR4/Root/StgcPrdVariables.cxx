/*
  Copyright (C) 2002-2026 CERN
  for the benefit of the ATLAS collaboration
*/
#include "MuonPRDTestR4/StgcPrdVariables.h"
#include "StoreGate/ReadHandle.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

namespace MuonValR4 {

void StgcPrdVariables::enableSeededDump() {
    m_applyFilter = true;
}

void StgcPrdVariables::dumpAllHitsInChamber( const Identifier& chamberId) {
    m_applyFilter = true;
    m_filteredChamb.insert( idHelperSvc()->chamberId(chamberId));
}

unsigned int StgcPrdVariables::push_back( const xAOD::sTgcStripCluster& prd) {
    m_applyFilter = true;
    const auto [it, inserted] = m_stripIdx.emplace( prd.identify(), m_stripIdx.size());

    if (inserted) {
        m_seedStrips.push_back(&prd);
    }

    return it->second;
}

unsigned int StgcPrdVariables::push_back( const xAOD::sTgcWireHit& prd) {
    m_applyFilter = true;
    const auto [it, inserted] = m_wireIdx.emplace( prd.identify(), m_wireIdx.size());

    if (inserted) {
        m_seedWires.push_back(&prd);
    }

    return it->second;
}

unsigned int StgcPrdVariables::push_back( const xAOD::sTgcPadHit& prd) {
    m_applyFilter = true;
    const auto [it, inserted] = m_padIdx.emplace( prd.identify(), m_padIdx.size());

    if (inserted) {
        m_seedPads.push_back(&prd);
    }

    return it->second;
}

void StgcPrdVariables::clearEventSelection() {
    m_filteredChamb.clear();
    m_stripIdx.clear();
    m_wireIdx.clear();
    m_padIdx.clear();
    m_seedStrips.clear();
    m_seedWires.clear();
    m_seedPads.clear();
    // Intentionally do NOT reset m_applyFilter (same as MM).
}

StgcPrdVariables::StgcPrdVariables(MuonVal::MuonTesterTree& tree,
                                   const std::string& stripKey,
                                   const std::string& wireKey,
                                   const std::string& padKey,
                                   bool writeStripNumbers,
                                   bool writeStripCharges,
                                   bool writeStripTimes,
                                   MSG::Level msgLvl)
    : TesterModuleBase{tree, "StgcPrds", msgLvl},
      m_stripKey{stripKey},
      m_wireKey{wireKey},
      m_padKey{padKey}{
    // Book optional branches only if requested, so no empty jagged
    // vectors or baskets are written when a detail switch is false.
    if (writeStripNumbers) {
        m_stripNumbers = &parent().newMatrix<uint16_t>("StgcStrip_stripNumbers");
    }
    if (writeStripCharges) {
        m_stripCharges = &parent().newMatrix<int>("StgcStrip_stripCharges");
    }
    if (writeStripTimes) {
        m_stripTimes = &parent().newMatrix<short>("StgcStrip_stripTimes");
    }
}


bool StgcPrdVariables::declare_keys() {
    return declare_dependency(m_stripKey) &&
           declare_dependency(m_wireKey) &&
           declare_dependency(m_padKey);
}

bool StgcPrdVariables::fill(const EventContext& ctx) {
    const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

    SG::ReadHandle<xAOD::sTgcStripContainer> strips{m_stripKey, ctx};
    SG::ReadHandle<xAOD::sTgcWireContainer> wires{m_wireKey, ctx};
    SG::ReadHandle<xAOD::sTgcPadContainer> pads{m_padKey, ctx};

    if (!strips.isValid() || !wires.isValid() || !pads.isValid()) {
        ATH_MSG_ERROR("Missing sTGC PRD container(s): "
                      << m_stripKey.fullKey() << " (valid=" << strips.isValid() << "), "
                      << m_wireKey.fullKey() << " (valid=" << wires.isValid() << "), "
                      << m_padKey.fullKey() << " (valid=" << pads.isValid() << ")");
        clearEventSelection();
        return false;
    }

    // The explicit seeds are written first. push_back() indices therefore
    // correspond to their positions in the resulting per-type output vectors.
    for (const auto* prd : m_seedStrips) dumpStrip(gctx, *prd);
    for (const auto* prd : m_seedWires) dumpWire(gctx, *prd);
    for (const auto* prd : m_seedPads) dumpPad(gctx, *prd);

    // Then append the other hits, subject to the optional chamber filter.
    // The maps already contain seeded IDs; insert new IDs as they are dumped.
    for (const auto* prd : *strips) {
        const Identifier id{prd->identify()};
        if (m_applyFilter && !m_filteredChamb.count(idHelperSvc()->chamberId(id))) continue;
        if (!m_stripIdx.emplace(id, m_stripIdx.size()).second) continue;
        dumpStrip(gctx, *prd);
    }
    for (const auto* prd : *wires) {
        const Identifier id{prd->identify()};
        if (m_applyFilter && !m_filteredChamb.count(idHelperSvc()->chamberId(id))) continue;
        if (!m_wireIdx.emplace(id, m_wireIdx.size()).second) continue;
        dumpWire(gctx, *prd);
    }
    for (const auto* prd : *pads) {
        const Identifier id{prd->identify()};
        if (m_applyFilter && !m_filteredChamb.count(idHelperSvc()->chamberId(id))) continue;
        if (!m_padIdx.emplace(id, m_padIdx.size()).second) continue;
        dumpPad(gctx, *prd);
    }

    clearEventSelection();
    return true;
}

void StgcPrdVariables::dumpStrip(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcStripCluster& prd) {
    const auto* strip = &prd;
    const Identifier id{strip->identify()};
    const auto* re = strip->readoutElement();
    const Amg::Vector3D global{ re->localToGlobalTransform(gctx, strip->layerHash()) * strip->localMeasurementPos()};

    m_stripId.push_back(id);
    m_stripGlobal.push_back(global);
    m_stripLocal.push_back(strip->localPosition<1>()[0]);
    m_stripCov.push_back(strip->localCovariance<1>()(0, 0));
    m_stripTime.push_back(strip->time());
    m_stripCharge.push_back(strip->charge());
    m_stripAuthor.push_back(static_cast<uint8_t>(strip->author()));
    //m_stripQuality.push_back(static_cast<uint8_t>(strip->quality()));

    if (m_stripNumbers) m_stripNumbers->push_back(strip->stripNumbers());
    if (m_stripCharges) m_stripCharges->push_back(strip->stripCharges());
    if (m_stripTimes) m_stripTimes->push_back(strip->stripTimes());
}

void StgcPrdVariables::dumpWire(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcWireHit& prd) {
    const auto* wire = &prd;
    const Identifier id{wire->identify()};
    const auto* re = wire->readoutElement();
    const Amg::Vector3D global{ re->localToGlobalTransform(gctx, wire->layerHash()) * wire->localMeasurementPos()};

    m_wireId.push_back(id);
    m_wireGlobal.push_back(global);
    // The scalar localPosition<1>() is along local Y for wires.
    m_wireLocal.push_back(wire->localPosition<1>()[0]);
    m_wireCov.push_back(wire->localCovariance<1>()(0, 0));
    m_wireTime.push_back(wire->time());
    m_wireCharge.push_back(wire->charge());
    m_wireAuthor.push_back(static_cast<uint8_t>(wire->author()));
}

void StgcPrdVariables::dumpPad(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcPadHit& prd) {
    const auto* pad = &prd;
    const Identifier id{pad->identify()};
    const auto* re = pad->readoutElement();
    const Amg::Vector3D global{ re->localToGlobalTransform(gctx, pad->layerHash()) * pad->localMeasurementPos()};

    const auto pos = pad->localPosition<2>();
    const auto cov = pad->localCovariance<2>();

    m_padId.push_back(id);
    m_padGlobal.push_back(global);
    m_padLocalX.push_back(pos[0]);
    m_padLocalY.push_back(pos[1]);
    m_padCovXX.push_back(cov(0, 0));
    m_padCovXY.push_back(cov(0, 1));
    m_padCovYY.push_back(cov(1, 1));
    m_padTime.push_back(pad->time());
    m_padCharge.push_back(pad->charge());
    m_padAuthor.push_back(static_cast<uint8_t>(pad->author()));
}


} // namespace MuonValR4
