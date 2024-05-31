/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtDigitizationTool.h"
#include "StoreGate/WriteHandle.h"
#include "MDT_Digitization/MdtDigiToolInput.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
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
        return StatusCode::SUCCESS;
    }
    StatusCode MdtDigitizationTool::digitize(const EventContext& ctx,
                                             const TimedHits& hitsToDigit,
                                             xAOD::MuonSimHitContainer* sdoContainer) const {
        
      
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        // Prepare the temporary cache
        DigiCache digitCache{};
        /// Fetch the needed conditions 
        const MuonCalib::MdtCalibDataContainer* calibData{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_calibDbKey, calibData));
        const MdtCondDbData* badTubes{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_badTubeKey, badTubes));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        double deadTime{0.};
        for (const TimedHitPtr<xAOD::MuonSimHit>& simHit : hitsToDigit) {
            const Identifier hitId{simHit->identify()};
            /// Hit is masked in the database for whatever reason
            if (!badTubes->isGood(hitId)) {
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" is rejected due to masking in DB.");
                continue;
            }
            const MuonGMR4::MdtReadoutElement* readOutEle = m_detMgr->getMdtReadoutElement(hitId);
            const IdentifierHash measHash{readOutEle->measurementHash(hitId)};


            const Amg::Vector3D locPos{xAOD::toEigen(simHit->localPosition())};

            const double distRO = std::abs(0.5*readOutEle->getParameters().readoutSide*readOutEle->activeTubeLength(measHash) - locPos.z());

            
            const MdtDigiToolInput digiInput(std::abs(locPos.perp()), distRO, 0., 0., 0., 0., hitId);

            const MdtDigiToolOutput digiOutput(m_digiTool->digitize(ctx, digiInput, rndEngine));
            if (!digiOutput.wasEfficient()) {
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" is rejected due to inefficiency modelling.");
                continue;
            }

            const MuonCalib::MdtFullCalibData* tubeConstants = calibData->getCalibData(hitId, msgStream());
            const MuonCalib::MdtTubeCalibContainer::SingleTubeCalib& tubeCalib{*tubeConstants->tubeCalib->getCalib(hitId)};
            
            const double sigPropTime = tubeCalib.inversePropSpeed*distRO;
            /// Total tdc time is the sum of the drift time, the time of flight of the muon, the propgation along the wire
            /// and finally the constant t0 tube offset
            const double totalTdcTime = digiOutput.driftTime() + simHit->globalTime() + sigPropTime + tubeCalib.t0;
            
            MdtDigitCollection* outColl = fetchCollection(hitId, digitCache);
            if (outColl->empty() || outColl->back()->identify() != hitId || totalTdcTime > deadTime) {
                deadTime = totalTdcTime + m_deadTime;
            } else if (totalTdcTime <= deadTime) {
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitId)<<" is within dead time "<<deadTime<<" totalTdcTime: "<<totalTdcTime);
                continue;
            }

            const bool hasHPTdc = m_idHelperSvc->hasHPTDC(hitId);
            /// The HPTdc has 4 times higher clock frequency. Smear both 
            const uint16_t tdcCounts = timeToTdcCnv*(hasHPTdc ? 4 : 1)*CLHEP::RandGaussZiggurat::shoot(rndEngine, totalTdcTime, m_timeResTDC);
            const uint16_t adcCounts = (hasHPTdc ? 4 : 1) *CLHEP::RandGaussZiggurat::shoot(rndEngine, digiOutput.adc(), m_timeResADC);

            auto digit = std::make_unique<MdtDigit>(hitId, tdcCounts, adcCounts);
            outColl->push_back(std::move(digit));
            addSDO(simHit, sdoContainer);
        }
        /// Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
        return StatusCode::SUCCESS;
    }
}