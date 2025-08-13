/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToPrepDataToolMT.h"

#include <algorithm>
#include <vector>

#include "GaudiKernel/PhysicalConstants.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "MdtRDO_Decoder.h"
#include "MuonPrepRawData/MdtTwinPrepData.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"
#include "MuonIdHelpers/IdentifierByDetElSorter.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleAuxContainer.h"


using namespace MuonGM;
using namespace Trk;

using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;


namespace {
    // the tube number of a tube in a tubeLayer is encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    constexpr unsigned int maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;

    inline void updateClosestApproachTwin(MdtCalibInput & in) {
        const MuonGM::MdtReadoutElement* descriptor = in.legacyDescriptor();
        if (descriptor) {
            if (std::abs(descriptor->getStationS()) < std::numeric_limits<double>::epsilon()) {
                return;
            }
            const Amg::Vector3D nominalTubePos = descriptor->tubePos(in.identify());
            double measuredPerp = std::sqrt(nominalTubePos.perp2() - descriptor->getStationS()* descriptor->getStationS());
            CxxUtils::sincos  tubeSC{nominalTubePos.phi()};
            Amg::Vector3D measurePos{tubeSC.cs * measuredPerp, tubeSC.sn *measuredPerp, nominalTubePos.z()};
            in.setClosestApproach(measurePos);
        }
    }
    inline std::string print(const Muon::MdtPrepData& prd) {
        const auto* idHelperSvc = prd.detectorElement()->idHelperSvc();
        std::stringstream sstr{};
        sstr<<" PrepData "<<idHelperSvc->toString(prd.identify())
            <<" radius: "<<prd.localPosition()[Trk::locR]<<" pm "
            <<std::sqrt(prd.localCovariance()(Trk::locR, Trk::locR))<<
            ", tdc: "<<prd.tdc()<<", adc: "<<prd.adc()<<", status: "<<prd.status();
        return sstr.str();
    }
}  // namespace

namespace Muon {

    
    MdtRdoToPrepDataToolMT::ConvCache::ConvCache(const Muon::IMuonIdHelperSvc* idHelperSvc):
        m_idHelperSvc{idHelperSvc} {
        addedCols.resize(m_idHelperSvc->mdtIdHelper().module_hash_max());
    }

