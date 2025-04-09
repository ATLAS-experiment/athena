/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentFitParDecorAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonSegment/MuonSegment.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
namespace MuonR4 {
    using namespace SegmentFit;
    using SegPars = xAOD::MeasVector<toInt(ParamDefs::nPars)>;
    using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
    using PrdLink_t = ElementLink<PrdCont_t>;
    using PrdLinkVec = std::vector<PrdLink_t>;
    using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;

    StatusCode SegmentFitParDecorAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_locParKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_keyTgc.initialize(!m_keyTgc.empty()));
        ATH_CHECK(m_keyRpc.initialize(!m_keyRpc.empty()));
        ATH_CHECK(m_keyMdt.initialize(!m_keyMdt.empty()));
        ATH_CHECK(m_keysTgc.initialize(!m_keysTgc.empty()));
        ATH_CHECK(m_keyMM.initialize(!m_keyMM.empty()));
        return StatusCode::SUCCESS;
    }
   StatusCode SegmentFitParDecorAlg::fetchMeasurement(const EventContext& ctx,
                                                      const MeasKey_t& key,
                                                      const Identifier& measId,
                                                      const xAOD::UncalibratedMeasurement*& meas) const {
        const PrdCont_t* cont{nullptr};
        ATH_CHECK(SG::get(cont, key, ctx));
        if (!cont) {
            ATH_MSG_VERBOSE("No container key given");
            return StatusCode::SUCCESS;
        }
        for (const xAOD::UncalibratedMeasurement* inCont : *cont) {
            if (xAOD::identify(inCont) == measId) {
                meas = inCont;
                break;
            }
        }
        if (!meas) {
            ATH_MSG_WARNING("Failed to find the hit "<<m_idHelperSvc->toString(measId));
        }
        return StatusCode::SUCCESS;
    }

    StatusCode SegmentFitParDecorAlg::execute(const EventContext& ctx) const {
        const xAOD::MuonSegmentContainer* segmentContainer{nullptr};
        ATH_CHECK(SG::get(segmentContainer, m_segmentKey, ctx));
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars> parDecor{m_locParKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, PrdLinkVec> prdLinkDecor{m_prdLinkKey, ctx};
        for (const xAOD::MuonSegment* seg : *segmentContainer) {
            PrdLinkVec& prdLinks{prdLinkDecor(*seg)};
            const Trk::Segment* trkSeg{*seg->muonSegment()};
            Identifier rotId{};

            auto addLink = [&, this](const Trk::RIO_OnTrack* rot) ->StatusCode{
                if (!rot) {
                    return StatusCode::SUCCESS;;
                }
                if (!rotId.is_valid()) {
                    rotId = rot->identify();
                }
                const xAOD::UncalibratedMeasurement* prd{nullptr};
                switch(m_idHelperSvc->technologyIndex(rot->identify())){
                    case TechIdx_t::MDT:
                        ATH_CHECK(fetchMeasurement(ctx, m_keyMdt, rot->identify(), prd));
                        break;
                    case TechIdx_t::RPC:
                        ATH_CHECK(fetchMeasurement(ctx, m_keyRpc, rot->identify(), prd));
                        break;
                    case TechIdx_t::TGC:
                        ATH_CHECK(fetchMeasurement(ctx, m_keyTgc, rot->identify(), prd));
                        break;
                    case TechIdx_t::MM:
                        ATH_CHECK(fetchMeasurement(ctx, m_keyMM, rot->identify(), prd));
                        break;
                    case TechIdx_t::STGC:
                        ATH_CHECK(fetchMeasurement(ctx, m_keysTgc, rot->identify(), prd));
                        break;
                    default:
                        break;
                };
                if (!prd) {
                    return StatusCode::SUCCESS;
                }
                ATH_MSG_VERBOSE("Link new measurement "<<m_idHelperSvc->toString(rot->identify()));
                PrdLink_t link{*static_cast<const PrdCont_t*>(prd->container()), 
                               prd->index()};
                prdLinks.push_back(std::move(link));
                return StatusCode::SUCCESS;
            };

            for (const Trk::MeasurementBase* meas : trkSeg->containedMeasurements()) {
                const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
                if (rot) {
                    ATH_CHECK(addLink(rot));
                    continue;
                }
                const auto* cRot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(meas);
                if (cRot) {                    
                    for (unsigned int r = 0 ; r < cRot->numberOfContainedROTs(); ++r){
                        ATH_CHECK(addLink(&cRot->rioOnTrack(r)));
                    }
                }
            }
            const MuonGMR4::SpectrometerSector* chamber = m_detMgr->getSectorEnvelope(rotId);
            const Amg::Transform3D globToLoc{chamber->globalToLocalTrans(*gctx)};

            SegPars& locPars{parDecor(*seg)};

            const Amg::Vector3D locDir = globToLoc.linear() * seg->direction();
            const Amg::Vector3D locPos = globToLoc * seg->position();

            const double travDist = Amg::intersect<3>(locPos, locDir, Amg::Vector3D::UnitZ(), 0).value_or(0);
            const Amg::Vector3D atCentre = locPos + travDist * locDir;
            
            locPars[toInt(ParamDefs::x0)]    = atCentre[toInt(AxisDefs::phi)];
            locPars[toInt(ParamDefs::y0)]    = atCentre[toInt(AxisDefs::eta)];
            locPars[toInt(ParamDefs::theta)] = locDir.theta();
            locPars[toInt(ParamDefs::phi)]   = locDir.phi();
            ATH_MSG_VERBOSE("Segment "<<m_idHelperSvc->toStringChamber(rotId)<<" at chamber centre "
                        <<Amg::toString(atCentre)<<" + x *"<<Amg::toString(locDir));
        }
        return StatusCode::SUCCESS;
    }
 
}