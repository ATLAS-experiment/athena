/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/AuxiliaryMeasurementHandler.h"

#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer1D.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer2D.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer3D.h"


#include <format>
#include <functional>
using ContType_t = xAOD::AuxiliaryMeasurementContainer;

#define RECORD_CONTAINER(HANDLE, AUXCONTAINER) \
    if (!HANDLE.record(std::make_unique<ContType_t>(),                  \
                       std::make_unique<AUXCONTAINER>()).isSuccess()){  \
        m_parent->m_msg<<MSG::FATAL<<"Failed to record "                \
                 <<HANDLE.fullKey()<<endmsg;                            \
        return StatusCode::FAILURE;                                     \
    }


namespace ActsTrk{
    StatusCode AuxiliaryMeasurementHandler::initialize(const std::string& preFix) {
        unsigned counter{1};
        for (auto& initMe : {&m_writeKey1D, &m_writeKey2D, &m_writeKey3D}){
            (*initMe) = std::format("{:}AuxiliaryMeasContainer{:}D", preFix, counter++);
            if (!initMe->initialize()) {
                m_msg<<MSG::FATAL<<"Failed to initialize "<<(*initMe).fullKey()<<endmsg;
                return StatusCode::FAILURE;
            }
        }
        return StatusCode::SUCCESS;
    }

    AuxiliaryMeasurementHandler::MeasurementProvider::MeasurementProvider(const EventContext& ctx,
                                      const AuxiliaryMeasurementHandler* parent,
                                      xAOD::TrackSurfaceContainer& surfaceBackend):
        m_ctx{ctx},
        m_parent{parent},
        m_surfaceContainer{surfaceBackend}{}
    StatusCode AuxiliaryMeasurementHandler::MeasurementProvider::setupContainers() {
        RECORD_CONTAINER(m_handle1D, xAOD::AuxiliaryMeasurementAuxContainer1D);
        RECORD_CONTAINER(m_handle2D, xAOD::AuxiliaryMeasurementAuxContainer2D);
        RECORD_CONTAINER(m_handle3D, xAOD::AuxiliaryMeasurementAuxContainer3D);
        return StatusCode::SUCCESS;
    }
    AuxiliaryMeasurementHandler::MeasurementProvider 
            AuxiliaryMeasurementHandler::makeHandle(const EventContext& ctx,
                                               xAOD::TrackSurfaceContainer& surfaceBackend) const {
        MeasurementProvider newHandle{ctx, this, surfaceBackend};
        if (!newHandle.setupContainers()) {
            THROW_EXCEPTION("Failed to setup the auxillary handle");
        }
        return newHandle;
    }
}
#undef RECORD_CONTAINER