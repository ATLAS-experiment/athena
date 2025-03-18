/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include "MuonPatternEvent/MuonHoughDefs.h"

#include "TruthUtils/HepMCHelpers.h"

#include "GaudiKernel/PhysicalConstants.h"

#include <unordered_map>

namespace{
    constexpr double c_inv = 1./Gaudi::Units::c_light;
}

namespace MuonR4{
    using namespace SegmentFit;
    template <class ContainerType>
        StatusCode TruthSegmentMaker::retrieveContainer(const EventContext& ctx, 
                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                        const ContainerType*& contToPush) const {
        contToPush = nullptr;
        if (key.empty()) {
            ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle readHandle{key, ctx};
        ATH_CHECK(readHandle.isPresent());
        contToPush = readHandle.cptr();
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegmentMaker::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKeys.initialize());
        if (m_readKeys.empty()){
            ATH_MSG_ERROR("No simulated hit containers have been parsed to build the segments from ");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_eleLinkKey.initialize());
        ATH_CHECK(m_ptKey.initialize());

        ATH_CHECK(m_locParKey.initialize());
        ATH_CHECK(m_qKey.initialize());

        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    Amg::Transform3D TruthSegmentMaker::toChamber(const ActsGeometryContext& gctx,
                                                  const Identifier& chanId) const {
        const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(chanId);
        const IdentifierHash trfHash{reEle->detectorType() == ActsTrk::DetectorType::Mdt ?
                                                    reEle->measurementHash(chanId) :
                                                    reEle->layerHash(chanId)};
        return reEle->msSector()->globalToLocalTrans(gctx) * reEle->localToGlobalTrans(gctx, trfHash);

    }
    StatusCode TruthSegmentMaker::execute(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));
        
        
        using HitsPerParticle = std::unordered_map<HepMC::ConstGenParticlePtr, std::vector<const xAOD::MuonSimHit*>>;
        using HitCollector = std::unordered_map<const MuonGMR4::SpectrometerSector*, HitsPerParticle>;
        HitCollector hitCollector{};

        for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key : m_readKeys) {
            const xAOD::MuonSimHitContainer* simHits{nullptr};
            ATH_CHECK(retrieveContainer(ctx, key, simHits));        
            for (const xAOD::MuonSimHit* simHit : *simHits) {
                const MuonGMR4::MuonReadoutElement* reElement = m_detMgr->getReadoutElement(simHit->identify()); 
                const MuonGMR4::SpectrometerSector* id{reElement->msSector()};
                auto genLink = simHit->genParticleLink();
                HepMC::ConstGenParticlePtr genParticle = nullptr; 
                if (genLink.isValid()){
                    genParticle = genLink.cptr(); 
                }
                /// skip empty truth matches for now
                if (!genParticle || (m_useOnlyMuonHits && !MC::isMuon(simHit))) {
                    ATH_MSG_VERBOSE("Skip hit "<<m_idHelperSvc->toString(simHit->identify())<<
                                  " pdgId: "<<simHit->pdgId()<<", energy: "<<simHit->kineticEnergy());
                    continue;
                }
                hitCollector[id][genParticle].push_back(simHit); 
            }
        } 

        SG::WriteHandle writeHandle{m_segmentKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                     std::make_unique<xAOD::MuonSegmentAuxContainer>()));
        
        using HitLinkVec = std::vector<ElementLink<xAOD::MuonSimHitContainer>>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, HitLinkVec> hitDecor{m_eleLinkKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, float> ptDecor{m_ptKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, float> qDecor{m_qKey, ctx};
        using SegPars = xAOD::MeasVector<toInt(ParamDefs::nPars)>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars> parDecor{m_locParKey, ctx};
        for (auto& [chamber, collectedParts] : hitCollector) {
            const Amg::Transform3D& locToGlob{chamber->localToGlobalTrans(*gctx)};
            
            for (auto& [particle, simHits]: collectedParts) {

                /* Take the hit that's closest to the chamber centre as reference */
                std::ranges::stable_sort(simHits,[gctx,this](const xAOD::MuonSimHit*a, const xAOD::MuonSimHit*b){
                    return std::abs((toChamber(*gctx, a->identify())* xAOD::toEigen(a->localPosition())).z()) <
                           std::abs((toChamber(*gctx, b->identify())* xAOD::toEigen(b->localPosition())).z());
                });
                const xAOD::MuonSimHit* simHit = simHits.front(); 
                const Identifier segId{simHit->identify()};
                
                const Amg::Transform3D inChamb = toChamber(*gctx, segId);
                const Amg::Vector3D localPos{inChamb * xAOD::toEigen(simHit->localPosition())};
                const Amg::Vector3D chamberDir = inChamb.linear() * xAOD::toEigen(simHit->localDirection());

                /// Express the simulated hit in the center of the chamber
                const double distance = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.).value_or(0.);
                const Amg::Vector3D chamberPos = localPos + distance*chamberDir;
                
                const Amg::Vector3D globPos = locToGlob * chamberPos;
                const Amg::Vector3D globDir = locToGlob.linear() * chamberDir;
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
                    ElementLink<xAOD::MuonSimHitContainer> link{*static_cast<const xAOD::MuonSimHitContainer*>(assocMe->container()), 
                                                                assocMe->index()};
                    associatedHits.push_back(std::move(link));
                }
                int nPrecisionHits = nMdt + nMm + nStgcEta;
                int nPhiLayers     = nTgcPhi + nRpcPhi + nStgcPhi;
                // if nMdt + nMm + nStgcEta < 3, do not create a segment
                if (nPrecisionHits < 3) continue;

                xAOD::MuonSegment* truthSegment = writeHandle->push_back(std::make_unique<xAOD::MuonSegment>());
                ptDecor(*truthSegment) = particle->momentum().perp();
                qDecor(*truthSegment) = particle->pdg_id() > 0 ? -1 : 1;
                SegPars& locPars{parDecor(*truthSegment)};
                locPars[toInt(ParamDefs::x0)] = chamberPos.x();
                locPars[toInt(ParamDefs::y0)] = chamberPos.y();
                locPars[toInt(ParamDefs::time)] = simHit->globalTime() + distance *c_inv /simHit->beta();
                locPars[toInt(ParamDefs::theta)] = chamberDir.theta();
                locPars[toInt(ParamDefs::phi)]   = chamberDir.phi();
                truthSegment->setPosition(globPos.x(), globPos.y(), globPos.z());
                truthSegment->setDirection(globDir.x(), globDir.y(), globDir.z());
                truthSegment->setT0Error(locPars[toInt(ParamDefs::time)], 0.);
                
                truthSegment->setNHits(nPrecisionHits, nPhiLayers, nTgcEta + nRpcEta);
                truthSegment->setIdentifier(m_idHelperSvc->sector(segId), 
                                            m_idHelperSvc->chamberIndex(segId),
                                            m_idHelperSvc->stationEta(segId),
                                            m_idHelperSvc->technologyIndex(segId));
                // adding chi2 and ndof (nHits - 5 for 2 position, 2 direction and 1 time)
                if (nPhiLayers == 0){
                    truthSegment->setFitQuality(0, (nPrecisionHits + nTgcEta + nRpcEta - 3));
                } else {
                    truthSegment->setFitQuality(0, (nPrecisionHits + nPhiLayers + nTgcEta + nRpcEta - 5));
                }
                hitDecor(*truthSegment) = std::move(associatedHits);
            }
        }
        return StatusCode::SUCCESS;
    }
}
