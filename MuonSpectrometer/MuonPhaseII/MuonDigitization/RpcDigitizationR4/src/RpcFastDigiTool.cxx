/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcFastDigiTool.h"

#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
namespace {
    constexpr double percentage(unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }
    static const SG::Decorator<std::int16_t> dec_phiChannel{"SDO_phiChannel"};
}
namespace MuonR4 {
    
    StatusCode RpcFastDigiTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        m_stIdxBIL = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
        m_stIdxBIS = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIS");
        return StatusCode::SUCCESS;
    }
    StatusCode RpcFastDigiTool::finalize() {
        ATH_MSG_INFO("Tried to convert "<<m_allHits[0]<<"/"<<m_allHits[1]<<" hits. In, "
                    <<percentage(m_acceptedHits[0], m_allHits[0]) <<"/"
                    <<percentage(m_acceptedHits[1], m_allHits[1]) <<"% of the cases, the conversion was successful");
        return StatusCode::SUCCESS;
    }
    StatusCode RpcFastDigiTool::digitize(const EventContext& ctx,
                                         const TimedHits& hitsToDigit,
                                         xAOD::MuonSimHitContainer* sdoContainer) const {
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        // Prepare the temporary cache
        DigiCache digitCache{};
        /// Fetch the conditions for efficiency calculations
        const Muon::DigitEffiData* efficiencyMap{nullptr};
        ATH_CHECK(SG::get(efficiencyMap, m_effiDataKey, ctx));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {
            DeadTimeMap deadTimes{};
            for (const TimedHit& simHit : viewer) {
                if (m_digitizeMuonOnly && !MC::isMuon(simHit)){
                    continue;
                }
                const Identifier hitId{simHit->identify()};
                const int stName = m_idHelperSvc->stationName(hitId);
                RpcDigitCollection* digiColl = fetchCollection(hitId, digitCache);
                bool run4_BI = (stName== m_stIdxBIS &&  std::abs(m_idHelperSvc->stationEta(hitId)) < 7) ||
                                stName == m_stIdxBIL;
                if (!run4_BI) {
                    /// Standard digitization path
                    const bool digitizedPhi = digitizeHit(simHit, true,  efficiencyMap, *digiColl, rndEngine, deadTimes);
                    std::int16_t phiChannel = digitizedPhi ? idHelper.channel(digiColl->back()->identify()) : -1;
                    const bool digitizedEta = digitizeHit(simHit, false, efficiencyMap, *digiColl, rndEngine, deadTimes);
                   
                    if (digitizedEta || digitizedPhi) {
                        xAOD::MuonSimHit* sdo = addSDO(simHit, sdoContainer);
                        sdo->setIdentifier(digiColl->back()->identify());
                        dec_phiChannel(*sdo) = phiChannel; 
                    }
                } else if (digitizeHitBI(simHit, efficiencyMap, *digiColl, rndEngine, deadTimes)) {
                    xAOD::MuonSimHit* sdo = addSDO(simHit, sdoContainer);
                    sdo->setIdentifier(digiColl->back()->identify());
                }
            }
        } while (viewer.next());
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }   
    bool RpcFastDigiTool::digitizeHit(const TimedHit& hit,
                                      const bool measuresPhi,
                                      const Muon::DigitEffiData* effiMap,
                                      RpcDigitCollection& outContainer,
                                      CLHEP::HepRandomEngine* rndEngine,
                                      DeadTimeMap& deadTimes) const {

        ++(m_allHits[measuresPhi]);

        const Identifier gasGapId = hit->identify();
        const MuonGMR4::RpcReadoutElement* reEle = m_detMgr->getRpcReadoutElement(gasGapId);

        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};

        bool isValid{false};
        const Identifier layerId = idHelper.channelID(gasGapId, 
                                                       idHelper.doubletZ(gasGapId), 
                                                       idHelper.doubletPhi(gasGapId), 
                                                       idHelper.gasGap(gasGapId),
                                                       measuresPhi, 1, isValid);


        const MuonGMR4::StripLayerPtr& layerDesign = reEle->sensorLayout(reEle->layerHash(layerId));

        const MuonGMR4::StripDesign& design{layerDesign->design(measuresPhi)};

        const double uncert = design.stripPitch() / std::sqrt(12.);
        Amg::Vector3D locHitPos{xAOD::toEigen(hit->localPosition())};
        locHitPos[measuresPhi] = CLHEP::RandGaussZiggurat::shoot(rndEngine, locHitPos[measuresPhi], uncert);

        const Amg::Vector2D locPos2D = layerDesign->to2D(locHitPos, measuresPhi); 
        if (!design.insideTrapezoid(locPos2D)) {
            ATH_MSG_VERBOSE("The hit "<<Amg::toString(locHitPos)<<" / "<<Amg::toString(locPos2D)
                          <<" is outside of the trapezoid bounds for "<<m_idHelperSvc->toString(layerId));
            return false;
        }
        const int strip = design.stripNumber(locPos2D);
        if (strip < 0) {
            ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) <<" / "<<Amg::toString(locPos2D) 
                    << " cannot trigger any signal in a strip for " << m_idHelperSvc->toString(layerId) 
                    <<std::endl<<design);
            return false;
        }

        const Identifier digitId{idHelper.channelID(gasGapId, 
                                                    idHelper.doubletZ(gasGapId), 
                                                    idHelper.doubletPhi(gasGapId), 
                                                    idHelper.gasGap(gasGapId),
                                                    measuresPhi, strip, isValid)};

        if (!isValid) {
            ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(gasGapId)
                            <<",  eta strip "<<strip<<" & hit "<<Amg::toString(locHitPos,2 )
                            <<" /// "<<design);
            return false;
        }
        /// Final check whether the digit is actually efficient
        if (effiMap && effiMap->getEfficiency(digitId) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)) {
            ATH_MSG_VERBOSE("Hit is marked as inefficient");
            return false;            
        }
        if (!passDeadTime(digitId, hitTime(hit), m_deadTime, deadTimes)) {
            ATH_MSG_VERBOSE("Reject hit due to dead map constraint");
            return false;
        }
        /// Correct for the signal propagation time
        const double signalTime = hitTime(hit) + reEle->distanceToEdge(reEle->measurementHash(digitId),
                                                                       locHitPos, EdgeSide::readOut) / m_propagationVelocity;
        const double digitTime = CLHEP::RandGaussZiggurat::shoot(rndEngine, signalTime, m_stripTimeResolution);
        ATH_MSG_VERBOSE("Created new digit "<<m_idHelperSvc->toString(digitId)<<", @ "<<Amg::toString(locPos2D)
                     <<", recorded time: "<<digitTime);
        outContainer.push_back(std::make_unique<RpcDigit>(digitId, digitTime, timeOverThreshold(rndEngine)));
        ++(m_acceptedHits[measuresPhi]);
        return true;
    }
    bool RpcFastDigiTool::digitizeHitBI(const TimedHit& simHit,
                                        const Muon::DigitEffiData* effiMap,
                                        RpcDigitCollection& outContainer,
                                        CLHEP::HepRandomEngine* rndEngine,
                                        DeadTimeMap& deadTimes) const {
        
        ++(m_allHits[false]);
        const Identifier gasGapId = simHit->identify();
        const MuonGMR4::RpcReadoutElement* reEle = m_detMgr->getRpcReadoutElement(gasGapId);
        const Amg::Vector3D locPos = xAOD::toEigen(simHit->localPosition());
        const MuonGMR4::StripDesign& design{*reEle->getParameters().etaDesign};
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};


        /// Smeared x coordinate
        const double uncert = design.stripPitch() / std::sqrt(12.);
        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPos.x(), uncert);

        /// Smear the Phi Coordinate
        // True propagation time in nanoseconds along strip to y=-stripLength/2 (L) and y=stripLength/2 (R)
        const IdentifierHash layHash = reEle->layerHash(gasGapId);
        const double propagationTimeL = reEle->distanceToEdge(layHash, locPos, EdgeSide::readOut)    / m_propagationVelocity; 
        const double propagationTimeR = reEle->distanceToEdge(layHash, locPos, EdgeSide::highVoltage)/ m_propagationVelocity; 

        /// Smeared propagation time in nanoseconds along strip to y=-stripLength/2 (L) and y=stripLength/2 (R)
        const double smearedTimeL = CLHEP::RandGaussZiggurat::shoot(rndEngine, propagationTimeL, m_stripTimeResolution); 
        const double smearedTimeR = CLHEP::RandGaussZiggurat::shoot(rndEngine, propagationTimeR, m_stripTimeResolution);     
        
        const double smearedDeltaT = smearedTimeR - smearedTimeL;

        /*
            |--- d1, t1 ---||--- d1, t1 ---||-- d --|                      For t2 > t1: 
            ||||||||||||||||X|||||||||||||||||||||||| <- RPC strip,            deltaT = t2 - t1,  d1 = v_prop * t1 (likewise for d2), 
                        |-------- d2, t2 -------|    X is a hit             l = d1 + d2, d = d2 - d1 -> d = l - 2d1 = v_prop * deltaT                         
            |----------------- l -------------------|
                                                    Hence, d1 = 0.5 * (l - d) = 0.5 * (l - v_prop * deltaT)

            Then converting to coordinate system where  0 -> -0.5*l to match strip local coordinates
                                            d1 -> d1 = -0.5*l + 0.5* (l-d) = -0.5*d = -0.5 * v_pro*deltaT
        */
        const double smearedY = 0.5 * m_propagationVelocity * smearedDeltaT; //in mm
        //If smearedDeltaT == 0 position is in the centre of the strip (0).

        const Amg::Vector2D locHitPos{smearedX, smearedY};

        if (!design.insideTrapezoid(locHitPos)) {
            ATH_MSG_VERBOSE("The hit "<<Amg::toString(locHitPos)<<" is outside of the trapezoid bounds for "
                            <<m_idHelperSvc->toStringGasGap(gasGapId));
            return false;
        }

        const int strip = design.stripNumber(locHitPos);
        if (strip < 0) {
            ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " cannot trigger any signal in a strip for "
                            << m_idHelperSvc->toStringGasGap(gasGapId) <<std::endl<<design);
            return false;
        }


        bool isValid{false};
        const Identifier digitId{idHelper.channelID(gasGapId, 
                                                     idHelper.doubletZ(gasGapId), 
                                                     idHelper.doubletPhi(gasGapId), 
                                                     idHelper.gasGap(gasGapId),
                                                     false, strip, isValid)};
        if (!isValid) {
            ATH_MSG_WARNING("Failed to create a valid strip "<<m_idHelperSvc->toStringGasGap(gasGapId)
                          <<", strip: "<<strip);
            return false;
        }
        if (!passDeadTime(digitId, hitTime(simHit), m_deadTime, deadTimes)) {
            ATH_MSG_VERBOSE("Reject hit due to dead map constraint");
            return false;
        }
        ATH_MSG_VERBOSE("Digitize hit "<<m_idHelperSvc->toString(digitId)<<" located at: "<<Amg::toString(locPos)
                     <<", SDO: "<<Amg::toString(locHitPos));
        /// Check whether the digit is actually efficient
        const bool effiSignal1 = !effiMap || effiMap->getEfficiency(gasGapId) >= CLHEP::RandFlat::shoot(rndEngine,0., 1.);
        const bool effiSignal2 = !effiMap || effiMap->getEfficiency(gasGapId) >= CLHEP::RandFlat::shoot(rndEngine,0., 1.);
        if (effiSignal1) {
            outContainer.push_back(std::make_unique<RpcDigit>(digitId, hitTime(simHit) + smearedTimeR, timeOverThreshold(rndEngine)));
        }
        if (effiSignal2) {
            outContainer.push_back(std::make_unique<RpcDigit>(digitId, hitTime(simHit) + smearedTimeL, timeOverThreshold(rndEngine), true));
        }
        if (effiSignal1 || effiSignal2) {
            ++(m_acceptedHits[false]);
            return true;
        }
        return false;
    }
    double RpcFastDigiTool::timeOverThreshold(CLHEP::HepRandomEngine* rndmEngine) {
        //mn Time-over-threshold modeled as a narrow and a wide gaussian
        //mn based on the fit documented in https://its.cern.ch/jira/browse/ATLASRECTS-7820
        constexpr double tot_mean_narrow = 16.;
        constexpr double tot_sigma_narrow = 2.;
        constexpr double tot_mean_wide = 15.;
        constexpr double tot_sigma_wide = 4.5;

        double thetot = 0.;

        if (CLHEP::RandFlat::shoot(rndmEngine)<0.75) {
          thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_narrow, tot_sigma_narrow);
        } else {
          thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_wide, tot_sigma_wide);
        }

        return std::max(thetot, 0.);
    }

}
