/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtRdoToMdtPrepDataTool.h"

#include "MuonDigitContainer/MdtDigit.h"

using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;

namespace MuonR4{
    StatusCode MdtRdoToMdtPrepDataTool::initialize(){
        // initialize handle keys
        ATH_CHECK(m_calibrationTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_mdtDecoder.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgrR4));
        ATH_CHECK(m_twinTubeKey.initialize(m_useTwin));
        ATH_CHECK(m_rdoContainerKey.initialize());
        ATH_CHECK(m_cablingKey.initialize());
        ATH_CHECK(m_xAODKey.initialize());
        ATH_CHECK(m_xAODTwinKey.initialize(m_useTwin));
        ATH_CHECK(m_geoCtxKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToMdtPrepDataTool::decode(const EventContext& ctx, const std::vector<IdentifierHash>& idVect) const {
        ATH_MSG_DEBUG("MdtRdoToMdtPrepDataTool for " << idVect.size() << " offline collections called");
        HandleCache cache = setupHandleCache(ctx);
        if(!cache.isValid){
            ATH_MSG_ERROR("Unable to setup valid HandleCache object");
            return StatusCode::FAILURE;
        }
        // seeded or unseeded decoding
        if (!idVect.empty()) {
            ATH_CHECK(processPRDHashes(ctx, idVect, cache));
        } else {
            /// Construct the hashes from the existing RDOs
            std::vector<IdentifierHash> rdoHashes{};
            const MdtCsmContainer* rdoContainer{nullptr};
            ATH_CHECK(SG::get(rdoContainer, m_rdoContainerKey, ctx));

            if (rdoContainer->empty()) {
                return StatusCode::SUCCESS;
            }
            rdoHashes.reserve(rdoContainer->size());
            for (const MdtCsm* csm : *rdoContainer) { 
                rdoHashes.push_back(csm->identifyHash());
            }
            ATH_CHECK(processPRDHashes(ctx, rdoHashes, cache));
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToMdtPrepDataTool::decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const {
        const MuonMDT_CablingMap* cablingMap{nullptr};
        ATH_CHECK(SG::get(cablingMap, m_cablingKey, ctx));
        return decode(ctx, cablingMap->getMultiLayerHashVec(robIds, msgStream()));
    }

    StatusCode MdtRdoToMdtPrepDataTool::provideEmptyContainer(const EventContext& ctx) const {
        return setupHandleCache(ctx).isValid ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    // setter for the handle cache
    MdtRdoToMdtPrepDataTool::HandleCache MdtRdoToMdtPrepDataTool::setupHandleCache(const EventContext& ctx) const {
        HandleCache cache{};
        cache.isValid = cache.prdContainer.record(m_xAODKey, ctx).isSuccess() &&
                        cache.prdTwinContainer.record(m_xAODTwinKey, ctx).isSuccess() &&
                        SG::get(cache.gctx, m_geoCtxKey, ctx) == StatusCode::SUCCESS;
        return cache;
    }

    void MdtRdoToMdtPrepDataTool::processDigit(const EventContext& ctx, std::unique_ptr<MdtDigit> digit, HandleCache& hCache) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        Identifier channelId = digit->identify();
        const MuonGMR4::MdtReadoutElement* reEle = m_detMgrR4->getMdtReadoutElement(channelId);
        bool isValidChannel = reEle->isValid(reEle->measurementHash(channelId));

        if (digit->isMasked() || !isValidChannel) {
            return;
        }
        // Rescale ADC/TDC of chambers using HPTDC digitization chip
        // Must create a new digit from the old one, because MdtDigit has no methods to set ADC/TDC
        if (m_idHelperSvc->hasHPTDC(channelId)) {
            digit->setAdc(digit->adc() / 4);
            digit->setTdc(digit->tdc() / 4);
        }
        // if ADC cut too small, skip 
        if(digit->adc() < m_adcCut){
            return;
        }

        // calibration
        const MdtCalibInput calibIn = MdtCalibInput{*digit, *m_detMgrR4, *hCache.gctx};
        const MdtCalibOutput calibResult{m_calibrationTool->calibrate(ctx, calibIn, false)};

        // copying over create prep data over
        // if it doesn't match adc cut, skip!
        if(calibResult.status() == MdtDriftCircleStatus::MdtStatusUnDefined) {
            ATH_MSG_VERBOSE("Do not create calib hit for "<<m_idHelperSvc->toString(calibIn.identify())
                            <<", adc: "<<calibIn.adc()<<" vs. "<<m_adcCut<<", calibration bailed out "
                            <<(calibResult.status() == MdtDriftCircleStatus::MdtStatusUnDefined? "si": "no"));
            return;
        }
        ATH_MSG_VERBOSE("Calibrated prepdata "<<m_idHelperSvc->toString(calibIn.identify())
                        <<std::endl<<calibIn<<std::endl<<calibResult);
        double cov{0.}, driftRadius{0.};
        if (calibResult.status() == MdtDriftCircleStatus::MdtStatusDriftTime){
            driftRadius = calibResult.driftRadius();
            cov = Acts::square(calibResult.driftRadiusUncert());
        } else {
          cov = Acts::square(reEle->innerTubeRadius());
        }

        // setting things
        xAOD::MdtDriftCircle* dc = hCache.prdContainer->push_back(std::make_unique<xAOD::MdtDriftCircle>());
        const Identifier id = calibIn.identify();
        dc->setTdc(calibIn.tdc());
        dc->setAdc(calibIn.adc());
        dc->setTube(id_helper.tube(id));
        dc->setLayer(id_helper.tubeLayer(id));
        dc->setStatus(calibResult.status());
        dc->setReadoutElement(reEle);

        const IdentifierHash detHash{m_idHelperSvc->detElementHash(id)};
        xAOD::MeasVector<1> locPos{driftRadius};
        xAOD::MeasMatrix<1> locCov{cov};
        dc->setMeasurement<1>(detHash, std::move(locPos), std::move(locCov));
        return;
    }

    StatusCode MdtRdoToMdtPrepDataTool::processCsm(const EventContext& ctx, const MdtCsm* rdoColl, HandleCache& cache) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        // first handle the case of twin tubes
        if (m_useTwin) {
            const Muon::TwinTubeMap* twinTubeMap{};
            ATH_CHECK(SG::get(twinTubeMap, m_twinTubeKey, ctx));
            if (twinTubeMap->isTwinTubeLayer(rdoColl->identify())) {
                return processCsmTwin(ctx, rdoColl, cache);
            }
        }
        ATH_MSG_DEBUG(" ***************** Start of processCsm");
        // initialise write handle
        /// MDT hit context
        const Identifier elementId = id_helper.parentID(rdoColl->identify());

        uint16_t subdetId = rdoColl->SubDetId();
        uint16_t mrodId = rdoColl->MrodId();
        uint16_t csmId = rdoColl->CsmId();
        ATH_MSG_VERBOSE("Identifier = " << m_idHelperSvc->toString(elementId) << " subdetId/ mrodId/ csmId = " << subdetId << " / "
                                        << mrodId << " / " << csmId);

        // for each Csm, loop over AmtHit, converter AmtHit to digit
        // retrieve/create digit collection, and insert digit into collection
        for (const MdtAmtHit* amtHit : *rdoColl) {

            // FIXME: Still use the digit class.
            ATH_MSG_VERBOSE("Amt Hit, tdcId = " << amtHit->tdcId());
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(ctx, *amtHit, subdetId, mrodId, csmId)};
            if (!newDigit) {
                ATH_MSG_WARNING("Found issue MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }
            processDigit(ctx, std::move(newDigit), cache);
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToMdtPrepDataTool::processCsmTwin(const EventContext& ctx, const MdtCsm* rdoColl, HandleCache& cache) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        ATH_MSG_DEBUG(" ***************** Start of processCsmTwin");
        ATH_MSG_DEBUG(" Number of AmtHit in this Csm " << rdoColl->size());
        /// MDT hit context
        Identifier elementId = id_helper.parentID(rdoColl->identify());

        uint16_t subdetId = rdoColl->SubDetId();
        uint16_t mrodId = rdoColl->MrodId();
        uint16_t csmId = rdoColl->CsmId();
        ATH_MSG_VERBOSE("Identifier = " << m_idHelperSvc->toString(elementId) << " subdetId/ mrodId/ csmId = " << rdoColl->SubDetId()
                                        << " / " << rdoColl->MrodId() << " / " << rdoColl->CsmId());

        // for each Csm, loop over AmtHit, converter AmtHit to digit
        // retrieve/create digit collection, and insert digit into collection        
        std::map<Identifier, std::array<std::unique_ptr<MdtDigit>, 2>> mdtDigitColl{};

        // initialise twintube
        const Muon::TwinTubeMap* twinTubeMap{};
        ATH_CHECK(SG::get(twinTubeMap, m_twinTubeKey, ctx));

        for (const MdtAmtHit* amtHit : *rdoColl) {
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(ctx, *amtHit, subdetId, mrodId, csmId)};

            if (!newDigit) {
                ATH_MSG_WARNING("Error in MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }
            std::array<std::unique_ptr<MdtDigit>, 2> & moveTo = mdtDigitColl[newDigit->identify()];
            if (!moveTo[0]) {
                moveTo[0] = std::move(newDigit);
            } else if  (!moveTo[1] && !m_discardSecondaryHitTwin) {
                moveTo[1] = std::move(newDigit);
            } else {
                ATH_MSG_VERBOSE(" TWIN TUBES: found a tertiary hit in a twin tube in one RdoCollection for "
                            << m_idHelperSvc->toString(newDigit->identify()) << " with adc  = " << newDigit->adc()
                            << "  tdc = " << newDigit->tdc());
            }
        }  // end for-loop over rdoColl

        auto convertTwins = [&](std::unique_ptr<MdtDigit> digit,
                                std::unique_ptr<MdtDigit> digit2) {
            // if first digit is bad, return
            if (!digit || digit->isMasked()) {
                return;
            }
            // if second digit is bad, return first digit as a single drift circle
            if (!digit2 || digit2->isMasked()) {
                ATH_MSG_VERBOSE("Got single digit " << m_idHelperSvc->toString(digit->identify())<<", tdc: "
                            <<digit->tdc()<<", adc: "<<digit->adc());

                processDigit(ctx, std::move(digit), cache);
                return;
            } 
            ATH_MSG_VERBOSE("Twin digit calibration "<<m_idHelperSvc->toString(digit->identify())
                        <<", tdc: "<<digit->tdc()<<", adc: "<<digit->adc()<<" -- "
                        <<m_idHelperSvc->toString(digit2->identify())
                        <<", tdc: "<<digit2->tdc()<<", adc: "<<digit2->adc());
            MdtCalibInput mdtCalib1st = MdtCalibInput{*digit, *m_detMgrR4, *cache.gctx};
            MdtCalibInput mdtCalib2nd = MdtCalibInput{*digit2, *m_detMgrR4, *cache.gctx};
                
            const MdtCalibTwinOutput twinCalib = m_calibrationTool->calibrateTwinTubes(ctx, 
                                                                                    std::move(mdtCalib1st), 
                                                                                    std::move(mdtCalib2nd));

            Amg::Vector2D hitPos{twinCalib.primaryDriftR(), twinCalib.locZ()};
            AmgSymMatrix(2) cov{AmgSymMatrix(2)::Identity()};
            cov(0, 0) = twinCalib.uncertPrimaryR() * twinCalib.uncertPrimaryR();
            cov(1, 1) = twinCalib.sigmaZ() * twinCalib.sigmaZ();
                
            const MuonGMR4::MdtReadoutElement* descriptor = mdtCalib1st.decriptor();
            xAOD::MdtTwinDriftCircle* dc = cache.prdTwinContainer->push_back(std::make_unique<xAOD::MdtTwinDriftCircle>());

            const Identifier id = twinCalib.primaryID();
            const Identifier twin_id = twinCalib.twinID();
            const IdentifierHash detHash{m_idHelperSvc->detElementHash(id)};           
            // record things into xAOD
            dc->setTdc(twinCalib.primaryTdc());
            dc->setAdc(twinCalib.primaryAdc());
            dc->setTwinTdc(twinCalib.twinTdc());
            dc->setTwinAdc(twinCalib.twinAdc());
            dc->setTube(id_helper.tube(id));
            dc->setLayer(id_helper.tubeLayer(id));
            dc->setTwinTube(id_helper.tube(twin_id));
            dc->setTwinLayer(id_helper.tubeLayer(twin_id));
            dc->setStatus(twinCalib.primaryStatus());
            dc->setReadoutElement(descriptor);
            dc->setMeasurement<2>(detHash, xAOD::toStorage(hitPos), xAOD::toStorage(cov));
            
            ATH_MSG_VERBOSE(" MADE A 2D TWINPREPDATA " << m_idHelperSvc->toString(twinCalib.primaryID()) << " & "
                                                    << m_idHelperSvc->toString(twinCalib.twinID()) << " "<<twinCalib);
        
            ATH_MSG_VERBOSE("local pos center tube w/ TWIN INFO "<<Amg::toString(twinCalib.locZ() * Amg::Vector3D::UnitZ(), 2)<<std::endl);

        };

        // iterate over mdtDigitColl
        for (auto &[id,  digits] : mdtDigitColl) {
            // get the twin hits from mdtDigitColl
            const Identifier twinId = twinTubeMap->twinId(id);
            /// check for the twin tube id
            if (id != twinId) {
                std::array<std::unique_ptr<MdtDigit>, 2>& twinDigits = mdtDigitColl[twinId];
                ATH_MSG_VERBOSE("Convert digits: "<<digits[0].get()<<" "<<twinDigits[0].get());
                convertTwins(std::move(digits[0]), std::move(twinDigits[0]));
                ATH_MSG_VERBOSE("Convert digits: "<<digits[1].get()<<" "<<twinDigits[1].get());
                convertTwins(std::move(digits[1]), std::move(twinDigits[1]));
            } else {
                convertTwins(std::move(digits[0]), nullptr);
                convertTwins(std::move(digits[1]), nullptr);
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToMdtPrepDataTool::processPRDHashes(const EventContext& ctx, const std::vector<IdentifierHash>& multiLayerHashInRobs, HandleCache& cache) const{
        // looping through each chamber
        const MdtCsmContainer* rdoContainer{nullptr};
        ATH_CHECK(SG::get(rdoContainer, m_rdoContainerKey, ctx));
        if (rdoContainer->empty()) {
             ATH_MSG_DEBUG("The container is empty");
             return StatusCode::SUCCESS;
        }

        for (const IdentifierHash& hash : multiLayerHashInRobs) {
            const MdtCsm* rdoColl = rdoContainer->indexFindPtr(hash);
            if(!rdoColl){
                ATH_MSG_DEBUG("The rdo container does not have the hash " << hash);
                continue;
            }
            ATH_CHECK(processCsm(ctx, rdoColl, cache));
        }
        return StatusCode::SUCCESS;
    }
}
