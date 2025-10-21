/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthSegmentMaker.h"


#include "xAODMuon/MuonSegmentAuxContainer.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"



#include "TruthUtils/HepMCHelpers.h"

#include "GaudiKernel/PhysicalConstants.h"

#include <unordered_map>

namespace{
    constexpr double c_inv = 1./Gaudi::Units::c_light;
}

namespace MuonR4{
    using namespace SegmentFit;
   
    StatusCode TruthSegmentMaker::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKeys.initialize());
        if (m_readKeys.empty()){
            ATH_MSG_ERROR("No simulated hit containers have been parsed to build the segments from ");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_mdtCalibKey.initialize(m_idHelperSvc->hasMDT()));
        ATH_CHECK(m_nswUncertKey.initialize(m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC()));
        
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_eleLinkKey.initialize());
        ATH_CHECK(m_ptKey.initialize());

        ATH_CHECK(m_locParKey.initialize());
        ATH_CHECK(m_qKey.initialize());

        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    float TruthSegmentMaker::hitUncertainty(const EventContext& ctx, const xAOD::MuonSimHit& hit) const {
        const Identifier hitId{hit.identify()};
        switch (const auto techIdx = m_idHelperSvc->technologyIndex(hitId)) {
            using enum Muon::MuonStationIndex::TechnologyIndex;
            case MDT:  {
                const MuonCalib::MdtCalibDataContainer* calibCont{nullptr};
                if (!SG::get(calibCont, m_mdtCalibKey, ctx).isSuccess()) {
                    THROW_EXCEPTION("Failed to retrieve Mdt calib constants");
                }
                const auto& rtCalib{calibCont->getCalibData(hitId, msgStream())->rtRelation};
                const double driftTime = rtCalib->tr()->driftTime(hit.localPosition().perp()).value_or(rtCalib->tr()->maxRadius());
                return rtCalib->rtRes()->resolution(driftTime);
            } case RPC: {
                const auto* re = m_detMgr->getRpcReadoutElement(hitId);
                return re->stripEtaPitch() / std::sqrt(12.);
            } case TGC: {
                const auto* re = m_detMgr->getTgcReadoutElement(hitId);
                const IdentifierHash measHash = re->measurementHash(hitId);
                const auto& design = re->wireGangLayout(measHash);
                return design.stripPitch()  / std::sqrt(12.);
            } case STGC:
              case MM: {
                const NswErrorCalibData* errorCalibDB{nullptr};
                if (!SG::get(errorCalibDB, m_nswUncertKey, ctx).isSuccess()) {
                    THROW_EXCEPTION("Failed to retrieve the STGC calibration constants");
                }
                NswErrorCalibData::Input errorCalibInput{};
                errorCalibInput.stripId= hitId;
                errorCalibInput.locTheta = M_PI - hit.localDirection().theta();
                if (techIdx == STGC) {
                    errorCalibInput.clusterAuthor = 3; // centroid
                } else {
                    errorCalibInput.clusterAuthor=66; // cluster time projection method
                }
                return errorCalibDB->clusterUncertainty(errorCalibInput);
            } 
            default:
                break;
        }
        return 0.;
    }
    float TruthSegmentMaker::muonPt(const xAOD::MuonSimHit& hit, const Amg::Vector3D& globDir) const{
        const auto& link = hit.genParticleLink();
        if (link.isValid()) {
            return link->momentum().perp();
        }
        const float e{hit.kineticEnergy()},m{hit.mass()};
        return std::sqrt(std::max(e*e - m*m, 0.f)) * std::sin(globDir.theta());
    }
    Amg::Transform3D TruthSegmentMaker::toChamber(const ActsTrk::GeometryContext& gctx,
                                                  const Identifier& chanId) const {
        const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(chanId);
        const IdentifierHash trfHash{reEle->detectorType() == ActsTrk::DetectorType::Mdt ?
                                                    reEle->measurementHash(chanId) :
                                                    reEle->layerHash(chanId)};
        return reEle->msSector()->globalToLocalTrans(gctx) * reEle->localToGlobalTrans(gctx, trfHash);

    }

    void TruthSegmentMaker::buildSegmentsFromBkg(const EventContext& ctx,
                                                 const Amg::Transform3D& locToGlob,
                                                 const SimHitVec_t& simHits,
                                                 WriteDecorHolder& out)const {
        
        ATH_MSG_VERBOSE("Assemble segments from "<<simHits.size()<<" background hits.");
        std::vector<char> alreadyUsed(simHits.size(), 0);
        for (std::size_t h = 0 ;h < simHits.size(); ++h) {
            if (alreadyUsed[h]) {
                continue;
            }
            const auto& [refHit, refPos, refDir] = simHits[h];
            /// Closest beam spot approach
            ATH_MSG_VERBOSE("Try to find other hits on trajectory given by "<<m_idHelperSvc->toString(refHit->identify())
            <<", "<<Amg::toString(refPos)<<", "<<Amg::toString(refDir)<<", E: "<<refHit->kineticEnergy() / Gaudi::Units::GeV<<" [GeV] "
            <<", pt: "<<muonPt(*refHit, locToGlob.linear()* refDir)/ Gaudi::Units::GeV <<" [GeV].");
            std::vector<std::size_t> indicesOnSeg{h};
            for (std::size_t h1 = h +1; h1 < simHits.size(); ++h1) {
                if (alreadyUsed[h1]) {
                    continue;
                }
                const auto&[testHit, testPos, testDir] = simHits[h1];

                /// First compare the energies
                if (std::abs(refHit->kineticEnergy() - testHit->kineticEnergy()) > m_pileUpHitELoss) {
                    continue;
                }
                if (MC::charge(refHit) != MC::charge(testHit)){
                    continue;
                }
                const double angleDir = Amg::angle(testDir, refDir);
                const double hitSep = std::abs(Amg::signedDistance(refPos, refDir, testPos,testDir));
                ATH_MSG_VERBOSE("Test hit "<<m_idHelperSvc->toString(testHit->identify())<<", "
                              <<Amg::toString(testPos)<<", testDir: "<<Amg::toString(testDir)
                              <<", E: "<<testHit->kineticEnergy() / (Gaudi::Units::GeV)<<", angle: "
                              <<(angleDir / Gaudi::Units::deg) <<", distance: "<<hitSep<<".");

                if (std::abs(angleDir) > m_pileUpHitAngleCone || hitSep > m_pileUpHitDistance) {
                    continue;
                }
                indicesOnSeg.push_back(h1);
            }
            /// Calculate the approach to the cylinder
            const Amg::Vector3D globPos = locToGlob * refPos;
            const Amg::Vector3D globDir = locToGlob.linear() * refDir;
            const Amg::Vector3D perigee = globPos + Amg::intersect<3>(Amg::Vector3D::Zero(), Amg::Vector3D::UnitZ(),
                                                                          globPos, globDir).value_or(0.) * globDir;
            ATH_MSG_VERBOSE("Found "<<indicesOnSeg.size()<<" matching hits. Closest perigee "
                    <<Amg::toString(perigee)<<", "<<m_idCylinderR<<", "<<m_idCylinderHalfZ);
            SimHitVec_t hitsOnSeg{};
            std::ranges::transform(indicesOnSeg,
                    std::back_inserter(hitsOnSeg), 
                    [&simHits](const auto idx) {
                        return simHits[idx];
                    });
            if (perigee.perp() > m_idCylinderR || std::abs(perigee.z()) > m_idCylinderHalfZ  ||
                constructSegmentFromHits(ctx, locToGlob, hitsOnSeg, out)){
                std::ranges::for_each(indicesOnSeg,
                        [&alreadyUsed](const auto idx) {
                             alreadyUsed[idx] = 1;
                        });
            }
        }
    }
    xAOD::MuonSegment* 
        TruthSegmentMaker::constructSegmentFromHits(const EventContext& ctx,
                                                    const Amg::Transform3D& locToGlob,
                                                    const SimHitVec_t& simHits,
                                                    WriteDecorHolder& out) const {
        /// Find the hit which is closest to the refernce plane to express the segment
        /// at the centre plane
        auto refHit_itr = std::ranges::min_element(simHits, 
                [](const HitPosTuple_t& a, const HitPosTuple_t& b){
                    return std::abs(std::get<1>(a).z()) <std::abs(std::get<1>(b).z());
                });
                
        const auto [simHit, localPos, chamberDir] = (*refHit_itr);
        ATH_MSG_VERBOSE("Create segement from hit: "<<m_idHelperSvc->toString(simHit->identify())<<
                            " pdgId: "<<simHit->pdgId()<<", energy: "<<simHit->kineticEnergy()
                            <<", genParticle: "<<simHit->genParticleLink().cptr());
        const Identifier segId{simHit->identify()};

        /// Express the simulated hit in the center of the chamber
        const double distance = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.).value_or(0.);
        const Amg::Vector3D chamberPos = localPos + distance*chamberDir;
        
        const Amg::Vector3D globPos = locToGlob * chamberPos;
        const Amg::Vector3D globDir = locToGlob.linear() * chamberDir;
        HitLinkVec_t associatedHits{};
        unsigned nMdt{0}, nRpcEta{0}, nRpcPhi{0}, nTgcEta{0}, nTgcPhi{0};
        unsigned nMm{0}, nStgcEta{0}, nStgcPhi{0};
        /// Associate the hits to the segment and calculate the expected hits
        float chi2{0.f};
        for (const auto& [assocMe, pos, dir] : simHits) {
            chi2 += std::pow(Amg::signedDistance(pos,dir, chamberPos, chamberDir) / hitUncertainty(ctx,*assocMe), 2);
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
                    const IdentifierHash gapHash = assocRE->measurementHash(assocMe->identify());
                    if (castRE->numStrips(gapHash)){
                        ++nTgcPhi;
                    } 
                    if (castRE->numWireGangs(gapHash)) {
                        ++nTgcEta;
                    }
                    break;
                } case ActsTrk::DetectorType::sTgc:{
                    ++nStgcEta;
                    ++nStgcPhi;
                    break;
                } case ActsTrk::DetectorType::Mm:{
                    ++nMm;
                    break;
                } default:
                    ATH_MSG_WARNING("Csc are not defined "<<m_idHelperSvc->toString(simHit->identify()));
            }
            ATH_MSG_VERBOSE("Associate hit "<<m_idHelperSvc->toString(assocMe->identify())
                <<" pdgId: "<<assocMe->pdgId()<<", energy: "<<assocMe->kineticEnergy()
                <<", genParticle: "<<assocMe->genParticleLink().cptr()<<", beta: "<<simHit->beta()
                <<" global time: "<<simHit->globalTime()<<", pos: "<<Amg::toString(pos)
                <<", dir: "<<Amg::toString(dir));
            EleLink_t link{*static_cast<const xAOD::MuonSimHitContainer*>(assocMe->container()), assocMe->index()};
            associatedHits.push_back(std::move(link));
        }
        int nPrecisionHits = nMdt + nMm + nStgcEta;
        int nPhiLayers     = nTgcPhi + nRpcPhi + nStgcPhi;
        // if nMdt + nMm + nStgcEta < 3, do not create a segment
        if (nPrecisionHits < 3) {
            return nullptr;
        }

        xAOD::MuonSegment* truthSegment = out.segments.push_back(std::make_unique<xAOD::MuonSegment>());
        out.ptDecor(*truthSegment) = muonPt(*simHit, globDir);
        out.chargeDecor(*truthSegment) = MC::charge(simHit);
        SegPars_t& locPars{out.paramDecor(*truthSegment)};
        locPars[Acts::toUnderlying(ParamDefs::x0)] = chamberPos.x();
        locPars[Acts::toUnderlying(ParamDefs::y0)] = chamberPos.y();
        constexpr float betaLowLimit = 1.e-6;
        locPars[Acts::toUnderlying(ParamDefs::t0)] = simHit->globalTime() + distance *c_inv / std::max(simHit->beta(), betaLowLimit);
        locPars[Acts::toUnderlying(ParamDefs::theta)] = chamberDir.theta();
        locPars[Acts::toUnderlying(ParamDefs::phi)]   = chamberDir.phi();
        
        truthSegment->setPosition(globPos.x(), globPos.y(), globPos.z());
        truthSegment->setDirection(globDir.x(), globDir.y(), globDir.z());
        truthSegment->setT0Error(locPars[Acts::toUnderlying(ParamDefs::t0)], 0.);
                
        truthSegment->setNHits(nPrecisionHits, nPhiLayers, nTgcEta + nRpcEta);
        truthSegment->setIdentifier(m_idHelperSvc->sector(segId), 
                                    m_idHelperSvc->chamberIndex(segId),
                                    m_idHelperSvc->stationEta(segId),
                                    m_idHelperSvc->technologyIndex(segId));
        // adding chi2 and ndof (nHits - 5 for 2 position, 2 direction and 1 time)
        if (nPhiLayers == 0){
            truthSegment->setFitQuality(chi2, (nPrecisionHits + nTgcEta + nRpcEta - 3));
        } else {
            truthSegment->setFitQuality(chi2, (nPrecisionHits + nPhiLayers + nTgcEta + nRpcEta - 5));
        }
        out.hitLinkDecor(*truthSegment) = std::move(associatedHits);
        return truthSegment;                                                            
    }
                                        
    StatusCode TruthSegmentMaker::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        
        using HitsPerParticle = std::unordered_map<HepMC::ConstGenParticlePtr, SimHitVec_t>;
        using HitCollector = std::unordered_map<const MuonGMR4::SpectrometerSector*, HitsPerParticle>;
        HitCollector hitCollector{};

        for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key : m_readKeys) {
            const xAOD::MuonSimHitContainer* simHits{nullptr};
            ATH_CHECK(SG::get(simHits, key, ctx));        
            for (const xAOD::MuonSimHit* simHit : *simHits) {
                const MuonGMR4::MuonReadoutElement* reElement = m_detMgr->getReadoutElement(simHit->identify()); 
                const MuonGMR4::SpectrometerSector* id{reElement->msSector()};
                auto genLink = simHit->genParticleLink();
                HepMC::ConstGenParticlePtr genParticle = nullptr; 
                if (genLink.isValid()){
                    genParticle = genLink.cptr(); 
                }
                if ( (!genParticle && (!m_includePileUpHits || simHit->kineticEnergy() < m_pileUpHitMinE 
                                    || !MC::isMuon(simHit))) || (m_useOnlyMuonHits && !MC::isMuon(simHit))) {
                    ATH_MSG_VERBOSE("Skip hit "<<m_idHelperSvc->toString(simHit->identify())<<
                                  " pdgId: "<<simHit->pdgId()<<", energy: "<<simHit->kineticEnergy()
                                <<", genParticle: "<<genParticle);
                    continue;
                }
                const Amg::Transform3D toChTrf{toChamber(*gctx, simHit->identify())};
                hitCollector[id][genParticle].emplace_back(simHit, 
                                                           toChTrf *xAOD::toEigen(simHit->localPosition()),
                                                           toChTrf.linear()* xAOD::toEigen(simHit->localDirection())); 
            }
        } 

        SG::WriteHandle writeHandle{m_segmentKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                     std::make_unique<xAOD::MuonSegmentAuxContainer>()));

        WriteDecorHolder writerHolder{*writeHandle,*this, ctx};

        for (auto& [chamber, collectedParts] : hitCollector) {
            const Amg::Transform3D& locToGlob{chamber->localToGlobalTrans(*gctx)};
            for (auto& [particle, simHits]: collectedParts) {                
                /* Sort hits by local z */
                std::ranges::stable_sort(simHits,[](const HitPosTuple_t& a, const HitPosTuple_t& b){
                    return std::get<1>(a).z() < std::get<1>(b).z();
                });
                if (!particle) {
                    buildSegmentsFromBkg(ctx, locToGlob, simHits, writerHolder);
                    continue;
                }
                constructSegmentFromHits(ctx, locToGlob, simHits, writerHolder);
            }
        }
        ATH_MSG_DEBUG("Constructed "<<writeHandle->size()<<" truth segments in total ");
        return StatusCode::SUCCESS;
    }
}
