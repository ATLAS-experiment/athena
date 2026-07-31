/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceDetectorDescriptionValidationAlg.h"
#include "StoreGate/StoreGateSvc.h"

#include <cmath>
#include <unordered_map>

namespace ActsTrk {

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

StatusCode DeviceDetectorDescriptionValidationAlg::finalize()
{

     ATH_MSG_DEBUG("Finalizing detector description validation alg");
    ATH_MSG_INFO("Checked " << m_nChecked << " shared surfaces ("
                 << m_nMissingInCandidate << " missing in candidate), "
                 << m_nIdMismatch << " ACTS Id mismatches, "
                 << m_nDesignMismatch << " design mismatches, "
                 << m_nShiftMismatch << " shift mismatches");

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


    for (std::size_t refIdx = 0; refIdx < refCond.size(); ++refIdx) {
        const auto geomIdValue = refCond.geometry_id()[refIdx].value();
        auto it = candByGeomId.find(geomIdValue);
        if (it == candByGeomId.end()) {
            ++m_nMissingInCandidate;
            ATH_MSG_DEBUG("geometry_id " << geomIdValue
                          << " present in reference, missing in candidate");
            continue;
        }
        const std::size_t candIdx = it->second;
        ++m_nChecked;

        // --- ACTS geometry ID ---
        const auto& refID = refCond.acts_geometry_id()[refIdx];
        const auto& candID = candCond.acts_geometry_id()[candIdx];
        bool actsIdOk = (refID == candID);
        if (!actsIdOk) {
            ++m_nIdMismatch;
            ATH_MSG_DEBUG("ACTS ID mismatch at geometry_id " << geomIdValue
                         << ": ref= " << refID
                         << " cand= " << candID);
        }

        // --- Lorentz shift ---
        const auto& refShift = refCond.measurement_translation()[refIdx];
        const auto& candShift = candCond.measurement_translation()[candIdx];
        const bool shiftOk =
            std::abs(static_cast<float>(refShift[0]) -
                     static_cast<float>(candShift[0])) <= m_shiftTolerance.value() &&
            std::abs(static_cast<float>(refShift[1]) -
                     static_cast<float>(candShift[1])) <= m_shiftTolerance.value();
        if (!shiftOk) {
            ++m_nShiftMismatch;
            ATH_MSG_DEBUG("Lorentz shift mismatch at geometry_id " << geomIdValue
                         << ": ref=(" << refShift[0] << "," << refShift[1]
                         << ") cand=(" << candShift[0] << "," << candShift[1] << ")");
        }

        // --- Design content, retrieved through the conditions table's
        //     module_to_design_id(), compared by shape rather than raw id value ---

        const unsigned int refDesignId = refCond.module_to_design_id()[refIdx];
        const unsigned int candDesignId = candCond.module_to_design_id()[candIdx];

        const auto& refDesignEntry = refDesign.at(refDesignId);
        const auto& candDesignEntry = candDesign.at(candDesignId);

        bool designOk = true;

        if(refDesignEntry.dimensions() != candDesignEntry.dimensions()){

            designOk = false;
            
            ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue
                            << ": ref design dimensions=" << refDesignEntry.dimensions()
                            << " cand design dimensions=" << candDesignEntry.dimensions());
        }

        if(refDesignEntry.subspace() != candDesignEntry.subspace()){

            designOk = false;
            
            ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue
                            << ": ref design subspace=" << refDesignEntry.subspace()
                            << " cand design subspace=" << candDesignEntry.subspace());
        }

        if(refDesignEntry.bin_edges_x().size() != candDesignEntry.bin_edges_x().size()){

            designOk = false;
            
            ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue
                            << ": ref design edges x size=" << refDesignEntry.bin_edges_x().size()
                            << " cand design edges x size=" << candDesignEntry.bin_edges_x().size());
        }else{

            if(refDesignEntry.bin_edges_x() != candDesignEntry.bin_edges_x()){

                designOk = false;

                ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue);

                for(size_t b = 0; b < refDesignEntry.bin_edges_x().size(); b++){
                    if(refDesignEntry.bin_edges_x().at(b) != candDesignEntry.bin_edges_x().at(b)){
                        ATH_MSG_DEBUG("At entry " << b << " ref edge: " << refDesignEntry.bin_edges_x().at(b) << " vs cand edge " << candDesignEntry.bin_edges_x().at(b));
                    }
                    
                }
            }

        }

        if(refDesignEntry.bin_edges_y().size() != candDesignEntry.bin_edges_y().size()){

            designOk = false;
            
            ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue
                            << ": ref design edges y size=" << refDesignEntry.bin_edges_y().size()
                            << " cand design edges y size=" << candDesignEntry.bin_edges_y().size());
        }else{

            if(refDesignEntry.bin_edges_y() != candDesignEntry.bin_edges_y()){

                designOk = false;

                ATH_MSG_DEBUG("Design mismatch at geometry_id " << geomIdValue);

                for(size_t b = 0; b < refDesignEntry.bin_edges_y().size(); b++){
                    if(refDesignEntry.bin_edges_y().at(b) != candDesignEntry.bin_edges_y().at(b)){
                        ATH_MSG_DEBUG("At entry " << b << " ref edge: " << refDesignEntry.bin_edges_y().at(b) << " vs cand edge " << candDesignEntry.bin_edges_y().at(b));
                    }
                    
                }
            }

        }


        if(!designOk) ++m_nDesignMismatch;
        
        
    }

    return StatusCode::SUCCESS;
}

}  // namespace ActsTrk