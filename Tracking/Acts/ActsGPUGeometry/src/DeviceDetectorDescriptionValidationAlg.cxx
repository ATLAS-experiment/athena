/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceDetectorDescriptionValidationAlg.h"
#include "StoreGate/StoreGateSvc.h"

#include <cmath>
#include <unordered_map>

namespace ActsTrk {

namespace {

// Content-based equality: same dimensionality + matching bin edges.
// design_id itself is never compared directly since it's assigned
// independently by deduplication pass.
bool designsMatch(const traccc::detector_design_description::host& a,
                   std::size_t ia,
                   const traccc::detector_design_description::host& b,
                   std::size_t ib, float positionTolerance) {
    if (a.dimensions()[ia] != b.dimensions()[ib]) return false;

    auto edgesMatch = [&](const auto& ex, const auto& ey) {
        if (ex.size() != ey.size()) return false;
        for (std::size_t k = 0; k < ex.size(); ++k) {
            if (std::abs(static_cast<float>(ex[k]) - static_cast<float>(ey[k])) >
                positionTolerance) {
                return false;
            }
        }
        return true;
    };

    return edgesMatch(a.bin_edges_x()[ia], b.bin_edges_x()[ib]) &&
           edgesMatch(a.bin_edges_y()[ia], b.bin_edges_y()[ib]);
}

std::size_t findMatchingDesign(
    const traccc::detector_design_description::host& design, std::size_t at,
    const traccc::detector_design_description::host& candidate,
    float positionTolerance) {
    for (std::size_t j = 0; j < candidate.size(); ++j) {
        if (designsMatch(design, at, candidate, j, positionTolerance)) return j;
    }
    return candidate.size();  // sentinel: no matching shape found
}

}  // namespace

StatusCode DeviceDetectorDescriptionValidationAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing detector description validation alg");

    ATH_CHECK(m_monCondKey.initialize());
    

    if (m_monDesignObjectName.value().empty() ||
        m_refHostDesignObjectName.value().empty()) {
        ATH_MSG_FATAL("HostDesignObjectName / RefHostDesignObjectName must both be set");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode DeviceDetectorDescriptionValidationAlg::execute(const EventContext& ctx) const
{
    // ---- candidate design (static, from detStore) ----
    const traccc::detector_design_description::host* monDesign = nullptr;
    ATH_CHECK(detStore()->retrieve(monDesign, m_monDesignObjectName.value()));

    // ---- reference design (static, from detStore) ----
    const traccc::detector_design_description::host* refDesign = nullptr;
    ATH_CHECK(detStore()->retrieve(refDesign, m_refHostDesignObjectName.value()));

    // ---- candidate conditions (per-IOV) ----
    SG::ReadCondHandle<traccc::detector_conditions_description::host> monCondHandle{m_monCondKey, ctx};
    ATH_CHECK(monCondHandle.isValid());
    const traccc::detector_conditions_description::host* monCond = monCondHandle.cptr();

    // ---- reference conditions (static, read from JSON and stored in detStore) ----
    const traccc::detector_conditions_description::host* refCond = nullptr;
    ATH_CHECK(detStore()->retrieve(refCond, m_refHostCondKey.value()));

    ATH_CHECK(validateDetectorDescription(*refDesign, *refCond, *monDesign, *monCond));

    return StatusCode::SUCCESS;
}

StatusCode DeviceDetectorDescriptionValidationAlg::validateDetectorDescription(
    const traccc::detector_design_description::host& refDesign,
    const traccc::detector_conditions_description::host& refCond,
    const traccc::detector_design_description::host& candDesign,
    const traccc::detector_conditions_description::host& candCond) const
{
    // candidate geometry_id (as raw value) -> row index
    std::unordered_map<std::uint64_t, std::size_t> candByGeomId;
    candByGeomId.reserve(candCond.size());
    for (std::size_t i = 0; i < candCond.size(); ++i) {
        candByGeomId[candCond.geometry_id()[i].value()] = i;
    }

    // ref design_id -> matching cand design row, cached per distinct shape.
    std::unordered_map<unsigned int, std::size_t> designMatchCache;

    std::size_t nChecked = 0;
    std::size_t nMissingInCandidate = 0;
    std::size_t nDesignMismatch = 0;
    std::size_t nShiftMismatch = 0;

    for (std::size_t refIdx = 0; refIdx < refCond.size(); ++refIdx) {
        const auto geomIdValue = refCond.geometry_id()[refIdx].value();
        auto it = candByGeomId.find(geomIdValue);
        if (it == candByGeomId.end()) {
            ++nMissingInCandidate;
            ATH_MSG_DEBUG("geometry_id " << geomIdValue
                          << " present in reference, missing in candidate");
            continue;
        }
        const std::size_t candIdx = it->second;
        ++nChecked;

        // --- Lorentz shift ---
        const auto& refShift = refCond.measurement_translation()[refIdx];
        const auto& candShift = candCond.measurement_translation()[candIdx];
        const bool shiftOk =
            std::abs(static_cast<float>(refShift[0]) -
                     static_cast<float>(candShift[0])) <= m_shiftTolerance.value() &&
            std::abs(static_cast<float>(refShift[1]) -
                     static_cast<float>(candShift[1])) <= m_shiftTolerance.value();
        if (!shiftOk) {
            ++nShiftMismatch;
            ATH_MSG_INFO("Lorentz shift mismatch at geometry_id " << geomIdValue
                         << ": ref=(" << refShift[0] << "," << refShift[1]
                         << ") cand=(" << candShift[0] << "," << candShift[1] << ")");
        }

        // --- Design content, matched through the conditions table's
        //     module_to_design_id(), by shape rather than raw id value ---
        const unsigned int refDesignId = refCond.module_to_design_id()[refIdx];
        std::size_t candMatchIdx;
        auto cacheIt = designMatchCache.find(refDesignId);
        if (cacheIt != designMatchCache.end()) {
            candMatchIdx = cacheIt->second;
        } else {
            candMatchIdx = findMatchingDesign(refDesign, refDesignId, candDesign,
                                               m_positionTolerance.value());
            designMatchCache.emplace(refDesignId, candMatchIdx);
        }

        const unsigned int candDesignId = candCond.module_to_design_id()[candIdx];
        const bool designOk =
            (candMatchIdx != candDesign.size()) &&
            (candDesignId == static_cast<unsigned int>(candMatchIdx));

        if (!designOk) {
            ++nDesignMismatch;
            ATH_MSG_VERBOSE("Design mismatch at geometry_id " << geomIdValue
                            << ": ref design_id=" << refDesignId
                            << " cand design_id=" << candDesignId
                            << (candMatchIdx == candDesign.size()
                                    ? " (no shape in candidate matches reference)"
                                    : ""));
        }
    }

    ATH_MSG_INFO("Checked " << nChecked << " shared surfaces ("
                 << nMissingInCandidate << " missing in candidate), "
                 << nDesignMismatch << " design mismatches, "
                 << nShiftMismatch << " shift mismatches");

    return StatusCode::SUCCESS;
}

}  // namespace ActsTrk