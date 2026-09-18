/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsGeometry/ContextUtility.h"

#include "AthenaBaseComps/AthCheckMacros.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "ActsCalibBase/CalibrationContext.h"

#include "GeoModelKernel/throwExcept.h"

namespace ActsTrk{
    StatusCode ContextUtility::initialize(const bool enable) {
        ATH_CHECK(m_geoCtxKey.initialize(enable && !m_geoCtxKey.empty()));
        ATH_CHECK(m_magCtxKey.initialize(enable && !m_magCtxKey.empty()));
        return StatusCode::SUCCESS;
    }

    MsgStream& ContextUtility::msg(const MSG::Level lvl) const {
        return m_msgPrinter(lvl);
    }

    bool ContextUtility::msgLvl(const MSG::Level lvl) const {
        return m_msgLevel(lvl);
    }

    Acts::GeometryContext ContextUtility::getGeometryContext(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        if (!SG::get(gctx, m_geoCtxKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to retrieve the geometry context "<<m_geoCtxKey.fullKey());

        }
        if (!gctx) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No geometry context passed");
            return Acts::GeometryContext{gctx};
        }
        return gctx->context();
    }

    Acts::CalibrationContext ContextUtility::getCalibrationContext(const EventContext& ctx) const {
        return ActsTrk::getCalibrationContext(ctx);
    }

    Acts::MagneticFieldContext ContextUtility::getMagneticFieldContext(const EventContext& ctx) const {
        const AtlasFieldCacheCondObj* mctx{nullptr};
        if (!SG::get(mctx, m_magCtxKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to retrieve the magnetic field context "<<m_magCtxKey.fullKey());

        }
        if (!mctx) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No magentic field context passed");
        }
        return Acts::MagneticFieldContext{mctx};
    }
}
          