    MdtPrepDataCollection* MdtRdoToPrepDataToolMT::ConvCache::createCollection(const Identifier& elementId) {

        IdentifierHash mdtHashId = m_idHelperSvc->moduleHash(elementId);

        std::unique_ptr<MdtPrepDataCollection>& coll {addedCols[mdtHashId]};
        if (!coll) {
            coll = std::make_unique<MdtPrepDataCollection>(mdtHashId);
            coll->setIdentifier(m_idHelperSvc->chamberId(elementId));
        }        
        return coll.get();
    }
    StatusCode MdtRdoToPrepDataToolMT::ConvCache::finalize(MsgStream& msg) {
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

        for (unsigned int moduleHash =0; moduleHash < addedCols.size(); ++moduleHash) {
            std::unique_ptr<MdtPrepDataCollection>& toInsert{addedCols[moduleHash]};
            if (!toInsert || toInsert->empty()) continue;
            if (xAODPrd) {
                /// Fill the prepdata objects just at this stage
                std::vector<const MdtPrepData*> sortMe{toInsert->begin(), toInsert->end()};
                std::ranges::sort(sortMe, IdentifierByDetElSorter{m_idHelperSvc});
                for (const MdtPrepData* prd : sortMe) {
                    const Identifier prdId{prd->identify()};
                    xAOD::MdtDriftCircle* dc{nullptr};
                    /// The prepdata represents an actual drift circle
                    if (!xAODTwinPrd|| !twinTubeMap || twinTubeMap->twinId(prdId) == prdId ||
                        prd->dimension() == 1) {
                        dc = xAODPrd->push_back(std::make_unique<xAOD::MdtDriftCircle>());
                    } else {
                        dc = xAODTwinPrd->push_back(std::make_unique<xAOD::MdtTwinDriftCircle>());                       
                    }
                    /// That method is kind of depreciated but needed for the Acts conversion test..
                    dc->setIdentifier(prdId.get_compact());
                    dc->setTdc(prd->tdc());
                    dc->setAdc(prd->adc());
                    dc->setTube(idHelper.tube(prdId));
                    dc->setLayer(idHelper.tubeLayer(prdId));
                    dc->setStatus(prd->status());
                    if (r4DetMgr){
                        dc->setReadoutElement(r4DetMgr->getMdtReadoutElement(prdId));
                    }
                    const IdentifierHash detHash{m_idHelperSvc->detElementHash(prdId)};
                    float driftRadius{0.f}, driftCov{0.f};
                    if (prd->status() == MdtDriftCircleStatus::MdtStatusDriftTime) {
                        driftRadius = prd->localPosition().x();
                        driftCov = prd->localCovariance()(0,0);
                    } else {
                        /// Invalid drift circles have a covariance as large as the inner radius assigned
                        const float maxR = r4DetMgr ? dc->readoutElement()->innerTubeRadius() 
                                                    : prd->detectorElement()->innerTubeRadius();
                        driftCov = std::pow(maxR, 2);
                    }
                    /// Ordinary 1D circle
                    if (dc->numDimensions() == 1) {
                        xAOD::MeasVector<1> locPos{driftRadius};
                        xAOD::MeasMatrix<1> locCov{driftCov};
                        dc->setMeasurement<1>(detHash, std::move(locPos), std::move(locCov));
                    } else {
                        xAOD::MeasMatrix<2> locCov{xAOD::MeasMatrix<2>::Identity()};
                        locCov(Trk::locR, Trk::locR) = prd->localCovariance()(Trk::locR, Trk::locR);
                        locCov(Trk::locZ, Trk::locZ) = prd->localCovariance()(Trk::locZ, Trk::locZ);
                        dc->setMeasurement<2>(detHash, xAOD::toStorage(prd->localPosition()), std::move(locCov));
                        auto* twinDC{static_cast<xAOD::MdtTwinDriftCircle*>(dc)};
                        auto* twinPRD{static_cast<const MdtTwinPrepData*>(prd)};
                        twinDC->setTwinAdc(twinPRD->adcTwin());
                        twinDC->setTwinTdc(twinPRD->tdcTwin());
                        const Identifier twinId = twinTubeMap->twinId(prdId);
                        twinDC->setTwinTube(idHelper.tube(twinId));
                        twinDC->setTwinLayer(idHelper.tubeLayer(twinId));
                    }
                }
            }
            MdtPrepDataContainer::IDC_WriteHandle lock = legacyPrd->getWriteHandle(moduleHash);
            if (lock.addOrDelete(std::move(toInsert)).isFailure()) {
                msg << MSG::ERROR << " Failed to add prep data collection " << moduleHash << endmsg;
                return StatusCode::FAILURE;
            }
        }
        if (xAODPrd) {
            xAODPrd->lock();
        }
        if (xAODTwinPrd) {
            xAODTwinPrd->lock();
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToPrepDataToolMT::initialize() {
        ATH_CHECK(m_calibrationTool.retrieve());
        ATH_MSG_VERBOSE("MdtCalibrationTool retrieved with pointer = " << m_calibrationTool);
        ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()));
        ATH_CHECK(m_idHelperSvc.retrieve());
        // Retrieve the RDO decoder
        ATH_CHECK(m_mdtDecoder.retrieve());

        ATH_CHECK(m_twinTubeKey.initialize(m_useTwin));
      
        m_BMGid = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
        m_BMGpresent = m_BMGid != -1;
        if (m_useNewGeo) {
            ATH_CHECK(detStore()->retrieve(m_detMgrR4));
        }
        if (m_BMGpresent && !m_useNewGeo) {
            const MuonGM::MuonDetectorManager* muDetMgr = nullptr;
            ATH_CHECK(detStore()->retrieve(muDetMgr));

            ATH_MSG_INFO("Processing configuration for layouts with BMG chambers.");

            for (int phi = 6; phi < 8; phi++) {                 // phi sectors
                for (int eta = 1; eta < 4; eta++) {             // eta sectors
                    for (int side = -1; side < 2; side += 2) {  // side
                        if (!muDetMgr->getMuonStation("BMG", side * eta, phi)) continue;
                        for (int roe = 1; roe <= (muDetMgr->getMuonStation("BMG", side * eta, phi))->nMuonReadoutElements();
                             roe++) {  // iterate on readout elemets
                            const MdtReadoutElement* mdtRE = dynamic_cast<const MdtReadoutElement*>(
                                (muDetMgr->getMuonStation("BMG", side * eta, phi))->getMuonReadoutElement(roe));  // has to be an MDT
                            if (mdtRE) initDeadChannels(mdtRE);
                        }
                    }
                }
            }
        } else if (m_useNewGeo) {
            std::vector<const MuonGMR4::MdtReadoutElement*> mdtRE = m_detMgrR4->getAllMdtReadoutElements();
            for (const MuonGMR4::MdtReadoutElement* re : mdtRE) {
                if (re->stationName() != m_BMGid) {
                    continue;
                }
                for (const IdentifierHash& dead : re->getParameters().removedTubes) {
                    m_DeadChannels.insert(re->measurementId(dead));
                }
            }
        }

        // check if initializing of DataHandle objects success
        ATH_CHECK(m_rdoContainerKey.initialize());
        ATH_CHECK(m_mdtPrepDataContainerKey.initialize());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_muDetMgrKey.initialize());
        ATH_CHECK(m_xAODKey.initialize(!m_xAODKey.empty()));
        ATH_CHECK(m_xAODTwinKey.initialize(!m_xAODTwinKey.empty()));
        ATH_CHECK(m_geoCtxKey.initialize(m_useNewGeo));
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToPrepDataToolMT::decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const {
        const MuonMDT_CablingMap* readCdo{nullptr};
        ATH_CHECK(SG::get(readCdo, m_readKey, ctx));
        return decode(ctx, readCdo->getMultiLayerHashVec(robIds, msgStream()));
    }

