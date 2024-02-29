/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitToTgcMeasCnvAlg.h"

#include <xAODMuonPrepData/TgcStripAuxContainer.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
#include <GaudiKernel/PhysicalConstants.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>

namespace {
    constexpr double percentage( unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }

}
xAODSimHitToTgcMeasCnvAlg::xAODSimHitToTgcMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToTgcMeasCnvAlg::initialize(){
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}
StatusCode xAODSimHitToTgcMeasCnvAlg::finalize() {
    ATH_MSG_INFO("Tried to convert "<<m_allHits[0]<<"/"<<m_allHits[1]<<" hits. In, "
                <<percentage(m_acceptedHits[0], m_allHits[0]) <<"/"
                <<percentage(m_acceptedHits[1], m_allHits[1]) <<" cases, the conversion was successful");
    return StatusCode::SUCCESS;
}
StatusCode xAODSimHitToTgcMeasCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainer{m_readKey, ctx};
    if (!simHitContainer.isPresent()){
        ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }

    const ActsGeometryContext gctx{};
    SG::WriteHandle<xAOD::TgcStripContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::TgcStripContainer>(),
                                  std::make_unique<xAOD::TgcStripAuxContainer>()));
    
    const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
    auto dumpHit = [&](const Identifier& hitId,
                       const double xLoc,
                       const double xUncert) {

        xAOD::TgcStrip* prd = new xAOD::TgcStrip();
        prdContainer->push_back(prd);
        prd->setIdentifier(hitId.get_compact());
        xAOD::MeasVector<1> lPos{xLoc};
        xAOD::MeasMatrix<1> cov{xUncert * xUncert};
        prd->setMeasurement<1>(m_idHelperSvc->detElementHash(hitId), 
                               std::move(lPos), std::move(cov));
        prd->setChannelNumber(id_helper.channel(hitId));
        prd->setGasGap(id_helper.gasGap(hitId));
        const bool measPhi{id_helper.measuresPhi(hitId)};
        ++(m_acceptedHits[measPhi]);
        prd->setMeasuresPhi(measPhi);
        const MuonGMR4::TgcReadoutElement* readOutEle = m_DetMgr->getTgcReadoutElement(hitId);
        prd->setReadoutElement(readOutEle);
        const Amg::Vector3D strip3D = lPos.x() * Amg::Vector3D::UnitX();
        const Amg::Transform3D& globToCenter{readOutEle->getChamber()->localToGlobalTrans(gctx)};
        prd->setStripPosInStation(xAOD::toStorage(globToCenter * readOutEle->localToGlobalTrans(gctx,prd->layerHash()) * strip3D)); 
    };
    
    auto processEtaHit = [&] (const Amg::Vector3D& locSimHitPos,
                              const Identifier& hitId) {
        ++(m_allHits[false]);
        const MuonGMR4::TgcReadoutElement* readOutEle = m_DetMgr->getTgcReadoutElement(hitId);
        const unsigned int gasGap = id_helper.gasGap(hitId);
        const MuonGMR4::WireGroupDesign& design{readOutEle->wireGangLayout(gasGap)};
        if (!design.insideTrapezoid(locSimHitPos.block<2,1>(0,0))) {
            ATH_MSG_DEBUG("The hit "<<Amg::toString(locSimHitPos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                            <<" is outside of the trapezoid "<<design);
            return;
        }
        const int wireGrpNum = design.stripNumber(locSimHitPos.block<2,1>(0,0));

        if (wireGrpNum < 0) {
            ATH_MSG_DEBUG("True hit is not "<<Amg::toString(locSimHitPos)<<" "<<m_idHelperSvc->toStringGasGap(hitId));
            return;
        }

        const double uncert = design.stripPitch() * design.numWiresInGroup(wireGrpNum) / std::sqrt(12);
        const double locX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locSimHitPos.x(), uncert);
        /// Recalculate the strip number with the smeared hit -> Use the real Y to ensure that the 
        /// hit remains within the active trapzoid
        const Amg::Vector2D smearedPos{locX, locSimHitPos.y()};
        const int prdWireNum = design.stripNumber(smearedPos);

        if (prdWireNum < 0) {
            if (design.insideTrapezoid(smearedPos)) {
                ATH_MSG_WARNING("True hit "<<Amg::toString(locSimHitPos, 2)<<" corresponding to "<<wireGrpNum<<" --> "
                                <<Amg::toString(smearedPos)<<" "<<uncert<<" is outside of "<<design);
            }
            return;
        }
        bool isValid{false};
        const Identifier prdId{id_helper.channelID(hitId, gasGap, false, prdWireNum, isValid)};
        if (!isValid) {
            ATH_MSG_WARNING("Invalid channel "<< m_idHelperSvc->toStringGasGap(hitId)<<", channel: "<<prdWireNum);
        }
        ATH_MSG_VERBOSE("Convert simulated hit "<<m_idHelperSvc->toStringGasGap(hitId)<<" located in gas gap at "
                        <<Amg::toString(locSimHitPos, 2)<<" eta strip number: "<<prdWireNum
                        <<" strip position "<<Amg::toString(design.center(prdWireNum).value_or(Amg::Vector2D::Zero()), 2));
     
        dumpHit(prdId, locX, uncert);
    };

    auto processStripHit  = [&] (const Amg::Vector3D& locSimHitPos,
                                 const Identifier& hitId) {
        ++(m_allHits[true]);
        const MuonGMR4::TgcReadoutElement* readOutEle = m_DetMgr->getTgcReadoutElement(hitId);
        const unsigned int gasGap = id_helper.gasGap(hitId);
        const MuonGMR4::RadialStripDesign& design{readOutEle->stripLayout(gasGap)};
        if (!design.insideTrapezoid(locSimHitPos.block<2,1>(0,0))) {
            ATH_MSG_DEBUG("The eta hit "<<Amg::toString(locSimHitPos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                            <<" is outside of the trapezoid "<<design);
            return;
        }
        int stripNum = design.stripNumber(locSimHitPos.block<2,1>(0,0));
        if (stripNum < 0) {
            ATH_MSG_WARNING("Strip hit "<<Amg::toString(locSimHitPos)<<" cannot be assigned to any active strip for "
                        <<m_idHelperSvc->toStringGasGap(hitId)<<". "<<design);
            return;
        }
        const double stripPitch = std::abs(*Amg::intersect<2>(design.stripLeftBottom(stripNum), design.stripLeftEdge(stripNum),
                                                              locSimHitPos.block<2,1>(0,0), design.stripNormal(stripNum))) +
                                  std::abs(*Amg::intersect<2>(design.stripRightBottom(stripNum), design.stripRightEdge(stripNum),
                                                              locSimHitPos.block<2,1>(0,0), design.stripNormal(stripNum)));
        
        const double uncert = stripPitch / std::sqrt(12);
        const double locX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locSimHitPos.x(), uncert);
        /// Recalculate the strip number with the smeared hit -> Use the real Y to ensure that the 
        /// hit remains within the active trapzoid
        const Amg::Vector2D smearedPos{locX, locSimHitPos.y()};
        const int prdStripNum = design.stripNumber(smearedPos);

        if (prdStripNum < 0) {
            if (design.insideTrapezoid(smearedPos)) {
                ATH_MSG_WARNING("True phi hit "<<Amg::toString(locSimHitPos, 2)<<" corresponding to "<<stripNum<<" --> "
                                <<Amg::toString(smearedPos)<<" "<<uncert<<" is outside of "<<design);
            }
            return;
        }
        bool isValid{false};
        const Identifier prdId{id_helper.channelID(hitId, gasGap, true, prdStripNum, isValid)};
        if (!isValid) {
            ATH_MSG_WARNING("Invalid channel "<< m_idHelperSvc->toStringGasGap(hitId)<<", channel: "<<prdStripNum);
        }
        ATH_MSG_VERBOSE("Convert simulated hit "<<m_idHelperSvc->toStringGasGap(hitId)<<" located in gas gap at "
                        <<Amg::toString(locSimHitPos, 2)<<" eta strip number: "<<prdStripNum
                        <<" strip position "<<Amg::toString(design.center(prdStripNum).value_or(Amg::Vector2D::Zero()), 2));
     
        dumpHit(prdId, locX, uncert);
    };
    for (const xAOD::MuonSimHit* simHit : *simHitContainer) {
        const Identifier hitId = simHit->identify();
        // ignore radiation for now
        if (std::abs(simHit->pdgId()) != 13) continue;

        const MuonGMR4::TgcReadoutElement* readOutEle = m_DetMgr->getTgcReadoutElement(hitId);
        const Amg::Vector3D locSimHitPos{xAOD::toEigen(simHit->localPosition())};
        const int gasGap = id_helper.gasGap(hitId);
        
        if (readOutEle->numWireGangs(gasGap)) {
            processEtaHit(locSimHitPos, hitId);
        }
        if (readOutEle->numStrips(gasGap)) {
            const IdentifierHash stripHash{MuonGMR4::TgcReadoutElement::constructHash(0, gasGap, true)};
            const IdentifierHash wireHash{MuonGMR4::TgcReadoutElement::constructHash(0, gasGap, false)};

            const Amg::Transform3D toPhiRot{readOutEle->globalToLocalTrans(gctx, stripHash) *
                                            readOutEle->localToGlobalTrans(gctx, wireHash)};

            processStripHit(toPhiRot * locSimHitPos, hitId);
        }
    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* xAODSimHitToTgcMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}
