/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentFitParDecorAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonSegment/MuonSegment.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "xAODMuonViews/ContainerDecorator.h"

namespace {
    std::string print(const xAOD::MuonMeasurementContainer& container) {
        if (container.empty()) {
            return "empty container";
        }
        const Muon::IMuonIdHelperSvc* idHelperSvc = container.at(0)->readoutElement()->idHelperSvc();
        std::stringstream sstr{};
        for (const xAOD::MuonMeasurement* m : container) {
            sstr<<" --- "<<idHelperSvc->toString(m->identify())<<", hash: "<<m->identifierHash()<<", dim:"<<m->numDimensions()<<std::endl;
        }
        return sstr.str();
    }
}


namespace MuonR4 {
    using namespace SegmentFit;
    using SegPars_t = xAOD::PosAccessor<Acts::toUnderlying(ParamDefs::nPars)>::element_type;

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
    const SegmentFitParDecorAlg::MeasKey_t& 
          SegmentFitParDecorAlg::fetchKey(const TechIdx_t idx) const {
        switch (idx) {
             using enum TechIdx_t;
            case MDT: return m_keyMdt;
            case RPC: return m_keyRpc;
            case TGC: return m_keyTgc;
            case MM: return m_keyMM;
            case STGC: return m_keysTgc;
            default:
                THROW_EXCEPTION("Invalid technology index "<<idx);
        }
        return m_keyMdt;
    }
   
    StatusCode SegmentFitParDecorAlg::execute(const EventContext& ctx) const {
        const xAOD::MuonSegmentContainer* segmentContainer{nullptr};
        const ActsTrk::GeometryContext* gctx{nullptr};

        ATH_CHECK(SG::get(segmentContainer, m_segmentKey, ctx));
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, SegPars_t> parDecor{m_locParKey, ctx};
        xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, PrdLinkVec> prdLinkDecor{m_prdLinkKey, ctx};
        for (const xAOD::MuonSegment* seg : *segmentContainer) {
            PrdLinkVec& prdLinks{prdLinkDecor(*seg)};
            const Trk::Segment* trkSeg{*seg->muonSegment()};
            
            std::array<std::vector<Identifier>, toInt(TechIdx_t::TechnologyIndexMax)> idsPerTech{};
            /// Sort first all measurement Identifiers by technology index
            auto appendId = [&] (const Identifier& id) {
                idsPerTech[toInt(m_idHelperSvc->technologyIndex(id))].push_back(id);
            };
            for (const Trk::MeasurementBase* meas : trkSeg->containedMeasurements()) {
                if (const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas)) {
                    appendId(rot->identify());
                } else if (const auto* cRot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(meas)){
                    for (unsigned int r = 0 ; r < cRot->numberOfContainedROTs(); ++r){
                        appendId(cRot->rioOnTrack(r).identify());
                    }
                }
            }
            for (std::vector<Identifier>& ids : idsPerTech) {
                if (ids.empty()) {
                    continue;
                }
                std::ranges::sort(ids,[this](const Identifier& a, const Identifier& b){
                    return m_idHelperSvc->detElementHash(a) < m_idHelperSvc->detElementHash(b);
                });
                const MeasKey_t& key = fetchKey(m_idHelperSvc->technologyIndex(ids.front()));
                if (key.empty()) {
                    ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The key for technology "
                        <<m_idHelperSvc->technologyIndex(ids.front())<<" is empty.");
                    return StatusCode::FAILURE;
                }
                const xAOD::MuonMeasurementContainer* container{nullptr};
                ATH_CHECK(SG::get(container, key, ctx));
                ATH_MSG_VERBOSE("Retrieve container "<<container->size()<<"\n\n"<<print(*container));
                xAOD::ChamberViewer viewer{*container};
                if (!viewer.loadView(m_idHelperSvc->detElementHash(ids.front()))){
                    ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Cannot find a xAOD view for "
                        <<m_idHelperSvc->toStringDetEl(ids.front())<<"\n\n"<<print(*container));
                    return StatusCode::FAILURE;
                }
                for (const Identifier& id : ids) {
                    const IdentifierHash dHash = m_idHelperSvc->detElementHash(id);
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Search for view "
                        <<m_idHelperSvc->toStringDetEl(id)<<", hash: "<<dHash);
                    while(viewer.at(0)->identifierHash() != dHash) {
                        if (!viewer.next()) {
                            if (viewer.loadView(dHash)) {
                                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Container is not sorted "
                                    <<m_idHelperSvc->toStringDetEl(id)<<".");
                                break;
                            }
                            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Cannot find a xAOD view for "
                                <<m_idHelperSvc->toStringDetEl(id)<<"\n\n"<<print(*container));
                            return StatusCode::FAILURE;
                        }
                    }
                    const auto itr = std::ranges::find_if(viewer,[&id](const xAOD::MuonMeasurement* meas){
                        return meas->identify() == id;
                    });
                    if (itr == viewer.end()) {
                        ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Cannot find measurement "<<m_idHelperSvc->toString(id)
                                    <<"\n\n"<<print(*container));
                        return StatusCode::FAILURE;
                    }
                    const auto* m{*itr};
                    prdLinks.emplace_back(container, m->index());
                }
            }
            const MuonGMR4::SpectrometerSector* chamber = m_detMgr->getSectorEnvelope(seg->chamberIndex(),
                                                                                      seg->sector(),
                                                                                      seg->etaIndex());
            const Amg::Transform3D globToLoc{chamber->globalToLocalTransform(*gctx)};

            SegPars_t& locPars{parDecor(*seg)};

            const Amg::Vector3D locDir = globToLoc.linear() * seg->direction();
            const Amg::Vector3D locPos = globToLoc * seg->position();

            const double travDist = Amg::intersect<3>(locPos, locDir, Amg::Vector3D::UnitZ(), 0).value_or(0);
            const Amg::Vector3D atCentre = locPos + travDist * locDir;
            
            locPars[Acts::toUnderlying(ParamDefs::x0)]    = atCentre.x();
            locPars[Acts::toUnderlying(ParamDefs::y0)]    = atCentre.y();
            locPars[Acts::toUnderlying(ParamDefs::theta)] = locDir.theta();
            locPars[Acts::toUnderlying(ParamDefs::phi)]   = locDir.phi();
            ATH_MSG_VERBOSE("Segment "<<chamber->identString()<<" at chamber centre "<<Amg::toString(atCentre)
                          <<" + x *"<<Amg::toString(locDir));
        }
        return StatusCode::SUCCESS;
    } 
}