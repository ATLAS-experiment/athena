/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "sTgcFastDigiTool.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "xAODMuonViews/ChamberViewer.h"
namespace {
    constexpr double percentage(unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }
    using channelType = sTgcIdHelper::sTgcChannelTypes;
    using ChVec_t = std::vector<std::uint16_t>;
      
    static const SG::Decorator<ChVec_t> dec_stripCh{"sTgc_stripChannels"};
    static const SG::Decorator<ChVec_t> dec_wireCh{"sTgc_wireChannels"};
    static const SG::Decorator<ChVec_t> dec_padCh{"sTgc_padChannels"};
   
}
namespace MuonR4 {
    
    StatusCode sTgcFastDigiTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        ATH_CHECK(m_uncertCalibKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode sTgcFastDigiTool::finalize() {
        ATH_MSG_INFO("Tried to convert "<<m_allHits[channelType::Strip]<<"/"
                                        <<m_allHits[channelType::Wire]<<"/"
                                        <<m_allHits[channelType::Pad]<<" strip/wire/pad hits. In, "
                    <<percentage(m_acceptedHits[channelType::Strip], m_allHits[channelType::Strip]) <<"/"
                    <<percentage(m_acceptedHits[channelType::Wire], m_allHits[channelType::Wire])<<"/"
                    <<percentage(m_acceptedHits[channelType::Pad], m_allHits[channelType::Pad])
                    <<"% of the cases, the conversion was successful");
        return StatusCode::SUCCESS;
    }
    StatusCode sTgcFastDigiTool::digitize(const EventContext& ctx,
                                          const TimedHits& hitsToDigit,
                                          xAOD::MuonSimHitContainer* sdoContainer) const {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        // Prepare the temporary cache
        DigiCache digitCache{};
        /// Fetch the conditions for efficiency calculations
        const Muon::DigitEffiData* efficiencyMap{nullptr};
        ATH_CHECK(SG::get(efficiencyMap, m_effiDataKey, ctx));
        const NswErrorCalibData* nswUncertDB{nullptr};
        ATH_CHECK(SG::get(nswUncertDB, m_uncertCalibKey, ctx));
        
        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {
            for (const TimedHit& simHit : viewer) {
                /// ignore radiation for now
                if (m_digitizeMuonOnly && !MC::isMuon(simHit)){
                    continue;
                }

                if (simHit->energyDeposit() < m_energyDepositThreshold){
                    ATH_MSG_VERBOSE("Hit with Energy Deposit of " << simHit->energyDeposit()
                    << " less than " << m_energyDepositThreshold << ". Skip this hit." );
                    continue;
                }

                const double hitKineticEnergy = simHit->kineticEnergy();
                if (hitKineticEnergy < m_limitElectronKineticEnergy && MC::isElectron(simHit)) {
                    ATH_MSG_DEBUG("Skip electron hit with kinetic energy " << hitKineticEnergy
                              << ", which is less than the lower limit of " << m_limitElectronKineticEnergy);
                    continue;
                }
                sTgcDigitCollection* digiColl = fetchCollection(simHit->identify(), digitCache);
                const bool digitizedPad = digitizePad(ctx, simHit, efficiencyMap, rndEngine, *digiColl);
                const std::int16_t padChannel = digitizedPad ? idHelper.channel(digiColl->back()->identify()) : -1;
                const bool digitizedWire = digitizeWire(ctx, simHit, efficiencyMap, rndEngine, *digiColl);
                const std::int16_t  wireChannel = digitizedWire ? idHelper.channel(digiColl->back()->identify()) : -1;
                const bool digitizedStrip = digitizeStrip(ctx, simHit, nswUncertDB, efficiencyMap, rndEngine, *digiColl);
                const std::int16_t stripCh = digitizedStrip ? idHelper.channel(digiColl->back()->identify()) : -1;
            
                if (digitizedStrip || digitizedPad || digitizedWire) {
                    xAOD::MuonSimHit* sdo = addSDO(simHit, sdoContainer);
                    ChVec_t& stripChV{dec_stripCh(*sdo)}, wireCh{dec_wireCh(*sdo)}, padCh{dec_padCh(*sdo)};
                    if (stripCh > 0) { stripChV.push_back(stripCh); }
                    if (wireChannel > 0) { stripChV.push_back(wireChannel); }
                    if (padChannel > 0) { stripChV.push_back(padChannel); }
                    sdo->setIdentifier(digiColl->back()->identify());
                }
            }
        } while (viewer.next());
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), 
                                      idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    } 
    bool sTgcFastDigiTool::digitizeStrip(const EventContext& ctx,
                                         const TimedHit& timedHit,
                                         const NswErrorCalibData* errorCalibDB,
                                         const Muon::DigitEffiData* efficiencyMap,
                                         CLHEP::HepRandomEngine* rndEngine,
                                         sTgcDigitCollection& outCollection) const {
   
        if (!m_digitizeStrip) {
            return false;
        }
        ++m_allHits[channelType::Strip];
        const Identifier hitId{timedHit->identify()};
        const MuonGMR4::sTgcReadoutElement* readOutEle{m_detMgr->getsTgcReadoutElement(hitId)};
        
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};

