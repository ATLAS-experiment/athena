/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSegmentCnvAlg.h"

#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripAuxContainer.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "Acts/Utilities/Enumerate.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Definitions/Units.hpp"

using namespace Acts::UnitLiterals;
namespace MuonR4{
    using namespace SegmentFit;
    /** @brief Abrivation of the link to the reco segment container  */
    using SegLink_t = ElementLink<MuonR4::SegmentContainer>;
    /** @brief Abrivation to call an uncalibrated measurement container */
    using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
    /** @brief Abrivation to call the link to an element inside an 
     *         uncalibrated measurement container */
    using PrdLink_t = ElementLink<PrdCont_t>;
    /** @brief Abrivation of a collection of Prd links */
    using PrdLinkVec_t = std::vector<PrdLink_t>;
    /** @brief Abrivation of the decorated local segment parameters */
    using SegPars_t = xAOD::MeasVector<Acts::toUnderlying(ParamDefs::nPars)>;

    StatusCode xAODSegmentCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_localSegParKey.initialize());
        ATH_CHECK(m_parentSegKey.initialize());
        ATH_CHECK(m_combMeasKey.initialize());
        ATH_CHECK(m_prdStateKey.initialize());
        ATH_CHECK(m_auxMeasProv.initialize(m_writeKey.key(), m_convertBeamSpot));
        return StatusCode::SUCCESS;
    }
    StatusCode xAODSegmentCnvAlg::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        SG::WriteHandle outContainer{m_writeKey, ctx};
        ATH_CHECK(outContainer.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                      std::make_unique<xAOD::MuonSegmentAuxContainer>()));
        
        SG::WriteHandle prdCombContainer{m_combMeasKey, ctx};
        ATH_CHECK(prdCombContainer.record(std::make_unique<xAOD::CombinedMuonStripContainer>(),
                                          std::make_unique<xAOD::CombinedMuonStripAuxContainer>()));
        
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegLink_t> dec_parentLink{m_parentSegKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars_t> dec_locPars{m_localSegParKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, PrdLinkVec_t> dec_prdLinks{m_prdLinkKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, std::vector<char>> dec_prdStates{m_prdStateKey, ctx};

        using State = CalibratedSpacePoint::State;
        // Cache all measurements that can be combined to two measurements in a single gas gap
        std::vector< std::tuple<const xAOD::MuonMeasurement*, State, std::size_t>> combineMap{};
        std::vector< std::tuple<const xAOD::UncalibratedMeasurement*, State, std::size_t>> linkMap{};

        const xAOD::UncalibratedMeasurement* beamSpotMeas{};

        auto beamSpotMeasCreator = m_auxMeasProv.makeHandle(ctx, gctx->context());

        /** @brief Decorate the prd links onto the output muon segment. Eta & phi measurements are absorbed converted
         *         into a CombinedMuonStrip which is a source link linke object carrying a link to both prds. In this way,
         *         only one track state is generated later in the track fit from the two measurements. 
         *         Two assumptions are made for the linking
         *                - There's exclusivley one eta & one phi measurement @maximum on the segment
         *                - The measurements are sorted along the segment trajectory.  */
        auto decorateLinks = [&](const Segment& inSegment, xAOD::MuonSegment& outSegment) {
            PrdLinkVec_t& links = dec_prdLinks(outSegment);
            std::vector<char>& linkStates = dec_prdStates(outSegment);
            links.reserve(2*inSegment.measurements().size());
            linkStates.reserve(2*inSegment.measurements().size());

            /** @brief Combine the two prds from the space point to a combined muonstrip and link
             *         the latter to the segment. */
            auto combine = [this,&prdCombContainer](const xAOD::MuonMeasurement* m1, 
                                                    const xAOD::MuonMeasurement* m2) {
                auto cmbMeas = prdCombContainer->push_back(std::make_unique<xAOD::CombinedMuonStrip>());

                cmbMeas->setPrimaryStrip(m1);
                cmbMeas->setSecondaryStrip(m2);
                const Identifier id1{m1->identify()}, id2{m2->identify()};
                ATH_MSG_VERBOSE("Combine "<<m_idHelperSvc->toString(id1)
                                <<" & "<<m_idHelperSvc->toString(id2));
                if ((m1->type() != xAOD::UncalibMeasType::sTgcStripType || 
                     m_idHelperSvc->stgcIdHelper().channelType(id1) == 
                     m_idHelperSvc->stgcIdHelper().channelType(id2))&&
                    m_idHelperSvc->measuresPhi(id1) == m_idHelperSvc->measuresPhi(id2)) {
                    THROW_EXCEPTION("Cannot combine "<<m_idHelperSvc->toString(id1)
                                <<" & "<<m_idHelperSvc->toString(id2));
                }
                return cmbMeas;
            };
            // Loop over the measurements
            for (const auto& [segIdx, meas] : Acts::enumerate(inSegment.measurements())) {
                const SpacePoint* sp = meas->spacePoint();
                if (!sp) {
                    if (!m_convertBeamSpot) {
                        continue;
                    }
                    // Up to now, there's no variety on the beamspot across the segments
                    if (!beamSpotMeas) {
                        if (!beamSpotMeasCreator.ok()) {
                            ATH_MSG_ERROR("Cannot create a beamspot measurement");
                            return StatusCode::FAILURE;
                        }

                        const Amg::Vector3D beamSpot = inSegment.msSector()->localToGlobalTransform(*gctx) *
                                                       meas->localPosition();
                        AmgSymMatrix(2) covariance{AmgSymMatrix(2)::Identity()};
                        using CovIdx = SpacePoint::CovIdx;
                        using ProjectorType = xAOD::AuxiliaryMeasurement::ProjectorType;
                        covariance(0,0) = meas->covariance()[Acts::toUnderlying(CovIdx::etaCov)];
                        covariance(1,1) = meas->covariance()[Acts::toUnderlying(CovIdx::phiCov)];
                        /// Size of the bounds purely for visualization purposes
                        auto surf = Acts::Surface::makeShared<Acts::StrawSurface>(Amg::getTranslate3D(beamSpot),
                                            std::make_shared<Acts::LineBounds>(std::sqrt(covariance(0,0)), 20._m));

                        beamSpotMeas = beamSpotMeasCreator->newMeasurement<2>(surf, 
                                                ProjectorType::e2DimNoTime, covariance);
                        ATH_MSG_DEBUG("Created beamspot measurement "<<(*meas)<<", "
                                      <<surf->toString(gctx->context()));
                    }
                    linkMap.emplace_back(beamSpotMeas, meas->fitState(), segIdx);
                    continue;
                }
                switch (sp->type()) {
                     using enum xAOD::UncalibMeasType;
                     // Mdt &  micromegas are never combined
                     case MdtDriftCircleType:
                     case MMClusterType: {
                        linkMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);                        
                        break;
                    } case RpcStripType:
                      case TgcStripType:
                      case sTgcStripType: {
                        if (sp->primaryMeasurement() && sp->secondaryMeasurement()) {
                            if (sp->primaryMeasurement() != sp->secondaryMeasurement()) {
                                linkMap.emplace_back(combine(sp->primaryMeasurement(), 
                                                             sp->secondaryMeasurement()),
                                                    meas->fitState(), segIdx);
                            } else {  // BI - RPC measurements
                                linkMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);
                            }
                        } else {
                            /// It might be that the segment has anoher 1D-measurement 
                            /// in the same gas gap
                            ATH_MSG_VERBOSE("Append for later combination "<<(*meas));
                            combineMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);
                        }
                        break;
                    } default:
                        break;
                }
            }
            // Finally we need to check whether there're measurements left to combine
            for (std::size_t cmbIdx = 0; cmbIdx < combineMap.size(); ++cmbIdx){
                const xAOD::MuonMeasurement* m1{std::get<0>(combineMap[cmbIdx])};
                const State s1{std::get<1>(combineMap[cmbIdx])};
                const std::size_t segIdx1{std::get<2>(combineMap[cmbIdx])};
                ATH_MSG_VERBOSE("Find another measurement to combine with "
                                <<m_idHelperSvc->toString(m1->identify()));
                if (cmbIdx +1 < combineMap.size()){
                    const xAOD::MuonMeasurement* m2{std::get<0>(combineMap[cmbIdx +1])};
                    const State s2{std::get<1>(combineMap[cmbIdx+1])};
                    ATH_MSG_VERBOSE("Check whether "<<m_idHelperSvc->toString(m2->identify())
                                    <<" is a good candidate");
                    if (m1->type() == m2->type() && 
                        m1->identifierHash() == m2->identifierHash() && 
                        m1->layerHash() == m2->layerHash() &&
                        s1 == s2) {
                        /// The first measurement should always be the eta measurement 
                        ATH_MSG_VERBOSE("They match");
                        if (m1->measuresPhi()) {
                            linkMap.emplace_back(combine(m2, m1), s2, segIdx1);
                        } else {
                            linkMap.emplace_back(combine(m1, m2), s1, segIdx1);
                        }
                        ++cmbIdx; // skip the next measurement as it's absorbed here
                        continue;
                    }
                }
                ATH_MSG_VERBOSE("No match found");
                linkMap.emplace_back(m1, s1, segIdx1);
            }

            std::ranges::sort(linkMap, [](const auto& a, const auto& b){
                return std::get<2>(a) < std::get<2>(b);
            });

            for (const auto& [prd, state, segIdx]: linkMap) {
                links.emplace_back( 
                    *static_cast<const xAOD::UncalibratedMeasurementContainer*>(prd->container()), 
                    prd->index());
                linkStates.emplace_back(Acts::toUnderlying(state));
            }
            linkMap.clear();
            combineMap.clear();
            return StatusCode::SUCCESS;
        };

        for (const SG::ReadHandleKey<SegmentContainer>& key : m_readKeys) {
            const SegmentContainer* segmentContainer{nullptr};
            ATH_CHECK(SG::get(segmentContainer, key, ctx));
         
            /// Counter for the reco segment link
            unsigned recoSegIdx{0};
            outContainer->reserve(outContainer->size() + segmentContainer->size());
            for (const Segment* inSegment : *segmentContainer) {
                const MuonGMR4::SpectrometerSector* sector = inSegment->msSector();

                xAOD::MuonSegment* convertedSeg = outContainer->push_back(std::make_unique<xAOD::MuonSegment>());
                dec_parentLink(*convertedSeg) = SegLink_t{segmentContainer, recoSegIdx};
                ++recoSegIdx;
            
                const Amg::Vector3D& pos{inSegment->position()};
                const Amg::Vector3D& dir{inSegment->direction()};
                convertedSeg->setPosition(pos.x(), pos.y(), pos.z());
                convertedSeg->setDirection(dir.x(), dir.y(), dir.z());
            

                convertedSeg->setIdentifier(sector->sector(), sector->chamberIndex(), sector->side(), 
                                            xAOD::toTechnologyIndex(inSegment->summary().tech));
                convertedSeg->setFitQuality(inSegment->chi2(), inSegment->nDoF());
                convertedSeg->setNHits(inSegment->summary().nPrecHits, inSegment->summary().nPhiHits,
                                       inSegment->summary().nEtaTrigHits);
           
                using enum ParamDefs;
                convertedSeg->setT0Error(inSegment->segementT0(), 
                                         Amg::error(inSegment->covariance(), Acts::toUnderlying(t0)));

                SegPars_t& localPars{dec_locPars(*convertedSeg)};
                const Amg::Transform3D globToLoc{sector->globalToLocalTransform(*gctx)};            
                const Amg::Vector3D locPos{globToLoc * pos};
                const Amg::Vector3D locDir{globToLoc.linear() * dir};

                localPars[Acts::toUnderlying(x0)] = locPos.x();
                localPars[Acts::toUnderlying(y0)] = locPos.y();
                localPars[Acts::toUnderlying(theta)] = locDir.theta();
                localPars[Acts::toUnderlying(phi)] = locDir.phi();
                localPars[Acts::toUnderlying(t0)] = inSegment->segementT0();
                ATH_CHECK(decorateLinks(*inSegment, *convertedSeg));
            }
        }  
        return StatusCode::SUCCESS;
    }
}
