/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtDigitizationTool.h"
#include "StoreGate/WriteHandle.h"
#include "MDT_Digitization/MdtDigiToolInput.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
namespace{
    constexpr double timeToTdcCnv = 1. / IMdtCalibrationTool::tdcBinSize;
}

namespace MuonR4 {
    
    MdtDigitizationTool::MdtDigitizationTool(const std::string& type, const std::string& name, const IInterface* pIID):
        MuonDigitizationTool{type,name, pIID} {}

    StatusCode MdtDigitizationTool::initialize() {
        ATH_CHECK(MuonDigitizationTool::initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_calibDbKey.initialize());
        ATH_CHECK(m_badTubeKey.initialize());
        ATH_CHECK(m_twinTubeKey.initialize(m_useTwinTube));
        return StatusCode::SUCCESS;
    }
    StatusCode MdtDigitizationTool::digitize(const EventContext& ctx,
                                             const TimedHits& hitsToDigit,
                                             xAOD::MuonSimHitContainer* sdoContainer) const {
        
      
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        // Prepare the temporary cache
        
        using DigitSDOPair = std::pair<std::unique_ptr<MdtDigit>, TimedHit>;
        /// Fetch the needed conditions 
        const MuonCalib::MdtCalibDataContainer* calibData{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_calibDbKey, calibData));
        const MdtCondDbData* badTubes{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_badTubeKey, badTubes));
        const Muon::TwinTubeMap* twinTubes{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_twinTubeKey, twinTubes));
        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
        
        DigiCache digitCache{};

        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {
            Identifier lastTube{};
            double deadTime{0.};
            std::vector<DigitSDOPair> digitsInChamber{};
        
            for (const TimedHitPtr<xAOD::MuonSimHit>& simHit : viewer) {
                const Identifier hitId{simHit->identify()};
                /// Hit is masked in the database for whatever reason
                if (!badTubes->isGood(hitId)) {
                    ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" is rejected due to masking in DB.");
                    continue;
                }
                ATH_MSG_VERBOSE("Process sim hit "<<m_idHelperSvc->toString(hitId) <<", pdgId: "<<simHit->pdgId()<<", "
                            <<Amg::toString(xAOD::toEigen(simHit->localPosition()))<<" + "<<Amg::toString(xAOD::toEigen(simHit->localDirection()))
                            <<", time: "<<simHit->globalTime()<<", energy: "<<simHit->kineticEnergy() / Gaudi::Units::GeV
                            <<" [GeV], mass: "<<simHit->mass()<<", deposited energy: "<<simHit->energyDeposit() / Gaudi::Units::eV
                            <<" [eV], genLink: "<<simHit->genParticleLink());
                const MuonGMR4::MdtReadoutElement* readOutEle = m_detMgr->getMdtReadoutElement(hitId);
                const IdentifierHash measHash{readOutEle->measurementHash(hitId)};
                if (m_digitizeMuonOnly && std::abs(simHit->pdgId()) != 13) {
                    ATH_MSG_VERBOSE("Hit is not from a muon");
                    continue;
                }

                const Amg::Vector3D locPos{xAOD::toEigen(simHit->localPosition())};
                const double distRO = readOutEle->distanceToReadout(measHash, locPos);

                const MdtDigiToolInput digiInput(std::abs(locPos.perp()), distRO, 0., 0., 0., 0., hitId);
                const MdtDigiToolOutput digiOutput(m_digiTool->digitize(ctx, digiInput, rndEngine));
                if (!digiOutput.wasEfficient()) {
                    ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" is rejected due to inefficiency modelling.");
                    continue;
                }
                const double arrivalTime{simHit->globalTime()};

                const MuonCalib::MdtFullCalibData* tubeConstants = calibData->getCalibData(hitId, msgStream());
                assert(tubeConstants != nullptr);
                assert(tubeConstants->tubeCalib.get()  != nullptr);
                assert(tubeConstants->tubeCalib->getCalib(hitId) != nullptr);

                const MuonCalib::MdtTubeCalibContainer::SingleTubeCalib& tubeCalib{*tubeConstants->tubeCalib->getCalib(hitId)};
            
                const double sigPropTime = calibData->inversePropSpeed()*distRO;
                ATH_MSG_VERBOSE(m_idHelperSvc->toString(hitId)<<" "<<Amg::toString(locPos)<<" distance to readout: "<<distRO<<" --> "<<sigPropTime);
                /// Total tdc time is the sum of the drift time, the time of flight of the muon, the propgation along the wire
                /// and finally the constant t0 tube offset
                const double totalTdcTime = digiOutput.driftTime() + arrivalTime + tubeCalib.t0 + sigPropTime;
                if (lastTube != hitId || deadTime < totalTdcTime) {
                    lastTube = hitId;
                    deadTime = totalTdcTime + m_deadTime;
                } else {
                    ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" has been produced within dead time");
                    continue;
                }
                /// Question: What happens if there're 2 digits such close by that their adc needs to be merged?

                const bool hasHPTdc = m_idHelperSvc->hasHPTDC(hitId);
                /// The HPTdc has 4 times higher clock frequency. Smear both 
                const uint16_t tdcCounts = timeToTdcCnv*(hasHPTdc ? 4 : 1)*CLHEP::RandGaussZiggurat::shoot(rndEngine, totalTdcTime, m_timeResTDC);
                const uint16_t adcCounts = (hasHPTdc ? 4 : 1) *CLHEP::RandGaussZiggurat::shoot(rndEngine, digiOutput.adc(), m_timeResADC);

                auto digit = std::make_unique<MdtDigit>(hitId, tdcCounts, adcCounts, false);
                ATH_MSG_VERBOSE("Add digit "<<m_idHelperSvc->toString(digit->identify())<<", tdc: "<<digit->tdc()<<", adc: "<<digit->adc());
                digitsInChamber.push_back(std::make_pair(std::move(digit), simHit));
                /// Put also in the twin tube digit
                if (!twinTubes || !twinTubes->isTwinTubeLayer(hitId)) {
                    continue;
                }
                const Identifier twinId{twinTubes->twinId(hitId)};
                if (twinId == hitId) {
                    ATH_MSG_VERBOSE("The hit "<<m_idHelperSvc->toString(hitId)<<" has no twins.");
                    continue;
                }
                const IdentifierHash twinHash{readOutEle->measurementHash(twinId)};
                if (!readOutEle->isValid(twinHash)) {
                    ATH_MSG_VERBOSE("Reject "<<m_idHelperSvc->toString(twinId)<<" as there's no tube ");
                    continue;
                }
                /// The signal for the twin tube needs to travel to the HV side & then through the complete twin tube
                const double twinDist = readOutEle->activeTubeLength(measHash) - distRO + 
                                        readOutEle->activeTubeLength(twinHash);
            
                ATH_MSG_VERBOSE("Twin hit :"<<m_idHelperSvc->toString(twinId)<< ", local Z: "<<simHit->localPosition().z()
                              <<", tube length: "<<readOutEle->activeTubeLength(measHash)<<", distRO: "<<distRO
                              <<", twin length: "<<readOutEle->activeTubeLength(twinHash)
                              <<", twin distance: "<<twinDist);
                const MuonCalib::MdtTubeCalibContainer::SingleTubeCalib& twinCalib{*tubeConstants->tubeCalib->getCalib(twinId)};
            
                const double twinPropTime = calibData->inversePropSpeed()*twinDist;
                /// Total tdc time is the sum of the drift time, the time of flight of the muon, the propgation along the wire
                /// and finally the constant t0 tube offset
                const double twinTdcTime = digiOutput.driftTime() + arrivalTime + twinPropTime 
                                         + twinCalib.t0 + twinTubes->hvDelayTime(twinId);

                const uint16_t twinTdcCounts = timeToTdcCnv*(hasHPTdc ? 4 : 1)*CLHEP::RandGaussZiggurat::shoot(rndEngine, twinTdcTime, m_resTwin);
                const uint16_t twinAdcCoutns = (hasHPTdc ? 4 : 1) *CLHEP::RandGaussZiggurat::shoot(rndEngine, digiOutput.adc(), m_timeResADC);
                digit = std::make_unique<MdtDigit>(twinId, twinTdcCounts, twinAdcCoutns, false);
                ATH_MSG_VERBOSE("Add twin digit "<<m_idHelperSvc->toString(digit->identify())<<", tdc: "<<digit->tdc()
                                <<", adc: "<<digit->adc()<<", local z: "<<simHit->localPosition().z());
                digitsInChamber.push_back(std::make_pair(std::move(digit), simHit));
            }
            /// The digitization might have produced digits which are overlapping in time, in particular if twin tubes are hit.
            /// Filter tube digits which are produced within the dead time interval
            std::ranges::sort(digitsInChamber,[](const DigitSDOPair&a, const DigitSDOPair& b){
                        if (a.first->identify() != b.first->identify()) {
                           return a.first->identify() < b.first->identify();
                        }
                        return a.first->tdc() < b.first->tdc();
                    });
      
            for (std::vector<DigitSDOPair>::iterator saveMe = digitsInChamber.begin(); saveMe != digitsInChamber.end() ; ) {
                addSDO(saveMe->second, sdoContainer);
                const MdtDigit* saved = fetchCollection(saveMe->first->identify(), digitCache)->push_back(std::move(saveMe->first));
                const uint16_t deadInterval = saved->tdc() + timeToTdcCnv*(m_idHelperSvc->hasHPTDC(saved->identify()) ?4 : 1)*m_deadTime;
                /// Find the next digit which is either another tube or beyond the dead time
                saveMe = std::find_if(saveMe +1, digitsInChamber.end(),
                                      [deadInterval, saved](const DigitSDOPair& digitized) {
                                         return saved->identify() != digitized.first->identify() || deadInterval < digitized.first->tdc();
                                      });
            }
        } while (viewer.next());
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }
}
