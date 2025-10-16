/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSegmentCnvAlg.h"

#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripAuxContainer.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"



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
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_localSegParKey.initialize());
        ATH_CHECK(m_parentSegKey.initialize());
        ATH_CHECK(m_combMeasKey.initialize());
        ATH_CHECK(m_prdStateKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode xAODSegmentCnvAlg::execute(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{nullptr};
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
        static std::atomic<unsigned> sTgcWarnings{0};
        bool printWarning{sTgcWarnings < 100};
        using State = CalibratedSpacePoint::State;
        std::vector<std::tuple<const xAOD::UncalibratedMeasurement*, State>> combineMap{};
        combineMap.reserve(10);
        /** @brief Decorate the prd links onto the output muon segment. Eta & phi measurements are absorbed converted
         *         into a CombinedMuonStrip which is a source link linke object carrying a link to both prds. In this way,
         *         only one track state is generated later in the track fit from the two measurements. 
         *         Two assumptions are made for the linking
         *                - There's exclusivley one eta & one phi measurement @maximum on the segment
         *                - The measurements are sorted along the segment trajectory.  */
        auto decorateLinks = [this, &dec_prdLinks, &prdCombContainer, &printWarning,
                              &combineMap, & dec_prdStates](const Segment& inSegment, xAOD::MuonSegment& outSegment) {
            PrdLinkVec_t& links = dec_prdLinks(outSegment);
            std::vector<char>& linkStates = dec_prdStates(outSegment);
            links.reserve(2*inSegment.measurements().size());
            linkStates.reserve(2*inSegment.measurements().size());
            /** @brief Transform the uncalibrated measurement pointer into a PrdLink & 
             *         append it to the list of decorated links */

            auto appendLink = [&links, &linkStates](const xAOD::UncalibratedMeasurement* prd, const State st) {
                if (!prd) {
                    return;
                }
                linkStates.emplace_back(Acts::toUnderlying(st));
                links.emplace_back(*static_cast<const xAOD::UncalibratedMeasurementContainer*>(prd->container()),
                                   prd->index());
            };
            /** @brief Combine the two prds from the space point to a combined muonstrip and link
             *         the latter to the segment. */
            auto combine = [&prdCombContainer,&appendLink](const xAOD::UncalibratedMeasurement* m1, 
                                                           const xAOD::UncalibratedMeasurement* m2, 
                                                           const State st) {
                auto cmbMeas = prdCombContainer->push_back(std::make_unique<xAOD::CombinedMuonStrip>());

                cmbMeas->setPrimaryStrip(m1);
                cmbMeas->setSecondaryStrip(m2);
                appendLink(cmbMeas, st);
            };
            for (const auto& meas : inSegment.measurements()) {
                const SpacePoint* sp = meas->spacePoint();
                if (!sp) {
                    continue;
                }
                switch (sp->type()) {
                     using enum xAOD::UncalibMeasType;
                     // Mdt &  micromegas are never combined
                     case MdtDriftCircleType:
                     case MMClusterType: {
                        appendLink(sp->primaryMeasurement(), meas->fitState());
                        break;
                    } case RpcStripType:
                      case TgcStripType: {
                        if (sp->primaryMeasurement() && sp->secondaryMeasurement()) {
                            combine(sp->primaryMeasurement(), sp->secondaryMeasurement(), meas->fitState());
                        } else if (sp->dimension() == 2) { // BI - RPC measurements
                            appendLink(sp->primaryMeasurement(), meas->fitState());
                        } else {
                            /// It might be that the segment has anoher 1D-measurement 
                            /// in the same gas gap
                            combineMap.emplace_back(sp->primaryMeasurement(), meas->fitState());
                        }
                        break;
                    } case sTgcStripType:{
                        /// @TODO Fix the combination of the three measurements
                        appendLink(sp->primaryMeasurement(), meas->fitState());
                        appendLink(sp->secondaryMeasurement(), meas->fitState());
                        /// Remember the user once per event
                        if (printWarning) {
                            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Please implement a stgc combination schema");
                            printWarning = false;
                        }
                    } default:
                        break;
                }
            }
            // Finally we need to check whether there're measurements left to combine
            for (std::size_t cmbIdx = 0; cmbIdx < combineMap.size(); ++cmbIdx){
                const xAOD::UncalibratedMeasurement* m1{std::get<0>(combineMap[cmbIdx])};
                ATH_MSG_VERBOSE("Find another measurement to combine with "
                                <<m_idHelperSvc->toString(xAOD::identify(m1)));
                if (cmbIdx +1 < combineMap.size()){
                    const xAOD::UncalibratedMeasurement* m2{std::get<0>(combineMap[cmbIdx +1])};
                    ATH_MSG_VERBOSE("Check whether "<<m_idHelperSvc->toString(xAOD::identify(m2))
                                    <<" is a good candidate");
                    if (m1->type() == m2->type() && 
                        m1->identifierHash() == m2->identifierHash()
                        && xAOD::layerHash(m1)  == xAOD::layerHash(m2)) {
                        /// The first measurement should always be the eta measurement 
                        ATH_MSG_VERBOSE("They match");
                        if (m_idHelperSvc->measuresPhi(xAOD::identify(m1))) {
                            combine(m2, m1, std::get<1>(combineMap[cmbIdx+1]));
                        } else {
                            combine(m1, m2, std::get<1>(combineMap[cmbIdx]));
                        }
                        ++cmbIdx; // skip the next measurement as it's absorbed here
                        continue;
                    }
                }
                ATH_MSG_VERBOSE("No match found");
                appendLink(m1, std::get<1>(combineMap[cmbIdx]));
            }
            combineMap.clear();
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
                                            toTechIdx(inSegment->summary().tech));
                convertedSeg->setFitQuality(inSegment->chi2(), inSegment->nDoF());
                convertedSeg->setNHits(inSegment->summary().nPrecHits, inSegment->summary().nPhiHits,
                                       inSegment->summary().nEtaTrigHits);
           
                using enum ParamDefs;
                convertedSeg->setT0Error(inSegment->segementT0(), 
                                         Amg::error(inSegment->covariance(), Acts::toUnderlying(t0)));

                SegPars_t& localPars{dec_locPars(*convertedSeg)};
                const Amg::Transform3D globToLoc{sector->globalToLocalTrans(*gctx)};            
                const Amg::Vector3D locPos{globToLoc * pos};
                const Amg::Vector3D locDir{globToLoc.linear() * dir};

                localPars[Acts::toUnderlying(x0)] = locPos.x();
                localPars[Acts::toUnderlying(y0)] = locPos.y();
                localPars[Acts::toUnderlying(theta)] = locDir.theta();
                localPars[Acts::toUnderlying(phi)] = locDir.phi();
                localPars[Acts::toUnderlying(t0)] = inSegment->segementT0();
                decorateLinks(*inSegment, *convertedSeg);
            }
        }  
        
        if (!printWarning) {
            sTgcWarnings = sTgcWarnings + 1;
        }
        return StatusCode::SUCCESS;
    }
}
