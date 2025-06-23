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
            m_chamberIndex = toInt(reFitMe->msSector()->chamberIndex());
            m_stationSide = reFitMe->msSector()->side();
            m_stationPhi = reFitMe->msSector()->stationPhi();
            /** parameters */
            m_preFitLocX = preFitPars[toInt(ParamDefs::x0)];
            m_preFitLocY = preFitPars[toInt(ParamDefs::y0)];
            m_preFitTheta = preFitPars[toInt(ParamDefs::theta)];
            m_preFitPhi = preFitPars[toInt(ParamDefs::phi)];
            /** uncertainty */
            m_uncertLocX = Amg::error(reFitMe->covariance(), toInt(ParamDefs::x0));
            m_uncertLocY = Amg::error(reFitMe->covariance(), toInt(ParamDefs::y0));
            m_uncertTheta = Amg::error(reFitMe->covariance(), toInt(ParamDefs::theta));
            m_uncertPhi = Amg::error(reFitMe->covariance(), toInt(ParamDefs::phi));

            m_preFitChi2 = reFitMe->chi2();
            m_preFitNdoF = reFitMe->nDoF();
            m_preFitNPrecHits = reFitMe->summary().nPrecHits;
            m_preFitNTrigEtaHits = reFitMe->summary().nEtaTrigHits;
            m_preFitNTrigPhiHits = reFitMe->summary().nPhiHits;
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
            m_postFitLocX  = segPars[toInt(ParamDefs::x0)];
            m_postFitLocY  = segPars[toInt(ParamDefs::y0)];
            m_postFitTheta = segPars[toInt(ParamDefs::theta)];
            m_postFitPhi   = segPars[toInt(ParamDefs::phi)];
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