        const IdentifierHash meashHash = readOutEle->measurementHash(hitId);
        const MuonGMR4::StripDesign& design{readOutEle->stripDesign(meashHash)};

        const Amg::Vector2D stripPos = readOutEle->stripLayer(meashHash)
                                    .to2D(xAOD::toEigen(timedHit->localPosition()), false);
                    
        const int stripNum = design.stripNumber(stripPos);
        if (stripNum < 0) {
            ATH_MSG_VERBOSE("Strip hit "<<Amg::toString(stripPos)<<" "<<m_idHelperSvc->toStringGasGap(hitId)
                          <<" is out of range "<<std::endl<<design);
            return false;
        }


        bool isValid{false};
        const int gasGap = idHelper.gasGap(hitId);
        const Identifier stripId = idHelper.channelID(hitId, readOutEle->multilayer(),
                                                      gasGap, channelType::Strip, stripNum, isValid);
        
        if (!isValid) {
            ATH_MSG_WARNING("Failed to deduce a valid identifier from "
                            <<m_idHelperSvc->toStringGasGap(hitId)<<" strip: "<<stripNum);
            return false;
        }

        /// Check efficiencies
        bool isInnerQ1 = readOutEle->isEtaZero(readOutEle->measurementHash(hitId), stripPos);
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId, isInnerQ1) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
            ATH_MSG_VERBOSE("Simulated strip hit "<<xAOD::toEigen(timedHit->localPosition())
                            << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
            return false;
        }

        NswErrorCalibData::Input errorCalibInput{};
        errorCalibInput.stripId= stripId;
        errorCalibInput.locTheta = M_PI - timedHit->localDirection().theta();
        errorCalibInput.clusterAuthor = 3; // centroid
    
        const double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);
        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, stripPos.x(), uncert);
        
        const Amg::Vector2D digitPos{smearedX * Amg::Vector2D::UnitX()};

        const int digitStrip = design.stripNumber(digitPos);
        if (digitStrip < 0) {
            ATH_MSG_VERBOSE("Smeared strip hit "<<Amg::toString(digitPos)<<" "<<m_idHelperSvc->toStringGasGap(hitId)
                          <<" is out of range "<<std::endl<<design);
            return false;
        }
        const Identifier digitId = idHelper.channelID(hitId, readOutEle->multilayer(),
                                                      gasGap, channelType::Strip, digitStrip, isValid);
  
        if (!isValid) {
            ATH_MSG_WARNING("Failed to deduce a valid identifier from "
                            <<m_idHelperSvc->toStringGasGap(hitId)<<" digit: "<<digitStrip);
            return false;
        }
        constexpr double dummyCharge = 66666;
        /// We're using the NSW uncertainty DB to smear the truth-hit positions using the best known
        /// uncertainties. In the process of digit -> RDO -> PRD, the smeared hit position is gonna be lost
        /// However, recall that the simplest way to generate a sTgc prd strip prd is the clustering by
        /// means of center of gravity. On average, a muon passage makes 3-4 strips to fire. Assuming 3 strips,
        /// the smeared position can be written as
        ///
        ///             x = w_{1}*(c - p) + w_{2}*c + w_{3}*(c + p),
        /// 
        ///  where c is the strip centre, p the pitch and w_{i} are the induced charge fractions on each strip
        /// 
        ///            1 =  w_{1} + w_{2} + w_{3}
        ///            x = (w_{1} + w_{2} + w_{3}) * c + (w_{3} - w_{1})*p
        ///               --> x = c + (w_{3} - w_{1}) *p
        ///               --> (x-c) / p = (w_{3} - w_{1})  
        ///               --> (x-c) / p + w_{1} = w_{3}
        ///
        ///                  1 =  2 * w_{1} + w_{2} + (x-c) / p
        ///                  1 - (x-c) /p - 2*w_{1} = w_{2}
        ///                
        ///                  0< w_{1} < 0.5 - (x-c) / 2p 
        const double pull = (smearedX - (*design.center(digitStrip)).x()) / design.stripPitch();
        const double w1 = CLHEP::RandFlat::shoot(rndEngine, 0., 0.5 *(1. - pull)); 
        const double w2 = 1. - pull -2.*w1;
        const double w3 = pull + w1;
        
        if (digitStrip> 1) {
            const Identifier stripIdB = idHelper.channelID(hitId, readOutEle->multilayer(),
                                                       gasGap, channelType::Strip, digitStrip -1);
            outCollection.push_back(std::make_unique<sTgcDigit>(stripIdB,
                                                                associateBCIdTag(ctx, timedHit), 
                                                                hitTime(timedHit), dummyCharge * w1, false, false));
        }
        outCollection.push_back(std::make_unique<sTgcDigit>(digitId,
                                                            associateBCIdTag(ctx, timedHit), 
                                                            hitTime(timedHit), dummyCharge * w2, false, false));
        
        if (digitStrip + 1 <= design.numStrips()) {
            const Identifier stripIdA = idHelper.channelID(hitId, readOutEle->multilayer(),
                                                           gasGap, channelType::Strip, digitStrip + 1);

            outCollection.push_back(std::make_unique<sTgcDigit>(stripIdA,
                                                                associateBCIdTag(ctx, timedHit), 
                                                                hitTime(timedHit), dummyCharge * w3, false, false));
        }
        ++m_acceptedHits[channelType::Strip];
        return true;
    }
            
    bool sTgcFastDigiTool::digitizeWire(const EventContext& ctx,
                                        const TimedHit& timedHit,
                                        const Muon::DigitEffiData* efficiencyMap,
                                        CLHEP::HepRandomEngine* rndEngine,
                                        sTgcDigitCollection& outCollection) const {

        if (!m_digitizeWire) {
            return false;
        }

        ++m_allHits[channelType::Wire];

        const Identifier hitId{timedHit->identify()};
        /// Check efficiencies
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
            ATH_MSG_VERBOSE("Simulated wire hit "<<xAOD::toEigen(timedHit->localPosition())
                            << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
            return false;
        }
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        const MuonGMR4::sTgcReadoutElement* readOutEle = m_detMgr->getsTgcReadoutElement(hitId);
        const IdentifierHash hitHash = readOutEle->measurementHash(hitId);
        const int gasGap = idHelper.gasGap(hitId);
        
        const Amg::Vector2D wirePos = readOutEle->stripLayer(hitHash).to2D(xAOD::toEigen(timedHit->localPosition()), true);
        // do not digitise wires that are never read out in reality
        bool isInnerQ1 = readOutEle->isEtaZero(hitHash, wirePos);
        if(isInnerQ1) {
            return false;
        }
        /// Check efficiencies
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId, isInnerQ1) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
            ATH_MSG_VERBOSE("Simulated wire hit "<<xAOD::toEigen(timedHit->localPosition())
                            << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
            return false;
        }

        const MuonGMR4::WireGroupDesign& design{readOutEle->wireDesign(hitHash)};
        
        const int wireGrpNum = design.stripNumber(wirePos);
        if (wireGrpNum < 0) {
            ATH_MSG_VERBOSE("The wire "<<Amg::toString(wirePos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                        <<" is outside of the acceptance of "<<std::endl<<design);
            return false;
        }
        const double uncert = design.stripPitch() * design.numWiresInGroup(wireGrpNum);

        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, wirePos.x(), uncert);
        
        const Amg::Vector2D digitPos{smearedX * Amg::Vector2D::UnitX()};

        const int digitWire = design.stripNumber(digitPos);
        if (digitWire < 0) {
            ATH_MSG_VERBOSE("Strip hit "<<Amg::toString(digitPos)<<" "<<m_idHelperSvc->toStringGasGap(hitId)
            <<" is out of range "<<std::endl<<design);
            return false;
        }
        bool isValid{false};
        const Identifier digitId = idHelper.channelID(hitId, readOutEle->multilayer(),
                                                      gasGap, channelType::Wire, digitWire, isValid);
  
        if (!isValid) {
            ATH_MSG_WARNING("Failed to deduce a valid identifier from "
                            <<m_idHelperSvc->toStringGasGap(hitId)<<" digit: "<<digitWire);
            return false;
        }
        outCollection.push_back(std::make_unique<sTgcDigit>(digitId,
                                                            associateBCIdTag(ctx, timedHit), 
                                                            hitTime(timedHit), 666, false, false));


        ++m_acceptedHits[channelType::Wire];
        return true;
    }
    int sTgcFastDigiTool::associateBCIdTag(const EventContext& /*ctx*/,
                                           const TimedHit& /*timedHit*/) const {
        /// To be implemented
        return 0;
    }

    bool sTgcFastDigiTool::digitizePad(const EventContext& ctx,
                                       const TimedHit& timedHit,
                                       const Muon::DigitEffiData* efficiencyMap,
                                       CLHEP::HepRandomEngine* rndEngine,
                                       sTgcDigitCollection& outCollection) const {
        
        if (!m_digitizePads) {
            return false;
        }

        ++m_allHits[channelType::Pad];

        const Identifier hitId{timedHit->identify()};
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        const MuonGMR4::sTgcReadoutElement* readOutEle = m_detMgr->getsTgcReadoutElement(hitId);
        const int gasGap = idHelper.gasGap(hitId);

        const IdentifierHash hitHash = readOutEle->createHash(gasGap, sTgcIdHelper::sTgcChannelTypes::Pad, 1);
        
        
        const Amg::Vector2D padPos{readOutEle->stripLayer(hitHash).to2D(xAOD::toEigen(timedHit->localPosition()), true)};
        /// 
        const MuonGMR4::PadDesign& design{readOutEle->padDesign(hitHash)};
        
        const auto [padEta, padPhi] = design.channelNumber(padPos);
        if (padEta < 0 || padPhi < 0) {
            ATH_MSG_VERBOSE("The pad "<<Amg::toString(padPos)<<" in "<<m_idHelperSvc->toStringGasGap(hitId)
                        <<" is outside of the acceptance of "<<std::endl<<design);
            return false;
        }
        bool isValid{false};
        const Identifier padId = idHelper.padID(hitId, readOutEle->multilayer(),
                                                gasGap, channelType::Pad, padEta, padPhi, isValid);


        if (!isValid) {
            ATH_MSG_WARNING("Failed to decuce a valid pad Identifier from "<<Amg::toString(padPos)
                            <<" in "<<m_idHelperSvc->toStringGasGap(hitId));
            return false;
        }
        
        /// Check efficiencies
        bool isInnerQ1 = readOutEle->isEtaZero(readOutEle->measurementHash(hitId), padPos);
        if (efficiencyMap && efficiencyMap->getEfficiency(hitId, isInnerQ1) < CLHEP::RandFlat::shoot(rndEngine,0.,1.)){
            ATH_MSG_VERBOSE("Simulated pad hit "<<xAOD::toEigen(timedHit->localPosition())
                            << m_idHelperSvc->toString(hitId) <<" is rejected because of efficency modelling");
            return false;
        }

        outCollection.push_back(std::make_unique<sTgcDigit>(padId,
                                                            associateBCIdTag(ctx, timedHit), 
                                                            hitTime(timedHit), 666, false, false));
    
        ++m_acceptedHits[channelType::Pad];

        return true;
    }
  

}
