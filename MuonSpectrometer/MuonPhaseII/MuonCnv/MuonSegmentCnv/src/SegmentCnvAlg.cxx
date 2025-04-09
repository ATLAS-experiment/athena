/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentCnvAlg.h"
#include "MuonSegment/MuonSegment.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonSegment/MuonSegmentQuality.h"


namespace MuonR4{
    
    StatusCode SegmentCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_keyTgc.initialize(!m_keyTgc.empty()));
        ATH_CHECK(m_keyRpc.initialize(!m_keyRpc.empty()));
        ATH_CHECK(m_keyMdt.initialize(!m_keyMdt.empty()));
        ATH_CHECK(m_keysTgc.initialize(!m_keysTgc.empty()));
        ATH_CHECK(m_keyMM.initialize(!m_keyMM.empty()));
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_mdtCreator.retrieve());
        ATH_CHECK(m_clusterCreator.retrieve());
        ATH_CHECK(m_compClusterCreator.retrieve(EnableTool{!m_keyTgc.empty() || !m_keyRpc.empty()}));
        ATH_CHECK(m_geoCtxKey.initialize());
        return StatusCode::SUCCESS;
    }
    
    StatusCode SegmentCnvAlg::execute(const EventContext& ctx) const {

        auto translatedSegments = std::make_unique<Trk::SegmentCollection>();
        for (const SG::ReadHandleKey<SegmentContainer>& key : m_readKeys) {
            const SegmentContainer* translateMe{nullptr};
            ATH_CHECK(SG::get(translateMe, key, ctx));
            for (const Segment* segment : *translateMe) {
                ATH_CHECK(convert(ctx, *segment, *translatedSegments));
            }

        }
        ATH_MSG_VERBOSE("Translated in total "<<translatedSegments->size()<<" segments.");
        
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(translatedSegments)));
        return StatusCode::SUCCESS;
    }
    template <class PrdType> 
        const PrdType* SegmentCnvAlg::fetchPrd(const Identifier& prdId,
                                               const Muon::MuonPrepDataContainerT<PrdType>* prdContainer) const {
        if (!prdContainer) {
            ATH_MSG_ERROR("Cannot fetch a prep data object as the container given for "<<
                          m_idHelperSvc->toString(prdId)<<" is a nullptr");
            return nullptr;
        }
        const Muon::MuonPrepDataCollection<PrdType>* coll = prdContainer->indexFindPtr(m_idHelperSvc->moduleHash(prdId));
        if (!coll) {
            ATH_MSG_ERROR("No prep data collection where "<<m_idHelperSvc->toString(prdId)<<" can reside in.");
            return nullptr;
        }
        for (const PrdType* prd : *coll) {
            if (prd->identify() == prdId){
                return prd;
            }
        }
        ATH_MSG_ERROR("There is no measurement "<<m_idHelperSvc->toString(prdId));

        return nullptr;    
    }
    template <class PrdType>
        StatusCode SegmentCnvAlg::convertMeasurement(const MuonR4::Segment& segment,
                                                     const CalibratedSpacePoint& spacePoint,
                                                     const Muon::MuonPrepDataContainerT<PrdType>* prdContainer,
                                                     std::vector<std::unique_ptr<Trk::RIO_OnTrack>>& convMeasVec) const {
        bool added{false};

        for (const xAOD::UncalibratedMeasurement* uncalib: {spacePoint.spacePoint()->primaryMeasurement(), 
                                                            spacePoint.spacePoint()->secondaryMeasurement()}){
            if (!uncalib) continue;
            added = true;

            const PrdType* prd = fetchPrd(xAOD::identify(uncalib), prdContainer);
            if (!prd) {
                ATH_MSG_FATAL("Failed to retrieve segment from "<<m_idHelperSvc->toString(xAOD::identify(uncalib)));
                return StatusCode::FAILURE;
            }
            const Trk::Surface& surf{prd->detectorElement()->surface(prd->identify())};
            Trk::Intersection isect = surf.straightLineIntersection(segment.position(), segment.direction());

            std::unique_ptr<Trk::RIO_OnTrack> rot{};
            if constexpr(std::is_same_v<PrdType, Muon::MdtPrepData>) {
                rot = std::unique_ptr<Trk::RIO_OnTrack>{m_mdtCreator->createRIO_OnTrack(*prd, 
                                                                                        isect.position,
                                                                                        &segment.direction())};
            } else {
                rot = std::unique_ptr<Trk::RIO_OnTrack>{m_clusterCreator->createRIO_OnTrack(*prd, 
                                                                                            isect.position,
                                                                                            segment.direction())};
            }
            if (!rot) {
                ATH_MSG_ERROR("Failed to create rot from "<<m_idHelperSvc->toString(prd->identify()));
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Created ROT "<<m_printer->print(*rot));
            convMeasVec.push_back(std::move(rot));
        }
        if (!added) {
            ATH_MSG_ERROR("Could not translate space point "<<m_idHelperSvc->toString(spacePoint.spacePoint()->identify()));
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode SegmentCnvAlg::convert(const EventContext& ctx,
                                      const MuonR4::Segment& segment,
                                      Trk::SegmentCollection& outContainer) const {

        const Muon::RpcPrepDataContainer* rpcPrds{nullptr};
        const Muon::MdtPrepDataContainer* mdtPrds{nullptr};
        const Muon::TgcPrepDataContainer* tgcPrds{nullptr};
        const Muon::sTgcPrepDataContainer* stgcPrds{nullptr};
        const Muon::MMPrepDataContainer* mmPrds{nullptr};

        ATH_CHECK(SG::get(mdtPrds, m_keyMdt, ctx));
        ATH_CHECK(SG::get(rpcPrds, m_keyRpc, ctx));
        ATH_CHECK(SG::get(tgcPrds, m_keyTgc, ctx));
        ATH_CHECK(SG::get(stgcPrds, m_keysTgc, ctx));
        ATH_CHECK(SG::get(mmPrds, m_keyMM, ctx));

        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        
        std::vector<std::unique_ptr<Trk::RIO_OnTrack>> rots{};
        unsigned int nPrec{0};
        for (const Segment::MeasType& spacePoint : segment.measurements()){
            if (spacePoint->fitState() != CalibratedSpacePoint::State::Valid) {
                // ATH_MSG_VERBOSE("Reject in")
                continue;
            }
            switch (spacePoint->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    ATH_CHECK(convertMeasurement(segment, *spacePoint, mdtPrds, rots));
                    ++nPrec;
                    break;
                }
                case xAOD::UncalibMeasType::RpcStripType: {
                    ATH_CHECK(convertMeasurement(segment,*spacePoint, rpcPrds, rots));
                    break;
                }
                case xAOD::UncalibMeasType::TgcStripType: {
                    ATH_CHECK(convertMeasurement(segment,*spacePoint, tgcPrds, rots));
                    break;
                }
                case xAOD::UncalibMeasType::MMClusterType:{
                    ATH_CHECK(convertMeasurement(segment,*spacePoint, mmPrds, rots));
                    ++nPrec;
                    break;
                }
                case xAOD::UncalibMeasType::sTgcStripType: {
                    ATH_CHECK(convertMeasurement(segment,*spacePoint, stgcPrds, rots));
                    ++nPrec;
                    break;
                }
                case xAOD::UncalibMeasType::Other:
                    break;
                default:
                    ATH_MSG_WARNING("Unsupported measurement type ");
            }
        }

        DataVector<const Trk::MeasurementBase> measurements{};
        auto makeCompetingROT = [this, &measurements](RotVec& rots) {
            if (rots.empty()){
                return;
            }
            std::list<const Trk::PrepRawData*> prds{};
            for (const std::unique_ptr<Trk::RIO_OnTrack>& rot : rots) {
                prds.push_back(rot->prepRawData());
            }
            measurements.push_back(m_compClusterCreator->createBroadCluster(std::move(prds),0.));
            rots.clear();
        };

        RotVec etaPrds{}, phiPrds{};
        for (std::unique_ptr<Trk::RIO_OnTrack>& rot : rots) {
            const Trk::PrepRawData* prd = rot->prepRawData();
            if (prd->type(Trk::PrepRawDataType::RpcPrepData) || prd->type(Trk::PrepRawDataType::TgcPrepData)) {
                std::vector<std::unique_ptr<Trk::RIO_OnTrack>>& pushMe{m_idHelperSvc->measuresPhi(rot->identify())? phiPrds : etaPrds};
                if (pushMe.size() && pushMe.back()->detectorElement() != rot->detectorElement()){
                    makeCompetingROT(pushMe);
                } else {
                    pushMe.push_back(std::move(rot));
                }
            
            } else {
                makeCompetingROT(etaPrds);
                makeCompetingROT(phiPrds);
                measurements.push_back(std::move(rot));
            }
        }
        makeCompetingROT(etaPrds);
        makeCompetingROT(phiPrds);     
        if (!nPrec) {
            ATH_MSG_WARNING("No precision hit on "<<std::endl<<m_printer->print(measurements.stdcont())
                <<". Do not convert segment due to potential puff.");
            return StatusCode::SUCCESS;
        }
        ATH_MSG_DEBUG("Fetched in total "<<measurements.size()<<" measurements. "<<std::endl<<
                        m_printer->print(measurements.stdcont()));
        /// Next build a surface
        const Amg::Transform3D& locToGlob{segment.msSector()->localToGlobalTrans(*gctx)};
        auto segSurf = std::make_unique<Trk::PlaneSurface>(Amg::getTransformFromRotTransl(locToGlob.linear(), segment.position()));
        Trk::LocalDirection segDir{};
        segSurf->globalToLocalDirection(segment.direction(), segDir);
        std::vector<Identifier> holes{};
        auto fitQuality = std::make_unique<Muon::MuonSegmentQuality>(segment.chi2(), 
                                                                     static_cast<double>(segment.nDoF()),
                                                                     std::move(holes));

        Amg::MatrixX covMatrix(4, 4);
        covMatrix.setIdentity();
        using namespace MuonR4::SegmentFit;
        covMatrix(Trk::locX, Trk::locX) = segment.covariance()(toInt(ParamDefs::x0), toInt(ParamDefs::x0));
        covMatrix(Trk::locY, Trk::locY) = segment.covariance()(toInt(ParamDefs::y0), toInt(ParamDefs::y0));

        covMatrix(Trk::phi0, Trk::phi0)   = segment.covariance()(toInt(ParamDefs::phi), toInt(ParamDefs::phi));
        covMatrix(Trk::theta, Trk::theta) = segment.covariance()(toInt(ParamDefs::theta), toInt(ParamDefs::theta));

        auto legacySeg = std::make_unique<Muon::MuonSegment>(Amg::Vector2D::Zero(), std::move(segDir),
                                                             std::move(covMatrix), segSurf.release(),
                                                             std::move(measurements), fitQuality.release());

        ATH_MSG_VERBOSE(m_printer->print(*legacySeg)<<", pos: "<<Amg::toString(legacySeg->globalPosition())<<" "
                     <<Amg::toString(legacySeg->globalDirection())<<std::endl<<m_printer->print(legacySeg->containedMeasurements()));
        outContainer.push_back(std::move(legacySeg));
        return StatusCode::SUCCESS;
    }
}
