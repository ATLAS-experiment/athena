/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcFastDigiTool.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
namespace {
    constexpr double percentage(unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }

}
namespace MuonR4 {
    
    RpcFastDigiTool::RpcFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID):
        MuonDigitizationTool{type,name, pIID} {}

    StatusCode RpcFastDigiTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        m_stIdxBIL = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
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
        ATH_CHECK(retrieveConditions(ctx, m_effiDataKey, efficiencyMap));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        for (const TimedHit& simHit : hitsToDigit) {
            const Identifier hitId{simHit->identify()};
            /// ignore radiation for now
            if (std::abs(simHit->pdgId()) != 13) continue;
            
            const MuonGMR4::RpcReadoutElement* readOutEle = m_detMgr->getRpcReadoutElement(hitId);
            const Amg::Vector3D locPos{xAOD::toEigen(simHit->localPosition())};
            RpcDigitCollection* digiColl = fetchCollection(hitId, digitCache);
            if (m_idHelperSvc->stationName(hitId) != m_stIdxBIL) {
                /// Standard digitization path
                bool digitized = digitizeHit(hitId, false, readOutEle->getParameters().etaDesign, 
                                             hitTime(simHit), locPos.x(), 
                                             efficiencyMap, *digiColl, rndEngine);
                digitized |=  digitizeHit(hitId, true, readOutEle->getParameters().phiDesign,  
                                          hitTime(simHit), locPos.y(),
                                          efficiencyMap, *digiColl, rndEngine);
                if (digitized) {
                    addSDO(simHit, sdoContainer);
                }
            } else if (digitizeHit(hitId, readOutEle->getParameters().etaDesign, 
                       hitTime(simHit), locPos.block<2,1>(0,0),
                       efficiencyMap, *digiColl, rndEngine)) {
                addSDO(simHit, sdoContainer);
            }
        }
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }   
    bool RpcFastDigiTool::digitizeHit(const Identifier& gasGapId,
                                      const bool measuresPhi,
                                      const MuonGMR4::StripDesignPtr& designPtr,
                                      const double hitTime,
                                      const double locPosOnStrip,
                                      const Muon::DigitEffiData* effiMap,
                                      RpcDigitCollection& outContainer,
                                      CLHEP::HepRandomEngine* rndEngine) const {

        /// There're Rpc chambers without phi strips (BI)
        if (!designPtr){
            return false;
        }
        ++(m_allHits[measuresPhi]);
        const MuonGMR4::StripDesign& design{*designPtr};

        const double uncert = design.stripPitch() / std::sqrt(12.);
        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPosOnStrip, uncert);
        const Amg::Vector2D locHitPos{smearedX * Amg::Vector2D::UnitX()};

        if (!design.insideTrapezoid(locHitPos)) {
            ATH_MSG_VERBOSE("The hit "<<Amg::toString(locHitPos)<<" is outside of the trapezoid bounds for "
                            <<m_idHelperSvc->toStringGasGap(gasGapId)<<", measuresPhi: "<<(measuresPhi ? "yay" : "nay"));
            return false;
        }
        const int strip = design.stripNumber(locHitPos);
        if (strip < 0) {
            ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " cannot trigger any signal in a strip for "
                            << m_idHelperSvc->toStringGasGap(gasGapId) <<std::endl<<design<<
                            std::endl<<", measuresPhi: "<<(measuresPhi ? "yay" : "nay"));
            return false;
        }

        const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};

        bool isValid{false};
        const Identifier digitId{id_helper.channelID(gasGapId, 
                                                     id_helper.doubletZ(gasGapId), 
                                                     id_helper.doubletPhi(gasGapId), 
                                                     id_helper.gasGap(gasGapId),
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
        outContainer.push_back(std::make_unique<RpcDigit>(digitId, hitTime));
        ++(m_acceptedHits[measuresPhi]);
        return true;
    }
    bool RpcFastDigiTool::digitizeHit(const Identifier& gasGapId,
                                      const MuonGMR4::StripDesignPtr& designPtr,
                                      const double hitTime,
                                      const Amg::Vector2D& locPos,
                                      const Muon::DigitEffiData* effiMap,
                                      RpcDigitCollection& outContainer,
                                      CLHEP::HepRandomEngine* rndEngine) const {
        
        ++(m_allHits[false]);
        /// Check whether the digit is actually efficient
        if (effiMap && effiMap->getEfficiency(gasGapId) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)) {
            ATH_MSG_VERBOSE("Hit is marked as inefficient");
            return false;            
        }        
        const MuonGMR4::StripDesign& design{*designPtr};

        const double uncert = design.stripPitch() / std::sqrt(12.);
        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPos.x(), uncert);
        const Amg::Vector2D locHitPos{smearedX * Amg::Vector2D::UnitX()};

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

        const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};

        bool isValid{false};
        const Identifier digitId{id_helper.channelID(gasGapId, 
                                                     id_helper.doubletZ(gasGapId), 
                                                     id_helper.doubletPhi(gasGapId), 
                                                     id_helper.gasGap(gasGapId),
                                                     false, strip, isValid)};

        ++(m_acceptedHits[false]);
        outContainer.push_back(std::make_unique<RpcDigit>(digitId, hitTime));
        return true;
    }
}