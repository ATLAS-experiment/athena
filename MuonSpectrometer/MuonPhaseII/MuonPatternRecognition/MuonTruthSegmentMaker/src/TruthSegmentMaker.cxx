/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthSegmentMaker.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODMuon/MuonSegmentAuxContainer.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"

#include <unordered_map>


namespace MuonR4{
    template <class ContainerType>
        StatusCode TruthSegmentMaker::retrieveContainer(const EventContext& ctx, 
                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                        const ContainerType*& contToPush) const {
        contToPush = nullptr;
        if (key.empty()) {
            ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle<ContainerType> readHandle{key, ctx};
        ATH_CHECK(readHandle.isPresent());
        contToPush = readHandle.cptr();
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegmentMaker::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_eleLinkKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegmentMaker::execute(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{nullptr};
        
        
        using HitsPerParticle = std::unordered_map<HepMC::ConstGenParticlePtr, std::vector<const xAOD::MuonSimHit*>>;
        using HitCollector = std::unordered_map<const MuonGMR4::MuonChamber*, HitsPerParticle>;
        HitCollector hitCollector{};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));
        for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key : m_readKeys) {
            const xAOD::MuonSimHitContainer* simHits{nullptr};
            ATH_CHECK(retrieveContainer(ctx, key, simHits));        
            for (const xAOD::MuonSimHit* simHit : *simHits) {
                const MuonGMR4::MuonReadoutElement* reElement = m_detMgr->getReadoutElement(simHit->identify()); 
                const MuonGMR4::MuonChamber* id{reElement->getChamber()};
                auto genLink = simHit->genParticleLink();
                HepMC::ConstGenParticlePtr genParticle = nullptr; 
                if (genLink.isValid()){
                    genParticle = genLink.cptr(); 
                }
                /// skip empty truth matches for now
                if (!genParticle) continue;
                hitCollector[id][genParticle].push_back(simHit); 
            }
        }
        

        SG::WriteHandle<xAOD::MuonSegmentContainer> writeHandle{m_segmentKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                     std::make_unique<xAOD::MuonSegmentAuxContainer>()));
        
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, HitLinkVec> hitDecor{m_eleLinkKey, ctx};
        for (const auto& [chamber, collectedParts] : hitCollector) {
            const Amg::Transform3D& locToGlob{chamber->localToGlobalTrans(*gctx)};
            const Amg::Transform3D  globToLoc{locToGlob.inverse()};
            
            for (const auto& [particle, simHits]: collectedParts) {
                const xAOD::MuonSimHit* simHit = simHits.front(); 
                const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(simHit->identify());
                const IdentifierHash trfHash{reEle->detectorType() == ActsTrk::DetectorType::Mdt ? 
                                                    reEle->measurementHash(simHit->identify()) :
                                                    reEle->layerHash(simHit->identify())};
                
                const Amg::Transform3D toChamber = globToLoc * reEle->localToGlobalTrans(*gctx, trfHash);
                
                const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
                const Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(simHit->localDirection());

                /// Express the simulated hit in the center of the chamber
                const std::optional<double> lambda = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.);
                const Amg::Vector3D chamberPos = localPos + lambda.value_or(0.)*chamberDir;
                
                const Amg::Vector3D globPos = locToGlob * chamberPos;
                const Amg::Vector3D globDir = locToGlob.linear() * chamberDir;

                xAOD::MuonSegment* truthSegment = writeHandle->push_back(std::make_unique<xAOD::MuonSegment>());
                truthSegment->setPosition(globPos.x(), globPos.y(), globPos.z());
                truthSegment->setDirection(globDir.x(), globDir.y(), globDir.z());
                truthSegment->setT0Error(simHit->globalTime(), 0.);

                HitLinkVec associatedHits{};
                unsigned int nMdt{0}, nRpcEta{0}, nRpcPhi{0}, nTgcEta{0}, nTgcPhi{0};
                unsigned int nMm{0}, nStgcEta{0}, nStgcPhi{0};
                for (const xAOD::MuonSimHit* assocMe : simHits) {
                    const MuonGMR4::MuonReadoutElement* assocRE = m_detMgr->getReadoutElement(assocMe->identify());
                    switch (assocRE->detectorType()) {
                        case ActsTrk::DetectorType::Mdt:
                            ++nMdt;
                            break;
                        case ActsTrk::DetectorType::Rpc: {
                            auto castRE{static_cast<const MuonGMR4::RpcReadoutElement*>(assocRE)};
                            if (castRE->nEtaStrips()) ++nRpcEta;
                            if (castRE->nPhiStrips()) ++nRpcPhi;
                            break;
                        } case ActsTrk::DetectorType::Tgc: {
                            auto castRE{static_cast<const MuonGMR4::TgcReadoutElement*>(assocRE)};
                            const int gasGap = m_idHelperSvc->gasGap(assocMe->identify());
                            if (castRE->numStrips(gasGap)) ++nTgcPhi;
                            if (castRE->numWireGangs(gasGap)) ++nTgcEta;
                            break;
                        } case ActsTrk::DetectorType::sTgc:{
                            ++nStgcEta;
                            ++nStgcPhi;
                            break;
                        } case ActsTrk::DetectorType::Mm:{
                            ++nMm;
                            break;
                        }
                        default:
                            ATH_MSG_WARNING("Csc are not defined "<<m_idHelperSvc->toString(simHit->identify()));
                    }
                    ElementLink<xAOD::MuonSimHitContainer> link{*static_cast<const xAOD::MuonSimHitContainer*>(simHit->container()), 
                                                                simHit->index()};
                    associatedHits.push_back(std::move(link));
                }
                truthSegment->setNHits(nMdt + nMm + nStgcEta, nTgcPhi + nRpcPhi + nStgcPhi, nTgcEta + nRpcEta);
                hitDecor(*truthSegment) = std::move(associatedHits);
            }
        }
        return StatusCode::SUCCESS;
    }
}
