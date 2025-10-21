/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "MlHitDumperAlg.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"


#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

#include <unordered_map>

namespace MuonR4{
    StatusCode MlHitDumperAlg::initialize() {
        ATH_CHECK(m_spacePointKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());

        ATH_CHECK(m_truthSegKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        m_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_tree, MuonVal::EventInfoBranch::isMC));

        
        m_spCollection = std::make_unique<MuonValR4::SpacePointTesterModule>(m_tree, m_spacePointKeys[0].key(), msgLevel());
        m_tree.addBranch(m_spCollection);
        m_muonP4 = std::make_unique<MuonValR4::IParticleFourMomBranch>(m_tree, "truthMuon");
        m_tree.addBranch(m_muonP4);
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MlHitDumperAlg::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const ActsTrk::GeometryContext* gctx{};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        std::unordered_map<const xAOD::MuonSimHit*, const xAOD::TruthParticle*> hitPartMap{};
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegKey, ctx));
        for (const xAOD::MuonSegment* segment : *truthSegs) {
            const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*segment);
            for (const xAOD::MuonSimHit* hit : getMatchingSimHits(*segment)) {
                hitPartMap[hit] = truthPart;
            }
        }

        for (const SG::ReadHandleKey<SpacePointContainer>& key : m_spacePointKeys){
            const SpacePointContainer* spCont{nullptr};
            ATH_CHECK(SG::get(spCont,key, ctx));
            for (const SpacePointBucket* bucket : *spCont) {
                m_spCollection->push_back(*bucket);
                for (const auto& sp : *bucket) {
                    const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*sp->primaryMeasurement());
                    m_muonP4->push_back(hitPartMap[simHit]);
                    m_truthLink[fillSpacePoint(*gctx, *sp)] = m_muonP4->find(hitPartMap[simHit]);
                }
            }
        }
        ATH_CHECK(m_tree.fill(ctx));

        return StatusCode::SUCCESS;
    }
    std::size_t MlHitDumperAlg::fillSpacePoint(const ActsTrk::GeometryContext& gctx, const SpacePoint& sp){
        const Amg::Transform3D& lToGlob = sp.msSector()->localToGlobalTrans(gctx);
        std::size_t idx = m_spCollection->push_back(sp);
        m_spGlobPos.set(lToGlob * sp.localPosition(), idx);

        switch (sp.type()) {
            using enum xAOD::UncalibMeasType;
            case MdtDriftCircleType:{
                const auto* prd = static_cast<const xAOD::MdtDriftCircle*>(sp.primaryMeasurement());
                const auto* re = prd->readoutElement();
                m_spGlobEdgeLow.set(re->readOutPos(gctx, prd->measurementHash()), idx);
                m_spGlobEdgeHigh.set(re->highVoltPos(gctx, prd->measurementHash()), idx);
                m_spReadoutSide[idx] = re->getParameters().readoutSide;
                m_spTime[idx] = prd->tdc();
                break; 
            }
            case RpcStripType: {
                const auto* prd = static_cast<const xAOD::RpcMeasurement*>(sp.primaryMeasurement());
                const auto* re = prd->readoutElement();
                m_spGlobEdgeLow.set(re->rightStripEdge(gctx, prd->measurementHash()), idx);
                m_spGlobEdgeHigh.set(re->leftStripEdge(gctx, prd->measurementHash()), idx);
                m_spReadoutSide[idx] = 1.;
                m_spTime[idx] = prd->time();
                break;
            }
            case TgcStripType: {
                const auto* prd = static_cast<const xAOD::TgcStrip*>(sp.primaryMeasurement());
                const auto* re = prd->readoutElement();
                const MuonGMR4::StripDesign& design = re->sensorLayout(prd->layerHash())->design();
                const double halfLength = design.stripLength(prd->channelNumber());
                m_spReadoutSide[idx] = 1.;
                m_spGlobEdgeLow.set( lToGlob*(sp.localPosition() -halfLength * sp.sensorDirection()) , idx);
                m_spGlobEdgeHigh.set(lToGlob*(sp.localPosition() +halfLength * sp.sensorDirection()) , idx);
                m_spTime[idx] = prd->bcBitMap();
                break;
            }
            case MMClusterType: {
                const auto* prd = static_cast<const xAOD::MMCluster*>(sp.primaryMeasurement());
                const auto* re = prd->readoutElement();
                m_spGlobEdgeLow.set(re->rightStripEdge(gctx, prd->measurementHash()), idx);
                m_spGlobEdgeHigh.set(re->leftStripEdge(gctx, prd->measurementHash()), idx);
                m_spReadoutSide[idx] = re->readoutSide(prd->measurementHash());
                m_spTime[idx] = prd->time();
                break;
            }
            case sTgcStripType: {
                const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(sp.primaryMeasurement());
                const auto* re = prd->readoutElement();
                m_spReadoutSide[idx] = 1.;
                m_spTime[idx] = prd->time();
                switch (prd->channelType()) {
                    using enum sTgcIdHelper::sTgcChannelTypes;
                    case Pad:{
                        double xLow{std::numeric_limits<double>::max()}, 
                               xHigh{-std::numeric_limits<double>::max()},
                               yLow{std::numeric_limits<double>::max()}, 
                               yHigh{-std::numeric_limits<double>::max()};
                        for (const Amg::Vector2D& corner : re->localPadCorners(prd->measurementHash())) {
                            xLow = std::min(xLow, corner.x());
                            yLow = std::min(yLow, corner.y());
                            xHigh = std::max(xHigh, corner.x());
                            yHigh = std::max(yHigh, corner.y());  
                        }
                        const Amg::Transform3D& padTrf{re->localToGlobalTrans(gctx, prd->layerHash())};
                        m_spGlobEdgeLow.set(padTrf * Amg::Vector3D{xLow,yLow, 0.}, idx);
                        m_spGlobEdgeHigh.set(padTrf * Amg::Vector3D{xHigh,yHigh, 0.}, idx);
                        break;
                    }
                    case Strip:
                    case Wire: {
                        const double halfLength = 0.5* re->stripLength(prd->measurementHash());
                        m_spGlobEdgeLow.set( lToGlob*(sp.localPosition() -halfLength * sp.sensorDirection()) , idx);
                        m_spGlobEdgeHigh.set(lToGlob*(sp.localPosition() +halfLength * sp.sensorDirection()) , idx);
                        break;
                    }
                }
                break;
            }
            default: break;
        }
        return idx;
    }
    StatusCode MlHitDumperAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}