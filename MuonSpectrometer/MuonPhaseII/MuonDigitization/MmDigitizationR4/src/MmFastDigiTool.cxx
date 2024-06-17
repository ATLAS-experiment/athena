/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MmFastDigiTool.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
namespace {
    constexpr double percentage(unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }
}
namespace MuonR4 {
    
    MmFastDigiTool::MmFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID):
        MuonDigitizationTool{type,name, pIID} {}

    StatusCode MmFastDigiTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_uncertCalibKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        return StatusCode::SUCCESS;
    }
    StatusCode MmFastDigiTool::finalize() {
        std::stringstream statstr{};
        unsigned allHits{0};
        for (unsigned int g = 0; g < m_allHits.size(); ++g) {
            allHits += m_allHits[g];
            statstr<<" *** Layer "<<(g+1)<<" "<<percentage(m_acceptedHits[g],m_allHits[g])
                   <<"% of "<<m_allHits[g]<<std::endl; 
        }
        if(!allHits) return StatusCode::SUCCESS;
        ATH_MSG_INFO("Tried to convert "<<allHits<<" hits. Successes rate per layer  "<<std::endl<<statstr.str());
        return StatusCode::SUCCESS;
    }
    StatusCode MmFastDigiTool::digitize(const EventContext& ctx,
                                        const TimedHits& hitsToDigit,
                                        xAOD::MuonSimHitContainer* sdoContainer) const {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        // Prepare the temporary cache
        DigiCache digitCache{};
        /// Fetch the conditions for efficiency calculations
        const Muon::DigitEffiData* efficiencyMap{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_effiDataKey, efficiencyMap));
        const NswErrorCalibData* errorCalibDB{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_uncertCalibKey, errorCalibDB));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        for (const TimedHit& simHit : hitsToDigit) {
            const Identifier hitId{simHit->identify()};
            /// ignore radiation for now
            if (std::abs(simHit->pdgId()) != 13) continue;
            
            const MuonGMR4::MmReadoutElement* readOutEle = m_detMgr->getMmReadoutElement(hitId);
            const Amg::Vector2D locPos{xAOD::toEigen(simHit->localPosition()).block<2,1>(0,0)};
            
            /// Calculate the index for the global hit counter
            const unsigned int hitGapInNsw = (idHelper.multilayer(hitId) -1) * 4 + idHelper.gasGap(hitId) -1;
            ++m_allHits[hitGapInNsw];
            
            const MuonGMR4::StripDesign& design{readOutEle->stripLayer(hitId).design()};
            bool isValid{false};

            int channelNumber = design.stripNumber(locPos);
            if(channelNumber<0){
                if (!design.insideTrapezoid(locPos)) {
                    ATH_MSG_WARNING("Hit "<<m_idHelperSvc->toString(hitId)<<" "<<Amg::toString(locPos)
                                  <<" is outside bounds "<<std::endl<<design<<" rejecting it");
                }
                continue;
            }
            const Identifier clusId = idHelper.channelID(hitId, 
                                                         idHelper.multilayer(hitId), 
                                                         idHelper.gasGap(hitId), 
                                                         channelNumber, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("Invalid strip identifier for layer " << m_idHelperSvc->toStringGasGap(hitId) 
                             << " channel " << channelNumber << " locPos " << Amg::toString(locPos));
                continue;
            }
            
            NswErrorCalibData::Input errorCalibInput{};
            errorCalibInput.stripId = clusId;
            errorCalibInput.locTheta = M_PI- simHit->localDirection().theta();
            errorCalibInput.clusterAuthor=66; // cluster time projection method
            const double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);
            ATH_MSG_VERBOSE("mm hit has theta " << errorCalibInput.locTheta / Gaudi::Units::deg << " and uncertainty " << uncert);

            const double newLocalX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPos.x(), uncert);

            const int newChannel = design.stripNumber(newLocalX * Amg::Vector2D::UnitX());
            if (newChannel < 0) {                
                continue;
            }
            const Identifier digitId = idHelper.channelID(hitId, 
                                                         idHelper.multilayer(hitId), 
                                                         idHelper.gasGap(hitId), 
                                                         newChannel, isValid);
            if(!isValid) {
                ATH_MSG_WARNING("Invalid strip identifier for layer " << m_idHelperSvc->toStringGasGap(hitId) 
                             << " channel " << newChannel << " locPos " << Amg::toString(locPos));
                continue;
            }
            
            /// Pipe some dummy values to the digit to ensure that the pdo / tdo calibration 
            /// does not reject any hit downstream
            constexpr int dummyResponseTime = 100;
            constexpr int dummyDepositedCharge = 666;
            fetchCollection(hitId, digitCache)->push_back(std::make_unique<MmDigit>(digitId, 
                                                                                    dummyResponseTime, 
                                                                                    dummyDepositedCharge));
            
            addSDO(simHit, sdoContainer);
            ++m_acceptedHits[hitGapInNsw];
        }
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }   
}