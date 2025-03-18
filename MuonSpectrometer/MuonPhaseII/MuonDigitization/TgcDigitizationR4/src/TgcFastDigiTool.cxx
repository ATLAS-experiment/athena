/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcFastDigiTool.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "xAODMuonViews/ChamberViewer.h"
namespace {
    constexpr double percentage(unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }
}
namespace MuonR4 {
    
    TgcFastDigiTool::TgcFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID):
        MuonDigitizationTool{type,name, pIID} {}

    StatusCode TgcFastDigiTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        return StatusCode::SUCCESS;
    }
    StatusCode TgcFastDigiTool::finalize() {
        ATH_MSG_INFO("Tried to convert "<<m_allHits[0]<<"/"<<m_allHits[1]<<" hits. In, "
                    <<percentage(m_acceptedHits[0], m_allHits[0]) <<"/"
                    <<percentage(m_acceptedHits[1], m_allHits[1]) <<"% of the cases, the conversion was successful");
        return StatusCode::SUCCESS;
    }

    int TgcFastDigiTool::associateBCIdTag(const EventContext& /*ctx*/,
                                          const TimedHit& /*timedHit*/) const {
        return TgcDigit::BC_CURRENT;
    }

    bool TgcFastDigiTool::digitizeWireHit(const EventContext& ctx,
                                          const TimedHit& timedHit,
                                          const Muon::DigitEffiData* efficiencyMap,
                                          TgcDigitCollection& outColl,
                                          CLHEP::HepRandomEngine* rndEngine,
                                          DeadTimeMap& deadTimes) const {

        
        /// Check efficiencies
        const Identifier hitId = timedHit->identify();
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        const MuonGMR4::TgcReadoutElement* readOutEle = m_detMgr->getTgcReadoutElement(hitId);
        const unsigned int gasGap = idHelper.gasGap(hitId);

        if (!readOutEle->numWireGangs(gasGap)) {
            ATH_MSG_VERBOSE("There're no wires in "<<m_idHelperSvc->toString(hitId)<<" nothing to do");
            return false;
        }
        ++m_allHits[false];
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
                ATH_MSG_VERBOSE("Simulated hit "<<xAOD::toEigen(timedHit->localPosition())
                              << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
                return false;
        }
        
        const Amg::Vector2D locSimHitPos{xAOD::toEigen(timedHit->localPosition()).block<2,1>(0,0)}; 
        
        const MuonGMR4::WireGroupDesign& design{readOutEle->wireGangLayout(gasGap)};
        if (!design.insideTrapezoid(locSimHitPos)) {
            ATH_MSG_DEBUG("The hit "<<Amg::toString(locSimHitPos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                        <<" is outside of the trapezoid "<<design);
            return false;
        }
        const int wireGrpNum = design.stripNumber(locSimHitPos);

        if (wireGrpNum < 0) {
            ATH_MSG_DEBUG("True hit "<<Amg::toString(locSimHitPos)<<" "<<m_idHelperSvc->toStringGasGap(hitId)
                        <<" is not covered by any wire gang");
            return false;
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
            return false;
        }
        bool isValid{false};
        const Identifier digitId{idHelper.channelID(hitId, gasGap, false, prdWireNum, isValid)};
        if (!isValid) {
            ATH_MSG_WARNING("Invalid channel "<< m_idHelperSvc->toStringGasGap(hitId)<<", channel: "<<prdWireNum);
            return false;
        }
        
        if (!passDeadTime(digitId, hitTime(timedHit), m_deadTime, deadTimes)) {
            ATH_MSG_VERBOSE("Reject hit due to dead map constraint");
            return false;
        }
        ATH_MSG_VERBOSE("Convert simulated hit "<<m_idHelperSvc->toString(digitId)<<" located at "
                      <<Amg::toString(locSimHitPos, 2)<<" wire group number: "<<prdWireNum<<", time: "<<hitTime(timedHit)
                      <<" wiregroup pos "<<Amg::toString(design.center(prdWireNum).value_or(Amg::Vector2D::Zero()), 2));
        outColl.push_back(std::make_unique<TgcDigit>(digitId, associateBCIdTag(ctx, timedHit)));
        ++m_acceptedHits[false];    
        return true;
    }
    bool TgcFastDigiTool::digitizeStripHit(const EventContext& ctx,
                                           const TimedHit& timedHit,
                                           const Muon::DigitEffiData* efficiencyMap,
                                           TgcDigitCollection& outColl,
                                           CLHEP::HepRandomEngine* rndEngine,
                                           DeadTimeMap& deadTimes) const {
        
        const Identifier hitId = timedHit->identify();
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        const MuonGMR4::TgcReadoutElement* readOutEle = m_detMgr->getTgcReadoutElement(hitId);
        
        const unsigned int gasGap = idHelper.gasGap(hitId);
        if (!readOutEle->numStrips(gasGap)) {
            ATH_MSG_VERBOSE("There're no strips in "<<m_idHelperSvc->toString(hitId)<<" nothing to do");
            return false;
        }
        ++(m_allHits[true]);

        /// Check efficiencies
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
                ATH_MSG_VERBOSE("Simulated hit "<<xAOD::toEigen(timedHit->localPosition())
                              << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
                return false;
        }
    
        const ActsGeometryContext& gctx{getGeoCtx(ctx)};

        const IdentifierHash stripHash{MuonGMR4::TgcReadoutElement::constructHash(0, gasGap, true)};
        const IdentifierHash wireHash{MuonGMR4::TgcReadoutElement::constructHash(0, gasGap, false)};

        const Amg::Transform3D toPhiRot{readOutEle->globalToLocalTrans(gctx, stripHash) *
                                        readOutEle->localToGlobalTrans(gctx, wireHash)};
        const Amg::Vector2D locSimHitPos{(toPhiRot*xAOD::toEigen(timedHit->localPosition())).block<2,1>(0,0)};


        const MuonGMR4::RadialStripDesign& design{readOutEle->stripLayout(gasGap)};
        if (!design.insideTrapezoid(locSimHitPos)) {
            ATH_MSG_DEBUG("The eta hit "<<Amg::toString(locSimHitPos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                        <<" is outside of the trapezoid "<<std::endl<<design);
            return false;
        }
        int stripNum = design.stripNumber(locSimHitPos);
        if (stripNum < 0) {
            ATH_MSG_WARNING("Strip hit "<<Amg::toString(locSimHitPos)<<" cannot be assigned to any active strip for "
                        <<m_idHelperSvc->toStringGasGap(hitId)<<". "<<design);
            return false;
        }
        /// Vector pointing perpendicular to the strips
        const Amg::Vector2D stripNorm{design.stripNormal(stripNum)};
        
        /// From the local sim hit walk to the left & to the right strip edge. The sum of the two steps is the 
        /// pitch at that position
        const double stripPitch = std::abs(*Amg::intersect<2>(design.stripLeftBottom(stripNum), design.stripLeftEdge(stripNum),
                                                              locSimHitPos, stripNorm)) +
                                  std::abs(*Amg::intersect<2>(design.stripRightBottom(stripNum), design.stripRightEdge(stripNum),
                                                              locSimHitPos, stripNorm));

        const double uncert = stripPitch / std::sqrt(12);
        const double locX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locSimHitPos.x(), uncert);
        /// Recalculate the strip number with the smeared hit -> Use the real Y to ensure that the 
        /// hit remains within the active trapzoid
        const Amg::Vector2D smearedPos{locX, locSimHitPos.y()};
        const int digitStripNum = design.stripNumber(smearedPos);

        if (digitStripNum < 0) {
            if (design.insideTrapezoid(smearedPos)) {
                ATH_MSG_WARNING("True phi hit "<<Amg::toString(locSimHitPos, 2)<<" corresponding to "<<stripNum<<" --> "
                                <<Amg::toString(smearedPos)<<" "<<uncert<<" is outside of "<<design);
            }
            return false;
        }
        
        bool isValid{false};
        const Identifier digitId{idHelper.channelID(hitId, gasGap, true, digitStripNum, isValid)};
        if (!isValid) {
            ATH_MSG_WARNING("Invalid channel "<< m_idHelperSvc->toStringGasGap(hitId)<<", channel: "<<digitStripNum);
            return false;
        }

        if (!passDeadTime(digitId, hitTime(timedHit), m_deadTime, deadTimes)) {
            ATH_MSG_VERBOSE("Reject hit due to dead time constraint.");
            return false;
        }
        ATH_MSG_VERBOSE("Convert simulated hit "<<m_idHelperSvc->toString(digitId)<<" located at "
                        <<Amg::toString(locSimHitPos, 2)<<" phi strip number: "<<digitStripNum<<", time: "<<hitTime(timedHit)
                        <<" strip position "<<Amg::toString(design.center(digitStripNum).value_or(Amg::Vector2D::Zero()), 2));

        outColl.push_back(std::make_unique<TgcDigit>(digitId, associateBCIdTag(ctx, timedHit)));

        ++(m_acceptedHits[true]);
        return true;
    }
    StatusCode TgcFastDigiTool::digitize(const EventContext& ctx,
                                         const TimedHits& hitsToDigit,
                                         xAOD::MuonSimHitContainer* sdoContainer) const {

        // Prepare the temporary cache
        DigiCache digitCache{};
        /// Fetch the conditions for efficiency calculations
        const Muon::DigitEffiData* efficiencyMap{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_effiDataKey, efficiencyMap));
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        
        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {
            DeadTimeMap deadTimes{};
            for (const TimedHit& simHit : viewer) {
                /// ignore radiation for now
                if (m_digitizeMuonOnly && !MC::isMuon(simHit)) {
                    continue;
                }
                TgcDigitCollection* outColl = fetchCollection(simHit->identify(), digitCache);
 
                const bool digitizedEta = digitizeWireHit(ctx,simHit, efficiencyMap,*outColl, rndEngine, deadTimes);
                const bool digitizedPhi = digitizeStripHit(ctx, simHit, efficiencyMap,*outColl, rndEngine, deadTimes);
                
                if (digitizedEta) {
                    xAOD::MuonSimHit* sdo = addSDO(simHit, sdoContainer);
                    sdo->setIdentifier(outColl->at(outColl->size() - 1 - digitizedPhi)->identify());
                } else if (digitizedPhi) {
                    xAOD::MuonSimHit* sdo = addSDO(simHit, sdoContainer);
                    sdo->setIdentifier(outColl->at(outColl->size() - 1)->identify());
                    const MuonGMR4::TgcReadoutElement* re{m_detMgr->getTgcReadoutElement(simHit->identify())};

                    const Amg::Transform3D etaToPhi{re->globalToLocalTrans(getGeoCtx(ctx), re->layerHash(sdo->identify())) *
                                                    re->localToGlobalTrans(getGeoCtx(ctx), re->layerHash(simHit->identify()))};
                
                    sdo->setLocalDirection(xAOD::toStorage(etaToPhi * xAOD::toEigen(sdo->localDirection())));
                    sdo->setLocalPosition(xAOD::toStorage(etaToPhi * xAOD::toEigen(sdo->localPosition())));
                }
            }
        } while(viewer.next());
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }   
}
