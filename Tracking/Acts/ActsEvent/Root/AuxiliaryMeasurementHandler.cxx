/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/AuxiliaryMeasurementHandler.h"

#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer1D.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer2D.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer3D.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"

#include "AthenaBaseComps/AthCheckMacros.h"

#include <format>
#include <functional>

namespace ActsTrk{
    MsgStream& AuxiliaryMeasurementHandler::msg(const MSG::Level lvl) const {
        return m_msgPrinter(lvl);
    }

    bool AuxiliaryMeasurementHandler::msgLvl(const MSG::Level lvl) const {
        return m_msgLevel(lvl);
    }
    StatusCode AuxiliaryMeasurementHandler::initialize(const std::string& preFix,
                                                       bool used) {
        if (preFix.empty()) {
            ATH_MSG_ERROR("The prefix key of the auxiliary measurements must no be empty");
            return StatusCode::FAILURE;
        }
        m_surfaceKey = std::format("{:}AuxiliarySurfaceContainer", preFix);
        if (!m_surfaceKey.initialize(used).isSuccess()) {
            ATH_MSG_ERROR("Failed to initialize "<<m_surfaceKey.fullKey());
            return StatusCode::FAILURE;
        }
        m_viewKey = preFix;
        if (!m_viewKey.initialize(used).isSuccess()) {
            ATH_MSG_ERROR("Failed to initialize the view element key "<<m_viewKey.fullKey());
            return StatusCode::FAILURE;
        }
        unsigned counter{1};
        for (auto& initMe : {&m_writeKey1D, &m_writeKey2D, &m_writeKey3D}){
            (*initMe) = std::format("{:}AuxiliaryMeasContainer{:}D", preFix, counter++);
            if (!initMe->initialize(used).isSuccess()) {
                ATH_MSG_ERROR("Failed to initialize "<<(*initMe).fullKey());
                return StatusCode::FAILURE;
            }
        }
        return StatusCode::SUCCESS;
    }
    template<typename AuxCont_t, typename Cont_t>
    StatusCode AuxiliaryMeasurementHandler::MeasurementProvider::recordContainer(SG::WriteHandle<Cont_t>& handle) {
        ATH_CHECK(handle.record(std::make_unique<Cont_t>(), std::make_unique<AuxCont_t>()));
        return StatusCode::SUCCESS;
    }


    AuxiliaryMeasurementHandler::MeasurementProvider::MeasurementProvider(const EventContext& ctx,
                                      const Acts::GeometryContext& gctx,
                                      const AuxiliaryMeasurementHandler* parent):
        m_ctx{ctx},
        m_gctx{gctx},
        m_parent{parent}{}
    StatusCode AuxiliaryMeasurementHandler::MeasurementProvider::setupContainers() {
        ATH_CHECK(m_viewHandle.record(std::make_unique<xAOD::AuxiliaryMeasurementContainer>(SG::VIEW_ELEMENTS)));        
        ATH_CHECK(recordContainer<xAOD::AuxiliaryMeasurementAuxContainer1D>(m_handle1D));
        ATH_CHECK(recordContainer<xAOD::AuxiliaryMeasurementAuxContainer2D>(m_handle2D));
        ATH_CHECK(recordContainer<xAOD::AuxiliaryMeasurementAuxContainer3D>(m_handle3D));
        ATH_CHECK(recordContainer<xAOD::TrackSurfaceAuxContainer>(m_surfaceContainer));
        return StatusCode::SUCCESS;
    }
    AuxiliaryMeasurementHandler::HandleReturn_t 
            AuxiliaryMeasurementHandler::makeHandle(const EventContext& ctx,
                                                    const Acts::GeometryContext& gctx) const  {
        if (m_surfaceKey.empty()) {
            return HandleReturn_t{HandleStatus::emptyKey};
        }
        MeasurementProvider newHandle{ctx, gctx, this};
        if (!newHandle.setupContainers().isSuccess()) {
            return HandleReturn_t{HandleStatus::recordFail};
        }
        return HandleReturn_t{std::move(newHandle)};
    }
}
#undef RECORD_CONTAINER