    const MdtCsmContainer* MdtRdoToPrepDataToolMT::getRdoContainer(const EventContext& ctx) const {
        SG::ReadHandle rdoContainerHandle{m_rdoContainerKey, ctx};
        if (rdoContainerHandle.isValid()) {
            ATH_MSG_DEBUG("MdtgetRdoContainer success");
            return rdoContainerHandle.cptr();
        }
        ATH_MSG_WARNING("Retrieval of Mdt RDO container failed !");
        return nullptr;
    }
    
    StatusCode MdtRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const{
        return setupMdtPrepDataContainer(ctx).isValid ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    void MdtRdoToPrepDataToolMT::processPRDHashes(const EventContext& ctx, ConvCache& mdtPrepDataContainer,
                                                 const std::vector<IdentifierHash>& multiLayerHashInRobs) const {
        for (const IdentifierHash& hash : multiLayerHashInRobs) {
            if (!handlePRDHash(ctx, mdtPrepDataContainer, hash)) { ATH_MSG_DEBUG("Failed to process hash " << hash); }
        }  // ends loop over chamberhash
    }

    bool MdtRdoToPrepDataToolMT::handlePRDHash(const EventContext& ctx,
                                               ConvCache& mdtPrepDataContainer,
                                               IdentifierHash rdoHash) const {
        const MdtCsmContainer* rdoContainer{getRdoContainer(ctx)};

        if (rdoContainer->empty()) {
            ATH_MSG_DEBUG("The container is empty");
            return true;
        }
        const MdtCsm* rdoColl = rdoContainer->indexFindPtr(rdoHash);
        if (!rdoColl) {
            ATH_MSG_DEBUG("The rdo container does not have the hash " << rdoHash);
            return true;
        }
        if (processCsm(ctx, mdtPrepDataContainer, rdoColl).isFailure()) {
            ATH_MSG_WARNING("processCsm failed for RDO id " << m_idHelperSvc->toString(rdoColl->identify()));
            return false;
        }
        return true;
    }

    StatusCode MdtRdoToPrepDataToolMT::decode(const EventContext& ctx, 
                                              const std::vector<IdentifierHash>& idVect) const {

        ATH_MSG_DEBUG("decodeMdtRDO for " << idVect.size() << " offline collections called");

        // setup output container
        ConvCache mdtPrepDataContainer = setupMdtPrepDataContainer(ctx);
        if (!mdtPrepDataContainer.isValid) { 
            return StatusCode::FAILURE; 
        }

      
        if (!m_decodeData) {
            ATH_MSG_DEBUG("Stored empty container. Decoding MDT RDO into MDT PrepRawData is switched off");
            return StatusCode::SUCCESS;
        }
        // seeded or unseeded decoding
        if (!idVect.empty()) {
            processPRDHashes(ctx, mdtPrepDataContainer, idVect);
        } else {
            /// Construct the hashes from the existing RDOs
            std::vector<IdentifierHash> rdoHashes{};
            const MdtCsmContainer* rdoContainer = getRdoContainer(ctx);
            if (!rdoContainer || rdoContainer->empty()) return StatusCode::SUCCESS;
            rdoHashes.reserve(rdoContainer->size());
            for (const MdtCsm* csm : *rdoContainer) rdoHashes.push_back(csm->identifyHash());

            processPRDHashes(ctx, mdtPrepDataContainer, rdoHashes);
        }
        ATH_CHECK(mdtPrepDataContainer.finalize(msgStream()));

        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MdtPrepData> MdtRdoToPrepDataToolMT::createPrepData(const MdtCalibInput& calibInput,
                                                                        const MdtCalibOutput& calibOutput,
                                                                        ConvCache& cache) const {
        if (calibInput.adc() < m_adcCut ||
            calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined) {
            ATH_MSG_VERBOSE("Do not create calib hit for "<<m_idHelperSvc->toString(calibInput.identify())
                            <<", adc: "<<calibInput.adc()<<" vs. "<<m_adcCut<<", calibration bailed out "
                          <<(calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined? "si": "no"));
            return nullptr;
        }
        const MuonGM::MdtReadoutElement* descriptor = calibInput.legacyDescriptor();
        if (!descriptor) {
            if (!cache.legacyDetMgr) {
                return nullptr;
            }
            descriptor = cache.legacyDetMgr->getMdtReadoutElement(calibInput.identify());
        }
        ATH_MSG_VERBOSE("Calibrated prepdata "<<m_idHelperSvc->toString(calibInput.identify())
                        <<std::endl<<calibInput<<std::endl<<calibOutput);

        Amg::Vector2D driftRadius{Amg::Vector2D::Zero()};
        Amg::MatrixX cov(1, 1);
        if (calibOutput.status() == MdtDriftCircleStatus::MdtStatusDriftTime){
            /// Test by how much do we break frozen Tier0
            const float r =  calibOutput.driftRadius();
            const float sigR = calibOutput.driftRadiusUncert();
            driftRadius[0] = r; 
            (cov)(0, 0) = sigR * sigR;
        } else (cov)(0, 0) = std::pow(descriptor->innerTubeRadius(), 2);
        
       return std::make_unique<MdtPrepData>(calibInput.identify(), 
                                            std::move(driftRadius), 
                                            std::move(cov), 
                                            descriptor,
                                            calibInput.tdc(),
                                            calibInput.adc(), 
                                            calibOutput.status());
    }

    StatusCode MdtRdoToPrepDataToolMT::processCsm(const EventContext& ctx, ConvCache& cache, const MdtCsm* rdoColl) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        // first handle the case of twin tubes
        if (m_useTwin) {
            if (cache.twinTubeMap->isTwinTubeLayer(rdoColl->identify())) {
                return processCsmTwin(ctx, cache, rdoColl);
            }
        }

        ATH_MSG_DEBUG(" ***************** Start of processCsm");

        /// MDT hit context
        const Identifier elementId = id_helper.parentID(rdoColl->identify());

        uint16_t subdetId = rdoColl->SubDetId();
        uint16_t mrodId = rdoColl->MrodId();
        uint16_t csmId = rdoColl->CsmId();
        ATH_MSG_VERBOSE("Identifier = " << m_idHelperSvc->toString(elementId) << " subdetId/ mrodId/ csmId = " << subdetId << " / "
                                        << mrodId << " / " << csmId);

        // for each Csm, loop over AmtHit, converter AmtHit to digit
        // retrieve/create digit collection, and insert digit into collection
        unsigned mc{0};

        for (const MdtAmtHit* amtHit : *rdoColl) {
            ++mc;

            // FIXME: Still use the digit class.
            ATH_MSG_VERBOSE("Amt Hit n. " << mc << " tdcId = " << amtHit->tdcId());
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(ctx, *amtHit, subdetId, mrodId, csmId)};
            if (!newDigit) {
                ATH_MSG_WARNING("Found issue MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }
            // Do something with it
            Identifier channelId = newDigit->identify();
            if (newDigit->isMasked() || m_DeadChannels.count(channelId)) {
                continue;
            }
            // Retrieve the proper PRD container. Note that there are cases where one CSM is either split into 2 chambers (BEE / BIS78
            // legacy) or 2 CSMs are split into one chamber
            MdtPrepDataCollection* driftCircleColl = cache.createCollection(channelId);
            if (!driftCircleColl) {
                ATH_MSG_DEBUG("Corresponding multi layer " << m_idHelperSvc->toString(channelId) << " is already decoded.");
                continue;
            }

            // check if the module ID of this channel is different from the CSM one
            // If it's the first case, create the additional collection

            ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                            << driftCircleColl->identifyHash());

            // Rescale ADC/TDC of chambers using HPTDC digitization chip
            // Must create a new digit from the old one, because MdtDigit has no methods to set ADC/TDC
            if (m_idHelperSvc->hasHPTDC(channelId)) {
                newDigit->setAdc(newDigit->adc() / 4);
                newDigit->setTdc(newDigit->tdc() / 4);
            }
            const MdtCalibInput calibIn = m_useNewGeo ? MdtCalibInput{*newDigit, *m_detMgrR4, *cache.gctx}: 
                                                        MdtCalibInput{*newDigit, *cache.legacyDetMgr};
            const MdtCalibOutput calibResult{m_calibrationTool->calibrate(ctx, calibIn, false)};

            std::unique_ptr<MdtPrepData> newPrepData = createPrepData(calibIn, calibResult, cache);
            if (!newPrepData) {
                continue;
            }
            if (driftCircleColl->size()) {
                MdtPrepData* prevPrd = driftCircleColl->at(driftCircleColl->size()-1);
                if (prevPrd->identify() == channelId) {
                    std::stringstream sstr{};
                    ATH_MSG_VERBOSE("Duplicated prep data object detected: "<<std::endl
                        <<"  **** "<<print(*prevPrd)<<std::endl
                        <<"  **** "<<print(*newPrepData));
                    if (prevPrd->status() == MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_VERBOSE("Prd is already good");
                    } else if (newPrepData->status() == MdtDriftCircleStatus::MdtStatusDriftTime) {
                        (*prevPrd) = std::move(*newPrepData);
                        prevPrd->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    }
                    continue;
                }
            }
            ATH_MSG_VERBOSE("New prd created "<<print(*newPrepData));
            newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
            driftCircleColl->push_back(std::move(newPrepData));
        }
        return StatusCode::SUCCESS;
    }
 StatusCode MdtRdoToPrepDataToolMT::processCsmTwin(const EventContext& ctx, ConvCache& cache, const MdtCsm* rdoColl) const {
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

        auto convertTwins = [this, &cache, &ctx](std::unique_ptr<MdtDigit> digit,
                                                 std::unique_ptr<MdtDigit> digit2) {
            if (!digit || digit->isMasked() || !cache.legacyDetMgr) {
                return;
            }

            MdtPrepDataCollection* driftCircleColl = cache.createCollection(digit->identify());
            
            if (!digit2 || digit2->isMasked()) {
                ATH_MSG_VERBOSE("Got single digit " << m_idHelperSvc->toString(digit->identify())<<", tdc: "
                             <<digit->tdc()<<", adc: "<<digit->adc() 
                             << ", hash: "<< driftCircleColl->identifyHash());

                const MdtCalibInput mdtCalibIn = m_useNewGeo ? MdtCalibInput{*digit, *m_detMgrR4, *cache.gctx}: 
                                                               MdtCalibInput{*digit, *cache.legacyDetMgr};

                const MdtCalibOutput mdtCalibOut{m_calibrationTool->calibrate(ctx, mdtCalibIn, false)};
                    
                /// Create new PrepData
                std::unique_ptr<MdtPrepData> newPrepData = createPrepData(mdtCalibIn, mdtCalibOut, cache);
                if (!newPrepData) return;
                
                newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                driftCircleColl->push_back(std::move(newPrepData));                
                return;
            } 
            ATH_MSG_VERBOSE("Twin digit calibration "<<m_idHelperSvc->toString(digit->identify())
                         <<", tdc: "<<digit->tdc()<<", adc: "<<digit->adc()<<" -- "
                         <<m_idHelperSvc->toString(digit2->identify())
                         <<", tdc: "<<digit2->tdc()<<", adc: "<<digit2->adc());
            MdtCalibInput mdtCalib1st = m_useNewGeo ? MdtCalibInput{*digit, *m_detMgrR4, *cache.gctx}
                                                    : MdtCalibInput{*digit, *cache.legacyDetMgr};
                
            MdtCalibInput mdtCalib2nd = m_useNewGeo ? MdtCalibInput{*digit2, *m_detMgrR4, *cache.gctx}
                                                    : MdtCalibInput{*digit2, *cache.legacyDetMgr};
                
            updateClosestApproachTwin(mdtCalib1st);
            updateClosestApproachTwin(mdtCalib2nd);

            const MdtCalibTwinOutput twinCalib = m_calibrationTool->calibrateTwinTubes(ctx, 
                                                                                       std::move(mdtCalib1st), 
                                                                                       std::move(mdtCalib2nd));

            Amg::Vector2D hitPos{twinCalib.primaryDriftR(), twinCalib.locZ()};
            Amg::MatrixX cov(2, 2);
            cov(0, 0) = twinCalib.uncertPrimaryR() * twinCalib.uncertPrimaryR();
            cov(1, 1) = twinCalib.sigmaZ() * twinCalib.sigmaZ();
            cov(0, 1) = cov(1, 0) = 0;
                
            const MuonGM::MdtReadoutElement* descriptor = cache.legacyDetMgr->getMdtReadoutElement(digit->identify());
            auto twin_newPrepData = std::make_unique<MdtTwinPrepData>(twinCalib.primaryID(),
                                                                      std::move(hitPos),
                                                                      std::move(cov),
                                                                      descriptor,
                                                                      twinCalib.primaryTdc(),
                                                                      twinCalib.primaryAdc(),
                                                                      twinCalib.twinTdc(),
                                                                      twinCalib.twinAdc(),
                                                                      twinCalib.primaryStatus());
                        
            ATH_MSG_VERBOSE(" MADE A 2D TWINPREPDATA " << m_idHelperSvc->toString(twinCalib.primaryID()) << " & "
                                                      << m_idHelperSvc->toString(twinCalib.twinID()) << " "<<twinCalib);
       
            ATH_MSG_VERBOSE("global pos center tube  " << Amg::toString(twin_newPrepData->globalPosition(), 2) << std::endl
                        <<"local pos center tube w/ TWIN INFO "<<Amg::toString(twinCalib.locZ() * Amg::Vector3D::UnitZ(), 2)<<std::endl
                        <<"global pos w/o TWIN INFO  "<<Amg::toString(descriptor->tubePos(twinCalib.primaryID())));

            twin_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
            driftCircleColl->push_back(std::move(twin_newPrepData));
        };

        // iterate over mdtDigitColl
        for (auto &[id,  digits] : mdtDigitColl) {
            // get the twin hits from mdtDigitColl
            const Identifier twinId = cache.twinTubeMap->twinId(id);
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
void MdtRdoToPrepDataToolMT::initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl) {
        PVConstLink cv = mydetEl->getMaterialGeom();  // it is "Multilayer"
        int nGrandchildren = cv->getNChildVols();
        if (nGrandchildren <= 0) return;

        std::vector<int> tubes;
        geoGetIds([&](int id) { tubes.push_back(id); }, &*cv);
        std::sort(tubes.begin(), tubes.end());

        const Identifier detElId = mydetEl->identify();
        const int ml = mydetEl->getMultilayer();
        std::vector<int>::iterator it = tubes.begin();
        for (int layer = 1; layer <= mydetEl->getNLayers(); layer++) {
            for (int tube = 1; tube <= mydetEl->getNtubesperlayer(); tube++) {
                int want_id = layer * maxNTubesPerLayer + tube;
                if (it != tubes.end() && *it == want_id) {
                    ++it;
                } else {
                    it = std::lower_bound(tubes.begin(), tubes.end(), want_id);
                    if (it != tubes.end() && *it == want_id) {
                        ++it;
                    } else {
                        Identifier deadTubeId = m_idHelperSvc->mdtIdHelper().channelID(detElId, ml, layer, tube);
                        m_DeadChannels.insert(deadTubeId);
                        ATH_MSG_VERBOSE("adding dead tube "<<m_idHelperSvc->toString(deadTubeId));
                    }
                }
            }
        }
        
    }
    MdtRdoToPrepDataToolMT::ConvCache MdtRdoToPrepDataToolMT::setupMdtPrepDataContainer(const EventContext& ctx) const {

        ConvCache cache{m_idHelperSvc.get()};

        SG::WriteHandle<MdtPrepDataContainer> handle{m_mdtPrepDataContainerKey, ctx};
        // Caching of PRD container
        if (m_prdContainerCacheKey.key().empty()) {
            // without the cache we just record the container
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container at " << m_mdtPrepDataContainerKey.key());
                return cache;
            }
            ATH_MSG_VERBOSE("Created container " << m_mdtPrepDataContainerKey.key());
            cache.legacyPrd = handle.ptr();
        } else {
            // use the cache to get the container
            SG::UpdateHandle update{m_prdContainerCacheKey, ctx};
            if (!update.isValid()) {
                ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
                return cache;
            }
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(update.ptr()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container using cache " << m_prdContainerCacheKey.key() << " - "
                                                                                                  << m_mdtPrepDataContainerKey.key());
                return cache;
            }
            ATH_MSG_VERBOSE("Created container using cache for " << m_prdContainerCacheKey.key());
            cache.legacyPrd = handle.ptr();
        }
        if (!m_xAODTwinKey.empty()) {
            SG::WriteHandle writeHandle{m_xAODTwinKey, ctx};
            if (!writeHandle.recordNonConst(std::make_unique<xAOD::MdtTwinDriftCircleContainer>(),
                                            std::make_unique<xAOD::MdtTwinDriftCircleAuxContainer>()).isSuccess() ||
                !writeHandle.isValid()) {
                ATH_MSG_FATAL("Failed to write xAOD::MdtPrepDataContainer "<<m_xAODTwinKey.fullKey());
                return cache;
            }
            cache.xAODTwinPrd = writeHandle.ptr();
        }
        if (!m_xAODKey.empty()) {
            SG::WriteHandle writeHandle{m_xAODKey, ctx};
            if (!writeHandle.recordNonConst(std::make_unique<xAOD::MdtDriftCircleContainer>(),
                                            std::make_unique<xAOD::MdtDriftCircleAuxContainer>()).isSuccess() ||
                !writeHandle.isValid()) {
                ATH_MSG_FATAL("Failed to write xAOD::MdtPrepDataContainer "<<m_xAODKey.fullKey());
                return cache;
            }
            cache.xAODPrd = writeHandle.ptr();
        }
        /// Retrieve the Geometry context if activated
        if (!m_geoCtxKey.empty()) {
            SG::ReadHandle readHandle{m_geoCtxKey, ctx};
            if (!readHandle.isPresent()) {
                ATH_MSG_FATAL("Failed to retrieve the geometry context "<<m_geoCtxKey.fullKey());
                return cache;
            }
            cache.gctx = readHandle.cptr();
        }
        /// Retrieve the legacy detector mananger if activated
        if (!m_muDetMgrKey.empty()) {
            SG::ReadCondHandle detMgrHandle{m_muDetMgrKey, ctx};
            if (!detMgrHandle.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve the detector manager from the conditions store "<<m_muDetMgrKey.fullKey());
                return cache;
            }
            cache.legacyDetMgr = detMgrHandle.cptr();
        }
        if (m_useTwin) {
            SG::ReadCondHandle twinTubeHandle{m_twinTubeKey, ctx};
            if (!twinTubeHandle.isValid()) {
                ATH_MSG_FATAL("Failed to initialize twin tube map "<<m_twinTubeKey.fullKey());
                return cache;
            }
            cache.twinTubeMap = twinTubeHandle.cptr();
        }
        cache.r4DetMgr = m_detMgrR4;
        // Pass the container from the handle
        cache.isValid = true;
        return cache;
    }
}  // namespace Muon
