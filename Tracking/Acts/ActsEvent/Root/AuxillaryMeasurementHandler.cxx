/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/AuxillaryMeasurementHandler.h"

#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer1D.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer2D.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer3D.h"


#include <format>
#include <functional>
using ContType_t = xAOD::AuxillaryMeasurementContainer;

#define RECORD_CONTAINER(HANDLE, AUXCONTAINER) \
    if (!HANDLE.record(std::make_unique<ContType_t>(),                  \
                       std::make_unique<AUXCONTAINER>()).isSuccess()){  \
        m_parent->m_msg<<MSG::FATAL<<"Failed to record "                \
                 <<HANDLE.fullKey()<<endmsg;                            \
        return StatusCode::FAILURE;                                     \
    }


namespace ActsTrk{
    StatusCode AuxillaryMeasurementHandler::initialize(const std::string& preFix) {
        unsigned counter{1};
        for (auto& initMe : {&m_writeKey1D, &m_writeKey2D, &m_writeKey3D}){
            (*initMe) = std::format("{:}AuxillaryMeasContainer{:}D", preFix, counter++);
            if (!initMe->initialize()) {
                m_msg<<MSG::FATAL<<"Failed to initialize "<<(*initMe).fullKey()<<endmsg;
                return StatusCode::FAILURE;
            }
        }
        return StatusCode::SUCCESS;
    }

    AuxillaryMeasurementHandler::MeasurementProvider::MeasurementProvider(const EventContext& ctx,
                                      const AuxillaryMeasurementHandler* parent,
                                      xAOD::TrackSurfaceContainer& surfaceBackend):
        m_ctx{ctx},
        m_parent{parent},
        m_surfaceContainer{surfaceBackend}{}
    StatusCode AuxillaryMeasurementHandler::MeasurementProvider::setupContainers() {
        RECORD_CONTAINER(m_handle1D, xAOD::AuxillaryMeasurementAuxContainer1D);
        RECORD_CONTAINER(m_handle2D, xAOD::AuxillaryMeasurementAuxContainer2D);
        RECORD_CONTAINER(m_handle3D, xAOD::AuxillaryMeasurementAuxContainer3D);
        return StatusCode::SUCCESS;
    }
    AuxillaryMeasurementHandler::MeasurementProvider 
            AuxillaryMeasurementHandler::makeHandle(const EventContext& ctx,
                                               xAOD::TrackSurfaceContainer& surfaceBackend) const {
        MeasurementProvider newHandle{ctx, this, surfaceBackend};
        if (!newHandle.setupContainers()) {
            THROW_EXCEPTION("Failed to setup the auxillary handle");
        }
        return newHandle;
    }
}
#undef RECORD_CONTAINER