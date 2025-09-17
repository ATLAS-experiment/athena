/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSegmentCnvAlg.h"

#include "xAODMuon/MuonSegmentAuxContainer.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h" 
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"



namespace MuonR4{
    using namespace SegmentFit;
    using SegLink_t = ElementLink<MuonR4::SegmentContainer>;
    using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
    using PrdLink_t = ElementLink<PrdCont_t>;
    using PrdLinkVec_t = std::vector<PrdLink_t>;

    using SegPars_t = xAOD::MeasVector<Acts::toUnderlying(ParamDefs::nPars)>;


    using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;
    
    constexpr TechIdx_t toTechIdx(const xAOD::UncalibMeasType aodType){
        switch (aodType){
            case xAOD::UncalibMeasType::MdtDriftCircleType:
                return TechIdx_t::MDT;
            case xAOD::UncalibMeasType::RpcStripType:
                return TechIdx_t::RPC;
            case xAOD::UncalibMeasType::TgcStripType:
                return TechIdx_t::TGC;
            case xAOD::UncalibMeasType::MMClusterType:
                return TechIdx_t::MM;
            case xAOD::UncalibMeasType::sTgcStripType:
                return TechIdx_t::STGC;
            default:
                return TechIdx_t::TechnologyUnknown;
        }
    }

    StatusCode xAODSegmentCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());

        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_localSegParKey.initialize());
        ATH_CHECK(m_parentSegKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode xAODSegmentCnvAlg::execute(const EventContext& ctx) const {

        SG::WriteHandle outContainer{m_writeKey, ctx};
        ATH_CHECK(outContainer.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                      std::make_unique<xAOD::MuonSegmentAuxContainer>()));
        
        
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegLink_t> dec_parentLink{m_parentSegKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars_t> dec_locPars{m_localSegParKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, PrdLinkVec_t> dec_prdLinks{m_prdLinkKey, ctx};
        const SegmentContainer* segmentContainer{nullptr};
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(segmentContainer, m_readKey, ctx));
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        for (const Segment* inSegment : *segmentContainer) {
            const MuonGMR4::SpectrometerSector* sector = inSegment->msSector();

            xAOD::MuonSegment* convertedSeg = outContainer->push_back(std::make_unique<xAOD::MuonSegment>());
            dec_parentLink(*convertedSeg) = SegLink_t{segmentContainer, outContainer->size() -1};
            
            const Amg::Vector3D& pos{inSegment->position()};
            const Amg::Vector3D& dir{inSegment->direction()};
            const Amg::Transform3D globToLoc{sector->globalToLocalTrans(*gctx)};
            convertedSeg->setPosition(pos.x(), pos.y(), pos.z());
            convertedSeg->setDirection(dir.x(), dir.y(), dir.z());
            

            convertedSeg->setIdentifier(sector->sector(), sector->chamberIndex(), sector->side(), 
                                        toTechIdx(inSegment->summary().tech));
            convertedSeg->setFitQuality(inSegment->chi2(), inSegment->nDoF());
            convertedSeg->setNHits(inSegment->summary().nPrecHits, inSegment->summary().nPhiHits,
                                   inSegment->summary().nEtaTrigHits);
           
            
            convertedSeg->setT0Error(inSegment->segementT0(), 
                                     Amg::error(inSegment->covariance(), Acts::toUnderlying(ParamDefs::t0)));

            /**  */
            SegPars_t& localPars{dec_locPars(*convertedSeg)};
            const Amg::Vector3D locPos{globToLoc * pos};
            const Amg::Vector3D locDir{globToLoc.linear() * dir};

            localPars[Acts::toUnderlying(ParamDefs::x0)] = locPos.x();
            localPars[Acts::toUnderlying(ParamDefs::y0)] = locPos.y();
            localPars[Acts::toUnderlying(ParamDefs::theta)] = locDir.theta();
            localPars[Acts::toUnderlying(ParamDefs::phi)] = locDir.phi();
            localPars[Acts::toUnderlying(ParamDefs::t0)] = inSegment->segementT0();

            /** Append the prd links */
            PrdLinkVec_t& prdLinks{dec_prdLinks(*convertedSeg)};
            auto appendLink = [&prdLinks](const xAOD::UncalibratedMeasurement* meas) {
                if (!meas) {
                    return;
                }
                prdLinks.emplace_back(static_cast<const xAOD::UncalibratedMeasurementContainer*>(meas->container()),
                                      meas->index());
                
            };
            for (const auto& meas : inSegment->measurements()) {
                if (!meas->spacePoint()) {
                    continue;
                }
                appendLink(meas->spacePoint()->primaryMeasurement());
                if (meas->spacePoint()->primaryMeasurement() != 
                    meas->spacePoint()->secondaryMeasurement()){
                    appendLink(meas->spacePoint()->secondaryMeasurement());
                }
            }
        }                             
        return StatusCode::SUCCESS;
    }
}
