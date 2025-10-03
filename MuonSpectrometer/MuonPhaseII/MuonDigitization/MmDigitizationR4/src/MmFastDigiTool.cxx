/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MmFastDigiTool.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "TruthUtils/HepMCHelpers.h"
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
        ATH_CHECK(SG::get(efficiencyMap, m_effiDataKey, ctx));
        const NswErrorCalibData* errorCalibDB{nullptr};
        ATH_CHECK(SG::get(errorCalibDB, m_uncertCalibKey, ctx));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {
            DeadTimeMap deadTimes{};
            for (const TimedHit& simHit : viewer) {
              if (m_digitizeMuonOnly && !MC::isMuon(simHit)){
                    continue;
                }

                const double hitKineticEnergy = simHit->kineticEnergy();
                // Don't consider electron hits below m_energyThreshold.
                // Electrons aren't consider for now in any case due to the cut above.
                // But this may change.
                if (hitKineticEnergy < m_energyThreshold && MC::isElectron(simHit)) {
                    continue;
                }
                
                const Identifier hitId{simHit->identify()};
 
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

                if(efficiencyMap && efficiencyMap->getEfficiency(clusId) < CLHEP::RandFlat::shoot(rndEngine, 0., 1.)){
                    continue;
                }
                if (!passDeadTime(clusId, hitTime(simHit), m_deadTime, deadTimes)) {
                    /// Reject hit within the dead time interval
                    continue;
                }
                NswErrorCalibData::Input errorCalibInput{};
                errorCalibInput.stripId = clusId;
                errorCalibInput.locTheta = M_PI- simHit->localDirection().theta();
                errorCalibInput.clusterAuthor=66; // cluster time projection method
                const double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);
                ATH_MSG_VERBOSE("mm hit has theta " << errorCalibInput.locTheta / Gaudi::Units::deg << " and uncertainty " << uncert);


                /// Pipe some dummy values to the digit to ensure that the pdo / tdo calibration 
                /// does not reject any hit downstream. Or in other words how much do you want... All charge
                constexpr int dummyResponseTime = 100;
                constexpr float dummyDepositedCharge = 666666;

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
                /// We're using the NSW uncertainty DB to smear the truth-hit positions using the best known
                /// uncertainties. In the process of digit -> RDO -> PRD, the smeared hit position is gonna be lost
                /// However, recall that the simplest way to generate a MM prd from RAW hits is the clustering by
                /// means of center of gravity. On average, a muon passage makes 3-4 strips fire. Assuming 3 strips,
                /// the smeared position can be written as
                ///
                ///             x = w_{1}*(c - p) + w_{2}*c + w_{3}*(c + p),
                /// 
                ///  where c is the strip centre, p the pitch and w_{i} are the induced charge fractions on each strip
                /// 
                ///            1 =  w_{1} + w_{2} + w_{3}
                ///
                ///            x = (w_{1} + w_{2} + w_{3}) * c + (w_{3} - w_{1})*p

                ///               --> x = c + (w_{3} - w_{1}) *p
                ///               --> (x-c) / p = (w_{3} - w_{1})  
                ///               --> (x-c) / p + w_{1} = w_{3}
                ///
                ///                  1 =  2 * w_{1} + w_{2} + (x-c) / p
                ///                  1 - (x-c) /p - 2*w_{1} = w_{2}
                ///                
                ///                  0< w_{1} < 0.5 - (x-c) / 2p 
                const double pull = (newLocalX - (*design.center(newChannel)).x()) / (design.stripPitch() * std::cos(design.stereoAngle()));
                const double w1 = CLHEP::RandFlat::shoot(rndEngine, 0., 0.5 *(1. - pull)); 
                const double w2 = 1 - pull -2*w1;
                const double w3 = pull + w1;
                MmDigitCollection* outColl = fetchCollection(hitId, digitCache);

                const Identifier digitIdB = idHelper.channelID(hitId, 
                                                               idHelper.multilayer(hitId), 
                                                               idHelper.gasGap(hitId), 
                                                               newChannel - 1, isValid);

                if (isValid) {
                    outColl->push_back(std::make_unique<MmDigit>(digitIdB, dummyResponseTime, w1 * dummyDepositedCharge));
                }
                outColl->push_back(std::make_unique<MmDigit>(digitId,dummyResponseTime, w2 * dummyDepositedCharge));

                const Identifier digitIdA = idHelper.channelID(hitId, 
                                                               idHelper.multilayer(hitId), 
                                                               idHelper.gasGap(hitId), 
                                                               newChannel + 1, isValid);
                if (isValid) {
                    outColl->push_back(std::make_unique<MmDigit>(digitIdA, dummyResponseTime, w3 * dummyDepositedCharge));
                }

                xAOD::MuonSimHit* sdoHit = addSDO(simHit, sdoContainer);
                sdoHit->setIdentifier(clusId);
                ++m_acceptedHits[hitGapInNsw];
            }
        } while(viewer.next());
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }   
}
