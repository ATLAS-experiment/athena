/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CombinedFitAlg.h"


namespace MuonCombinedR4 {

    StatusCode CombinedFitAlg::initialize() {
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_stacoKey.initialize());
        ATH_CHECK(m_outTagKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode CombinedFitAlg::prepareDataShip(const EventContext& ctx, DataShip& ship) const {
        ATH_CHECK(SG::get(ship.preMatchedTags, m_stacoKey, ctx));
        ATH_CHECK(ship.combinedTags.record(m_outTagKey, ctx));
        ship.calContext = m_ctxProvider.getCalibrationContext(ctx);
        ship.mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
        ship.tgContext = m_ctxProvider.getGeometryContext(ctx);
        return StatusCode::SUCCESS;
    }
    StatusCode CombinedFitAlg::execute(const EventContext& ctx) const  {
        DataShip ship{};
        ATH_CHECK(prepareDataShip(ctx, ship));
        return StatusCode::SUCCESS;
    }
}