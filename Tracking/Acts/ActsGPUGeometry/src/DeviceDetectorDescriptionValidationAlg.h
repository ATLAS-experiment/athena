/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONVALIDATIONALG_H
#define ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONVALIDATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"

namespace ActsTrk {

/**
 * @class DeviceDetectorDescriptionValidationAlg
 *
 * @brief Validates that a "candidate" detector description (design +
 * conditions) matches a "reference" one.
 *
 * Design objects are static and retrieved directly from detStore.
 * Conditions objects are per-IOV and retrieved via ReadCondHandle.
 *
 * Matching strategy:
 *  - Conditions rows are matched 1:1 by detray geometry_id, since both
 *    pipelines enumerate the same detray surfaces.
 *  - Design rows have no cross-pipeline identity (design_id is assigned
 *    independently by each pipeline's own dedup pass), so they are
 *    matched by content (dimensionality + bin edges), resolved through
 *    each side's module_to_design_id() in the conditions table.
 */
class DeviceDetectorDescriptionValidationAlg : public AthReentrantAlgorithm
{
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    StatusCode validateDetectorDescription(
        const traccc::detector_design_description::host& refDesign,
        const traccc::detector_conditions_description::host& refCond,
        const traccc::detector_design_description::host& candDesign,
        const traccc::detector_conditions_description::host& candCond) const;

    // ---- candidate (production) design + conditions ----
    Gaudi::Property<std::string> m_monDesignObjectName{
        this, "MonDesignObjectName", "", "Candidate host design object in detStore"};
    SG::ReadCondHandleKey<traccc::detector_conditions_description::host> m_monCondKey{
        this, "MonCondKey", "DeviceDetectorDescriptionHostCond",
        "Key for reading the candidate host conditions object"};

    // ---- reference design + conditions ----
    Gaudi::Property<std::string> m_refHostDesignObjectName{
        this, "RefHostDesignObjectName", "", "Reference host design object in detStore"};
    Gaudi::Property<std::string> m_refHostCondKey{
        this, "RefHostCondKey", "", "Reference host cond object in detStore"};

    Gaudi::Property<float> m_shiftTolerance{
        this, "ShiftTolerance", 1e-4f, "Tolerance (mm) for Lorentz shift comparison"};
    Gaudi::Property<float> m_positionTolerance{
        this, "PositionTolerance", 1e-3f, "Tolerance (mm) for bin-edge comparison"};
};

}  // namespace ActsTrk

#endif  // ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONVALIDATIONALG_H