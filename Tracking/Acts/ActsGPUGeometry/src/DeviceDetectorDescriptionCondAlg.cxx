/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceDetectorDescriptionCondAlg.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/WriteCondHandle.h"

namespace ActsTrk {

StatusCode DeviceDetectorDescriptionCondAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing  device detector conditions description algorithm ");

    ATH_CHECK(m_MRs.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(detStore()->retrieve(m_pixelID, m_pixelIdHelperName) );
    ATH_CHECK(detStore()->retrieve(m_stripID, m_stripIdHelperName));

    ATH_CHECK(m_writeHostCondKey.initialize());
    ATH_CHECK(m_writeDeviceCondKey.initialize());

    ATH_CHECK(m_stripLorentzAngleTool.retrieve());
    ATH_CHECK(m_pixelLorentzAngleTool.retrieve());

    // The static part of the detector description (id maps, module designs,
    // device detector) is built by the service during its initialization.
    ATH_CHECK(m_detDescSvc.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode DeviceDetectorDescriptionCondAlg::execute(const EventContext& ctx) const
{
    SG::WriteCondHandle<traccc::detector_conditions_description::host> writeHostHandle{m_writeHostCondKey, ctx};
    SG::WriteCondHandle<traccc::detector_conditions_description::buffer> writeDeviceHandle{m_writeDeviceCondKey, ctx};

    if (writeHostHandle.isValid() && writeDeviceHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandles " << writeHostHandle.fullKey() << " and "
                      << writeDeviceHandle.fullKey() << " are already valid.");
        return StatusCode::SUCCESS;
    }

    const std::vector<StaticCondEntry>& staticCondEntries = m_detDescSvc->staticCondEntries();

    auto hostCond = std::make_unique<traccc::detector_conditions_description::host>(*m_MRs->hostMR());
    hostCond->resize(staticCondEntries.size());

    // ---- 5. Write detector conditions object ----
    // This object basically stores any information that we need per-module
    // like: detray id, acts id, index to module design, lorentz shift etc.
    // since lorentz shift is conditional, we need a valid event context
    // all the static info (like the id maps) have been pre-filled by the service,
    // here we just populate anything that might be conditions dependant

    for (std::size_t condIndex = 0; condIndex < staticCondEntries.size(); ++condIndex) {
        const auto& entry = staticCondEntries[condIndex];

        hostCond->module_to_design_id()[condIndex] = entry.designId;
        hostCond->geometry_id()[condIndex] = entry.detrayGeometryId;
        hostCond->acts_geometry_id()[condIndex] = entry.actsGeometryId;

        float shiftX = 0.f, shiftY = 0.f;
        if (entry.hasAthenaModule) {
            if (entry.isPixel) {
                const IdentifierHash pixelHash = m_pixelID->wafer_hash(entry.athenaId);
                shiftX = m_pixelLorentzAngleTool->getLorentzShift(pixelHash, ctx);
            } else {
                const IdentifierHash waferHash = m_stripID->wafer_hash(entry.athenaId);
                shiftX = m_stripLorentzAngleTool->getLorentzShift(waferHash, ctx);
            }
        }

        hostCond->measurement_translation()[condIndex] =
            traccc::vector2{static_cast<traccc::scalar>(shiftX), static_cast<traccc::scalar>(shiftY)};
    }

    auto copy = m_copy->copy(ctx);
    auto deviceCond = std::make_unique<traccc::detector_conditions_description::buffer>(
        static_cast<traccc::detector_conditions_description::buffer::size_type>(hostCond->size()),
        m_MRs->mainMR());
    (*copy).setup(*deviceCond)->wait();
    (*copy)(vecmem::get_data(*hostCond), *deviceCond)->wait();

    // PLACEHOLDER: no real conditions dependency yet (e.g. alignment tag),
    // so mark this object as valid for all run/lumi. Replace with a proper
    // addDependency(ReadCondHandle<...>&) once this depends on something
    // that actually changes over the run.
    
    writeHostHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
    writeDeviceHandle.addDependency(IOVInfiniteRange::infiniteRunLB());

    ATH_CHECK(writeHostHandle.record(std::move(hostCond)));
    ATH_CHECK(writeDeviceHandle.record(std::move(deviceCond)));

    ATH_MSG_DEBUG("Recorded host and device detector conditions description: " << m_writeHostCondKey.key() << ", " << m_writeDeviceCondKey.key());

    return StatusCode::SUCCESS;
}

} // namespace ActsTrk