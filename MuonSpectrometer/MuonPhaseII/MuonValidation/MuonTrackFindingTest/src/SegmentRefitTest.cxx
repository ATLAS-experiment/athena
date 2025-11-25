/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentRefitTest.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "StoreGate/ReadDecorHandle.h"

using namespace MuonR4::SegmentFit;
using namespace Acts;

namespace MuonValR4{
    StatusCode SegmentRefitTest::initialize() {
        ATH_CHECK(m_preFitKey.initialize());
        ATH_CHECK(m_postFitKey.initialize());    
        ATH_CHECK(m_linkKey.initialize());    
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentRefitTest::finalize(){
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    StatusCode SegmentRefitTest::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const xAOD::MuonSegmentContainer* postFitSegments{nullptr};
        ATH_CHECK(SG::get(postFitSegments, m_postFitKey, ctx));

        using Link_t = ElementLink<xAOD::MuonSegmentContainer>;
        SG::ReadDecorHandle<xAOD::MuonSegmentContainer, Link_t> acc_segLink{m_linkKey, ctx};
        /// Create the context object
        std::unordered_set<const xAOD::MuonSegment*> filledPreFits{};
        auto fillPrefit = [&] (const xAOD::MuonSegment* seg) -> StatusCode {
            if (!filledPreFits.insert(seg).second) {
                return StatusCode::SUCCESS;
            }
            const MuonR4::Segment* reFitMe = MuonR4::detailedSegment(*seg);

            auto preFitPars = localSegmentPars(*seg);
            m_chamberIndex = toUnderlying(reFitMe->msSector()->chamberIndex());
            m_stationSide = reFitMe->msSector()->side();
            m_stationPhi = reFitMe->msSector()->stationPhi();
            /** parameters */
            using enum ParamDefs;
            m_preFitLocX = preFitPars[toUnderlying(x0)];
            m_preFitLocY = preFitPars[toUnderlying(y0)];
            m_preFitTheta = preFitPars[toUnderlying(theta)];
            m_preFitPhi = preFitPars[toUnderlying(phi)];
            /** uncertainty */
            m_uncertLocX = Amg::error(reFitMe->covariance(), toUnderlying(x0));
            m_uncertLocY = Amg::error(reFitMe->covariance(), toUnderlying(y0));
            m_uncertTheta = Amg::error(reFitMe->covariance(), toUnderlying(theta));
            m_uncertPhi = Amg::error(reFitMe->covariance(), toUnderlying(phi));

            m_preFitChi2 = reFitMe->chi2();
            m_preFitNdoF = reFitMe->nDoF();
            m_preFitNPrecHits = reFitMe->summary().nPrecHits;
            m_preFitNTrigEtaHits = reFitMe->summary().nEtaTrigHits;
            m_preFitNTrigPhiHits = reFitMe->summary().nPhiHits;
            static const SG::ConstAccessor<xAOD::MeasVector<toUnderlying(nPars)>> acc_seed{"seedSegPars"};
            m_seedFitLocY  = acc_seed(*seg)[toUnderlying(y0)];
            m_seedFitTheta = acc_seed(*seg)[toUnderlying(theta)];
            m_seedFitLocX  = acc_seed(*seg)[toUnderlying(x0)];
            m_seedFitPhi   = acc_seed(*seg)[toUnderlying(phi)];

            return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
        };
        /// Loop over the post fit segment container
        for (const xAOD::MuonSegment* seg: *postFitSegments){
            m_postFitChi2 = seg->chiSquared();           
            m_postFitNdoF = seg->numberDoF();
            m_postFitNPrecHits = seg->nPrecisionHits();
            m_postFitNTrigEtaHits = seg->nTrigEtaLayers();
            m_postFitNTrigPhiHits = seg->nPhiLayers();
            const auto segPars = localSegmentPars(*seg);
            using enum ParamDefs;
            m_postFitLocX  = segPars[toUnderlying(x0)];
            m_postFitLocY  = segPars[toUnderlying(y0)];
            m_postFitTheta = segPars[toUnderlying(theta)];
            m_postFitPhi   = segPars[toUnderlying(phi)];
            
            m_goodFit = true;
            ATH_CHECK(fillPrefit((*acc_segLink(*seg))));
        }
        const xAOD::MuonSegmentContainer* preFitSegments{nullptr};
        ATH_CHECK(SG::get(preFitSegments, m_preFitKey, ctx));
        for (const xAOD::MuonSegment* preFit: *preFitSegments) {
            ATH_CHECK(fillPrefit(preFit));
        }
        ATH_MSG_DEBUG("Processing done");
        return StatusCode::SUCCESS;
    }
}