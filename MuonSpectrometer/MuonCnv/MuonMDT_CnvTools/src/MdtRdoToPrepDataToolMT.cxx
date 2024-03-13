/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToPrepDataToolMT.h"

#include <algorithm>
#include <vector>

#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "MdtRDO_Decoder.h"
#include "MuonPrepRawData/MdtTwinPrepData.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"

using namespace MuonGM;
using namespace Trk;

using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;


namespace {
    // the tube number of a tube in a tubeLayer is encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    constexpr unsigned int maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;

    inline void updateClosestApproachTwin(MdtCalibInput & in) {
        const MuonGM::MdtReadoutElement* descriptor = in.legacyDescriptor();
        if (std::abs(descriptor->getStationS()) < std::numeric_limits<double>::epsilon()) {
            return;
        }
        const Amg::Vector3D nominalTubePos = descriptor->tubePos(in.identify());        
        double measuredPerp = std::sqrt(nominalTubePos.perp2() - descriptor->getStationS()* descriptor->getStationS());
        CxxUtils::sincos  tubeSC{nominalTubePos.phi()};
        Amg::Vector3D measurePos{tubeSC.cs * measuredPerp, tubeSC.sn *measuredPerp, nominalTubePos.z()};
        in.setClosestApproach(measurePos);
    }
}  // namespace

namespace Muon {

    MdtPrepDataCollection* MdtRdoToPrepDataToolMT::ModfiablePrdColl::createCollection(const Identifier& elementId,
                                                                                        const MdtIdHelper& id_helper, MsgStream& msg) {
        IdentifierHash mdtHashId{0};
        if (id_helper.get_module_hash(elementId, mdtHashId)) {
            msg << MSG::ERROR << "Module hash creation failed. " << elementId << endmsg;
            return nullptr;
        }
        PrdCollMap::iterator itr = addedCols.find(mdtHashId);
        if (itr != addedCols.end()) return itr->second.get();
        MdtPrepDataContainer::IDC_WriteHandle lock = prd_cont->getWriteHandle(mdtHashId);
        if (lock.alreadyPresent()) {
            if (msg.level() <= MSG::DEBUG) {
                msg << MSG::DEBUG << "MdtPrepDataCollection already contained in IDC " << elementId << " " << mdtHashId << endmsg;
            }
            return nullptr;
        }
        std::unique_ptr<MdtPrepDataCollection> newColl = std::make_unique<MdtPrepDataCollection>(mdtHashId);
        newColl->setIdentifier(id_helper.elementID(elementId));
        return addedCols.insert(std::make_pair(mdtHashId, std::move(newColl))).first->second.get();
    }
    StatusCode MdtRdoToPrepDataToolMT::ModfiablePrdColl::finalize(std::vector<IdentifierHash>& prdHashes, MsgStream& msg) {
        for (auto& to_insert : addedCols) {
            if (to_insert.second->empty()) continue;
            MdtPrepDataContainer::IDC_WriteHandle lock = prd_cont->getWriteHandle(to_insert.first);
            if (lock.addOrDelete(std::move(to_insert.second)).isFailure()) {
                msg << MSG::ERROR << " Failed to add prep data collection " << to_insert.first << endmsg;
                return StatusCode::FAILURE;
            }
            prdHashes.emplace_back(to_insert.first);
        }
        return StatusCode::SUCCESS;
    }

    MdtRdoToPrepDataToolMT::MdtRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p) :
        base_class(t, n, p) {}

    StatusCode MdtRdoToPrepDataToolMT::initialize() {
        ATH_CHECK(AthAlgTool::initialize());
        const MuonGM::MuonDetectorManager* muDetMgr = nullptr;
        ATH_CHECK(detStore()->retrieve(muDetMgr));
        ATH_CHECK(m_calibrationTool.retrieve());
        ATH_MSG_VERBOSE("MdtCalibrationTool retrieved with pointer = " << m_calibrationTool);
        ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()));
        ATH_CHECK(m_idHelperSvc.retrieve());
        // Retrieve the RDO decoder
        ATH_CHECK(m_mdtDecoder.retrieve());

        // + TWIN TUBES
        // make an array of [multilayer][layer][twin-pair]; 2 multilayers, 3 layer per multilayer, 36 twin-pairs per layer
        if (m_useTwin) {
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 36; k++) {
                        // fill m_twin_chamber array with unique numbers
                        m_twin_chamber[i][j][k] = 1000 * i + 100 * j + k;
                        // for secondary hits we need to make a second array with unique numbers
                        // (i+1 is used in the expression, so numbers are always different from m_twin_chamber array)
                        m_secondaryHit_twin_chamber[i][j][k] = 10000 * (i + 1) + 100 * j + k;
                    }
                }
            }
        }  // end if(m_useTwin){
        // - TWIN TUBES


        m_BMGid = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
        m_BMGpresent = m_BMGid != -1;
        if (m_BMGpresent) {
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
        }

        // check if initializing of DataHandle objects success
        ATH_CHECK(m_rdoContainerKey.initialize());
        ATH_CHECK(m_mdtPrepDataContainerKey.initialize());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_muDetMgrKey.initialize());
        ATH_CHECK(m_mdtxAODKey.initialize(!m_mdtxAODKey.empty()));
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToPrepDataToolMT::decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const {
        SG::ReadCondHandle<MuonMDT_CablingMap> readHandle{m_readKey, ctx};
        const MuonMDT_CablingMap* readCdo{*readHandle};
        if (!readCdo) {
            ATH_MSG_ERROR("nullptr to the read conditions object");
            return StatusCode::FAILURE;
        }
        return decode(ctx, readCdo->getMultiLayerHashVec(robIds, msgStream()));
    }

    const MdtCsmContainer* MdtRdoToPrepDataToolMT::getRdoContainer(const EventContext& ctx) const {
        SG::ReadHandle<MdtCsmContainer> rdoContainerHandle{m_rdoContainerKey, ctx};
        if (rdoContainerHandle.isValid()) {
            ATH_MSG_DEBUG("MdtgetRdoContainer success");
            return rdoContainerHandle.cptr();
        }
        ATH_MSG_WARNING("Retrieval of Mdt RDO container failed !");
        return nullptr;
    }
    
    StatusCode MdtRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const{
        return setupMdtPrepDataContainer(ctx).prd_cont ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    StatusCode MdtRdoToPrepDataToolMT::decode(const EventContext& ctx, const std::vector<IdentifierHash>& multiLayerHashInRobs) const {
        // setup output container
        ModfiablePrdColl mdtPrepDataContainer = setupMdtPrepDataContainer(ctx);
        if (!mdtPrepDataContainer.prd_cont) { return StatusCode::FAILURE; }

        SG::WriteHandle<xAOD::MdtDriftCircleContainer> outputContainer;
        if (!m_mdtxAODKey.empty()) {
            outputContainer = SG::WriteHandle<xAOD::MdtDriftCircleContainer>(m_mdtxAODKey, ctx);
        }
        auto mdtxAODPrepDataContainer = setupxAODMdtPrepDataContainer(outputContainer);

        if (!m_decodeData) {
            ATH_MSG_DEBUG("Stored empty container. Decoding MDT RDO into MDT PrepRawData is switched off");
            return StatusCode::SUCCESS;
        }

        // left unused, needed by other decode function and further down the code.
        std::vector<IdentifierHash> idWithDataVect;
        processPRDHashes(ctx, mdtPrepDataContainer, mdtxAODPrepDataContainer, multiLayerHashInRobs);
        ATH_CHECK(mdtPrepDataContainer.finalize(idWithDataVect, msgStream()));
        return StatusCode::SUCCESS;
    }  // end decode

    void MdtRdoToPrepDataToolMT::processPRDHashes(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                                                    xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer,
                                                    const std::vector<IdentifierHash>& multiLayerHashInRobs) const {
        for (const IdentifierHash& hash : multiLayerHashInRobs) {
            if (!handlePRDHash(ctx, mdtPrepDataContainer, xAODMdtPrepDataContainer, hash)) { ATH_MSG_DEBUG("Failed to process hash " << hash); }
        }  // ends loop over chamberhash
    }

    bool MdtRdoToPrepDataToolMT::handlePRDHash(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                                                xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer,
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
        SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgr{m_muDetMgrKey, ctx};
        if (!muDetMgr.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Muon detector manager " << m_muDetMgrKey.fullKey());
            return false;
        }

        if (processCsm(ctx, mdtPrepDataContainer, xAODMdtPrepDataContainer, rdoColl, *muDetMgr).isFailure()) {
            ATH_MSG_WARNING("processCsm failed for RDO id " << m_idHelperSvc->toString(rdoColl->identify()));
            return false;
        }
        return true;
    }

    StatusCode MdtRdoToPrepDataToolMT::decode(const EventContext& ctx, 
                                              std::vector<IdentifierHash>& idVect, 
                                              std::vector<IdentifierHash>& idWithDataVect) const {
        // clear output vector of selected data collections containing data
        idWithDataVect.clear();
        ATH_MSG_DEBUG("decodeMdtRDO for " << idVect.size() << " offline collections called");

        // setup output container
        ModfiablePrdColl mdtPrepDataContainer = setupMdtPrepDataContainer(ctx);
        if (!mdtPrepDataContainer.prd_cont) { return StatusCode::FAILURE; }

        SG::WriteHandle<xAOD::MdtDriftCircleContainer> outputContainer;
        if (!m_mdtxAODKey.empty()) {
            outputContainer = SG::WriteHandle<xAOD::MdtDriftCircleContainer>(m_mdtxAODKey, ctx);
        }
        auto mdtxAODPrepDataContainer = setupxAODMdtPrepDataContainer(outputContainer);

        if (!m_decodeData) {
            ATH_MSG_DEBUG("Stored empty container. Decoding MDT RDO into MDT PrepRawData is switched off");
            return StatusCode::SUCCESS;
        }
        // seeded or unseeded decoding
        if (!idVect.empty()) {
            processPRDHashes(ctx, mdtPrepDataContainer, mdtxAODPrepDataContainer, idVect);
        } else {
            /// Construct the hashes from the existing RDOs
            std::vector<IdentifierHash> rdoHashes{};
            const MdtCsmContainer* rdoContainer = getRdoContainer(ctx);
            if (!rdoContainer || rdoContainer->empty()) return StatusCode::SUCCESS;
            rdoHashes.reserve(rdoContainer->size());
            for (const MdtCsm* csm : *rdoContainer) rdoHashes.push_back(csm->identifyHash());

            processPRDHashes(ctx, mdtPrepDataContainer, mdtxAODPrepDataContainer, rdoHashes);
        }
        ATH_CHECK(mdtPrepDataContainer.finalize(idWithDataVect, msgStream()));

        return StatusCode::SUCCESS;
    }

    // dump the RDO in input
    void MdtRdoToPrepDataToolMT::printInputRdo(const EventContext& ctx) const {
        ATH_MSG_DEBUG("******************************************************************************************");
        ATH_MSG_DEBUG("***************** Listing MdtCsmContainer collections content ********************************");

        const MdtCsmContainer* rdoContainer = getRdoContainer(ctx);

        if (rdoContainer->empty()) ATH_MSG_DEBUG("MdtCsmContainer is Empty");

        ATH_MSG_DEBUG("-----------------------------------------------------------------------------");

        unsigned int ncsm{0}, namt{0};
        // loop on the MdtCsm collections
        for (const MdtCsm* mdtColl : *rdoContainer) {
            ++ncsm;
            ATH_MSG_DEBUG("**** MdtCsm with online Id: subdetector: " << MSG::hex << mdtColl->SubDetId() << MSG::dec
                                                                      << "  mrod: " << MSG::hex << mdtColl->MrodId() << MSG::dec
                                                                      << "  csmid: " << MSG::hex << mdtColl->CsmId() << MSG::dec);
            ATH_MSG_DEBUG("  number of mdt hits: " << mdtColl->size());

            // loop on the hits of the CSM
            for (const MdtAmtHit* amtHit : *mdtColl) {
                ++namt;
                ATH_MSG_DEBUG(">> AmtHit in tdc: " << MSG::hex << amtHit->tdcId() << MSG::dec << "  channel: " << MSG::hex
                                                   << amtHit->channelId() << MSG::dec << "  fine time: " << amtHit->fine()
                                                   << "  coarse time: " << amtHit->coarse() << "  width: " << amtHit->width());
            }
        }

        ATH_MSG_DEBUG("*** Event Summary: csm collections:" << ncsm << "  amt hits: " << namt);
   }

    void MdtRdoToPrepDataToolMT::printPrepDataImpl(const MdtPrepDataContainer* mdtPrepDataContainer) const {
        // Dump info about PRDs
        ATH_MSG_DEBUG("******************************************************************************************");
        ATH_MSG_DEBUG("***************** Listing MdtPrepData collections content ********************************");

        if (mdtPrepDataContainer->empty()) ATH_MSG_DEBUG("No MdtPrepRawData collections found");
        int ncoll{0}, nhits{0};
        ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
        for (const MdtPrepDataCollection* mdtColl : *mdtPrepDataContainer) {
            int nhitcoll = mdtColl->size();
            nhits += mdtColl->size();
            ++ncoll;
            ATH_MSG_DEBUG("PrepData Collection ID " << m_idHelperSvc->toString(mdtColl->identify()));
                
            for (const MdtPrepData* prepData : *mdtColl) {
                ATH_MSG_DEBUG(" in this coll. " << nhitcoll << " prepData id = " << m_idHelperSvc->toString(prepData->identify())
                                                << " tdc/adc =" << prepData->tdc() << "/" << prepData->adc());
            }
            ATH_MSG_DEBUG("*** Collection " << ncoll << " Summary: N. hits = " << nhitcoll);
            ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
        }
        ATH_MSG_DEBUG("*** Event  Summary: " << ncoll << " Collections / " << nhits << " hits  ");
        ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
    }

    std::unique_ptr<MdtPrepData> MdtRdoToPrepDataToolMT::createPrepData(const MdtCalibInput& calibInput,
                                                                        const MdtCalibOutput& calibOutput) const {
        if (!calibInput.legacyDescriptor() || calibInput.isMasked() || calibInput.adc() < m_adcCut ||
            calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined) {
            ATH_MSG_VERBOSE("Do not create calib hit for "<<m_idHelperSvc->toString(calibInput.identify())<<
                            " because it's masked "<<(calibInput.isMasked() ? "si" : "no") <<", "
                          <<"adc: "<<calibInput.adc()<<" vs. "<<m_adcCut<<", calibration bailed out "
                          <<(calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined? "si": "no"));
            return nullptr;
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
        } else (cov)(0, 0) = 0;
        
        return std::make_unique<MdtPrepData>(calibInput.identify(), 
                                             calibInput.legacyDescriptor()->identifyHash(), 
                                             std::move(driftRadius), 
                                             std::move(cov), 
                                             calibInput.legacyDescriptor(),
                                             calibInput.tdc(),
                                             calibInput.adc(), 
                                             calibOutput.status());
    }

    void MdtRdoToPrepDataToolMT::createxAODPrepData(const MdtCalibInput& calibInput,
                                            const MdtCalibOutput& calibOutput, 
                                            xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer) const {
      if (!calibInput.legacyDescriptor() || calibInput.isMasked() ||
          calibInput.adc() < m_adcCut ||
          calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined) {
        ATH_MSG_VERBOSE(
            "Do not create calib hit for "
            << m_idHelperSvc->toString(calibInput.identify())
            << " because it's masked " << (calibInput.isMasked() ? "si" : "no")
            << ", "
            << "adc: " << calibInput.adc() << " vs. " << m_adcCut
            << ", calibration bailed out "
            << (calibOutput.status() == MdtDriftCircleStatus::MdtStatusUnDefined
                    ? "si"
                    : "no"));
        return;
      }
      ATH_MSG_VERBOSE("Calibrated xAOD prepdata "
                      << m_idHelperSvc->toString(calibInput.identify())
                      << std::endl
                      << calibInput << std::endl
                      << calibOutput);

      xAOD::MeasVector<1> driftRadius{0.0};
      xAOD::MeasMatrix<1> cov{0.0};

      if (calibOutput.status() == MdtDriftCircleStatus::MdtStatusDriftTime) {
        /// Test by how much do we break frozen Tier0
        const float r = calibOutput.driftRadius();
        const float sigR = calibOutput.driftRadiusUncert();
        driftRadius[0] = r;
        (cov)(0, 0) = sigR * sigR;
      } else
        (cov)(0, 0) = 0;

      auto xprd = new xAOD::MdtDriftCircle();
      xAODMdtPrepDataContainer->push_back(xprd);

      const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

      const Identifier prdId{calibInput.identify()};
      xprd->setIdentifier(calibInput.identify().get_identifier32().get_compact());

      xprd->setMeasurement(calibInput.legacyDescriptor()->identifyHash(), driftRadius,
                          cov);

      xprd->setTdc(calibInput.tdc());
      xprd->setAdc(calibInput.adc());
      xprd->setTube(id_helper.tube(prdId));
      xprd->setLayer(id_helper.tubeLayer(prdId));
      xprd->setStatus(calibOutput.status());
      // TODO tubePosInStation - but this needs ReadoutElement?

      return;                                        
    }

    StatusCode MdtRdoToPrepDataToolMT::processCsm(const EventContext& ctx, ModfiablePrdColl& prepDataContainer, 
                                                  xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer,
                                                  const MdtCsm* rdoColl,
                                                  const MuonGM::MuonDetectorManager* muDetMgr) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        // first handle the case of twin tubes
        if (m_useTwin) {
            // two chambers in ATLAS are installed with Twin Tubes; in detector coordinates BOL4A13 & BOL4C13; only INNER multilayer(=1) is
            // with twin tubes implement twin tube writing to prepData either for all BOL (m_useAllBOLTwin = true) _OR_ only for two
            // chambers really installed
            Identifier elementId = rdoColl->identify();
            MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(elementId);
            if (chIndex == MuonStationIndex::BOL &&
                (m_useAllBOLTwin || (std::abs(id_helper.stationEta(elementId)) == 4 && id_helper.stationPhi(elementId) == 7))) {
                return processCsmTwin(ctx, prepDataContainer, xAODMdtPrepDataContainer, rdoColl, muDetMgr);
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
        int mc = 0;
        for (const MdtAmtHit* amtHit : *rdoColl) {
            mc++;

            // FIXME: Still use the digit class.
            ATH_MSG_VERBOSE("Amt Hit n. " << mc << " tdcId = " << amtHit->tdcId());
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(amtHit, subdetId, mrodId, csmId)};
            if (!newDigit) {
                ATH_MSG_WARNING("Found issue MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }
            // Do something with it
            Identifier channelId = newDigit->identify();
            if (deadBMGChannel(channelId)) continue;

            // Retrieve the proper PRD container. Note that there are cases where one CSM is either split into 2 chambers (BEE / BIS78
            // legacy) or 2 CSMs are split into one chamber
            MdtPrepDataCollection* driftCircleColl = prepDataContainer.createCollection(channelId, id_helper, msgStream());
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
                int adc = newDigit->adc() / 4;
                int tdc = newDigit->tdc() / 4;
                int mask = newDigit->is_masked();
                newDigit = std::make_unique<MdtDigit>(channelId, tdc, adc, mask);
                ATH_MSG_DEBUG("Change HPTDC ADC/TDC " << m_idHelperSvc->toString(channelId) << " Old ADC/TDC=" << adc * 4 << " " << tdc * 4
                                                      << " New=" << adc << " " << tdc);
            }
            const MdtCalibInput calibIn{*newDigit, *muDetMgr};
            const MdtCalibOutput calibResult{m_calibrationTool->calibrate(ctx, calibIn, false)};

            std::unique_ptr<MdtPrepData> newPrepData = createPrepData(calibIn, calibResult);
            if (!newPrepData) continue;

            newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
            driftCircleColl->push_back(std::move(newPrepData));

            if (xAODMdtPrepDataContainer) createxAODPrepData(calibIn, calibResult, xAODMdtPrepDataContainer);
        }
        return StatusCode::SUCCESS;
    }
    bool MdtRdoToPrepDataToolMT::deadBMGChannel(const Identifier& channelId) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        if (id_helper.stationName(channelId) != m_BMGid || !m_BMGpresent) return false;
        std::map<Identifier, std::set<Identifier>>::const_iterator myIt = m_DeadChannels.find(id_helper.multilayerID(channelId));
        if (myIt == m_DeadChannels.end()) { return false; }
        if (myIt->second.count(channelId)) {
            ATH_MSG_DEBUG("deadBMGChannel : Deleting BMG digit with identifier" << m_idHelperSvc->toString(channelId));
            return true;
        }
        return false;
    }
    StatusCode MdtRdoToPrepDataToolMT::processCsmTwin(const EventContext& ctx, ModfiablePrdColl& prepDataContainer, 
                                                      xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer, const MdtCsm* rdoColl,
                                                      const MuonGM::MuonDetectorManager* muDetMgr) const {
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

        // make a map to be filled for every twin-pair
        //   std::map<int, std::vector<MdtDigit*> > mdtDigitColl;

        using twin_digit = std::pair<std::unique_ptr<MdtDigit>, std::unique_ptr<MdtDigit>>;
        std::map<int, twin_digit> mdtDigitColl;

        for (const MdtAmtHit* amtHit : *rdoColl) {
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(amtHit, subdetId, mrodId, csmId)};

            if (!newDigit) {
                ATH_MSG_WARNING("Error in MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }

            // make an Identifier
            Identifier channelId = newDigit->identify();
            // IdentifierHash channelHash = newDigit->identifyHash();
            if (deadBMGChannel(channelId)) continue;

            // get tube params
            int tube = id_helper.tube(channelId);
            int layer = id_helper.tubeLayer(channelId);
            int multilayer = id_helper.multilayer(channelId);

            // find the correct twin-pair (tube-1 & tube-3 are twin pair 1, tube-2 & tube-4 are twin pair 2)
            int twinPair = -1;
            if (tube % 4 == 1) {
                twinPair = (tube + 1) / 2;
            } else if (tube % 4 == 3) {
                twinPair = (tube - 1) / 2;
            } else if (tube % 4 == 2) {
                twinPair = (tube + 2) / 2;
            } else {
                twinPair = tube / 2;
            }  // tube%4 == 0

            // fill the digitColl map
            twin_digit& pair = mdtDigitColl[m_twin_chamber[multilayer - 1][layer - 1][twinPair - 1]];
            if (!pair.first) {
                pair.first = std::move(newDigit);
            } else if (!pair.second) {
                pair.second = std::move(newDigit);
            }
            // if a secondary hit appears in a tube add it to mdtDigitColl, unless m_discardSecondaryHitTwin flag is true
            else {
                ATH_MSG_VERBOSE(" TWIN TUBES: found a secondary(not twin) hit in a twin tube");
                twin_digit& secondPair = mdtDigitColl[m_secondaryHit_twin_chamber[multilayer - 1][layer - 1][twinPair - 1]];
                if (!m_discardSecondaryHitTwin) {
                    if (!secondPair.first) {
                        secondPair.first = std::move(newDigit);
                    } else if (!secondPair.second) {
                        secondPair.second = std::move(newDigit);
                    } else {
                        ATH_MSG_VERBOSE(" TWIN TUBES: found a tertiary hit in a twin tube in one RdoCollection for "
                                        << m_idHelperSvc->toString(channelId) << " with adc  = " << newDigit->adc()
                                        << "  tdc = " << newDigit->tdc());
                    }
                }  // end --   if(!m_discardSecondaryHitTwin){
                else {
                    ATH_MSG_DEBUG(
                        " TWIN TUBES: discarding secondary(non-twin) hit in a twin tube as flag m_discardSecondaryHitTwin is set to true");
                }
            }
        }  // end for-loop over rdoColl

        // iterate over mdtDigitColl
        for (std::pair<const int, twin_digit>& digitPair : mdtDigitColl) {
            // get the twin hits from mdtDigitColl
            std::unique_ptr<MdtDigit>& digit = digitPair.second.first;
            std::unique_ptr<MdtDigit>& second_digit = digitPair.second.second;

            if (!digit) {
                ATH_MSG_FATAL("nullptr to a digit ");
                return StatusCode::FAILURE;
            }

            // Do something with it
            Identifier channelId = digit->identify();
            int multilayer = id_helper.multilayer(channelId);

            MdtPrepDataCollection* driftCircleColl = prepDataContainer.createCollection(channelId, id_helper, msgStream());
            if (!driftCircleColl) {
                ATH_MSG_DEBUG("Corresponding multi layer " << m_idHelperSvc->toString(channelId) << " is already decoded.");
                continue;
            }

            // check if the hit is in multilayer=1
            // two chambers in ATLAS are installed with Twin Tubes; in detector coordinates BOL4A13 & BOL4C13; only INNER multilayer(=1) is
            // with twin tubes
            if (multilayer == 1) {
                // if no twin hit present in data, use standard PRD making
                if (!second_digit) {
                    ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                                    << driftCircleColl->identifyHash());

                    const MdtCalibInput mdtCalibIn{*digit, *muDetMgr};
                    const MdtCalibOutput mdtCalibOut{m_calibrationTool->calibrate(ctx, mdtCalibIn, false)};
                    
                    // Create new PrepData
                    std::unique_ptr<MdtPrepData> newPrepData = createPrepData(mdtCalibIn, mdtCalibOut);
                    if (!newPrepData) continue;

                    newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(std::move(newPrepData));

                    if (xAODMdtPrepDataContainer) createxAODPrepData(mdtCalibIn, mdtCalibOut, xAODMdtPrepDataContainer);
                    

                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA " << m_idHelperSvc->toString(channelId) << " " << mdtCalibOut);
                    continue;
                }
                if (digit->is_masked() || second_digit->is_masked()) continue;
                MdtCalibInput mdtCalib1st{*digit, *muDetMgr};
                MdtCalibInput mdtCalib2nd{*second_digit, *muDetMgr};
                updateClosestApproachTwin(mdtCalib1st);
                updateClosestApproachTwin(mdtCalib2nd);

                const MdtCalibTwinOutput twinCalib = m_calibrationTool->calibrateTwinTubes(ctx, mdtCalib1st, mdtCalib2nd);

                Amg::Vector2D hitPos{twinCalib.primaryDriftR(), twinCalib.locZ()};
                Amg::MatrixX cov(2, 2);
                (cov)(0, 0) = twinCalib.uncertPrimaryR() * twinCalib.uncertPrimaryR();
                (cov)(1, 1) = twinCalib.sigmaZ() * twinCalib.sigmaZ();
                (cov)(0, 1) = 0;
                (cov)(1, 0) = 0;
                
                const MuonGM::MdtReadoutElement* descriptor = mdtCalib1st.legacyDescriptor();
                std::unique_ptr<MdtTwinPrepData> twin_newPrepData = std::make_unique<MdtTwinPrepData>(twinCalib.primaryID(),
                                                                                                      descriptor->identifyHash(),
                                                                                                      std::move(hitPos),
                                                                                                      std::move(cov),
                                                                                                      descriptor,
                                                                                                      twinCalib.primaryTdc(),
                                                                                                      twinCalib.primaryAdc(),
                                                                                                      twinCalib.twinTdc(),
                                                                                                      twinCalib.twinAdc(),
                                                                                                      twinCalib.primaryStatus());
                        
                ATH_MSG_DEBUG(" MADE A 2D TWINPREPDATA " << m_idHelperSvc->toString(twinCalib.primaryID()) << " & "
                                                         << m_idHelperSvc->toString(twinCalib.twinID()) << " "<<twinCalib);
                Amg::Vector3D gpos_centertube = twin_newPrepData->globalPosition();
               
                Amg::Vector3D locpos_centertube = twinCalib.locZ() * Amg::Vector3D::UnitZ();
                const Amg::Vector3D gpos_twin = descriptor->localToGlobalTransf(twinCalib.primaryID())*locpos_centertube;

                ATH_MSG_DEBUG("global pos center tube  " << Amg::toString(gpos_centertube, 2) << std::endl
                            <<"local pos center tube w/ TWIN INFO "<<Amg::toString(locpos_centertube, 2)<<std::endl
                            <<"global pos w/ TWIN INFO  "<<Amg::toString(gpos_twin));

                twin_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                driftCircleColl->push_back(std::move(twin_newPrepData));

                   
            }      // end -- if(multilayer==1)
            else if (multilayer == 2) {
                // if multilayer=2, then treat every hit as a separate hit, no twin hit should be present here as the hardware is not
                // installed

                const MdtCalibInput calibInput1st{*digit, *muDetMgr};
                const MdtCalibOutput calibResult1st{m_calibrationTool->calibrate(ctx, calibInput1st, false)};
                // Create new PrepData
                std::unique_ptr<MdtPrepData> newPrepData = createPrepData(calibInput1st, calibResult1st);
                if (newPrepData) {
                    newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(std::move(newPrepData));
                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA " << m_idHelperSvc->toString(channelId) << " "<<calibResult1st);
                }
                if (xAODMdtPrepDataContainer) createxAODPrepData(calibInput1st, calibResult1st, xAODMdtPrepDataContainer);
                
                if (!second_digit) continue;
                    // Calculate radius
                    
                const MdtCalibInput calibInput2nd{*second_digit, *muDetMgr};
                const MdtCalibOutput calibResult2nd{m_calibrationTool->calibrate(ctx, calibInput2nd, false)};

                // second_digit
                // Create new PrepData
                std::unique_ptr<MdtPrepData> second_newPrepData = createPrepData(calibInput2nd, calibResult2nd);
                if (!second_newPrepData) continue;
                second_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                driftCircleColl->push_back(std::move(second_newPrepData));
                if (xAODMdtPrepDataContainer) createxAODPrepData(calibInput2nd, calibResult2nd, xAODMdtPrepDataContainer);
                
                // second_digit
                ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA FOR SECOND DIGIT " 
                              << m_idHelperSvc->toString(calibInput2nd.identify()) 
                              << " "<<calibResult2nd);
              
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
        std::set<Identifier>& deadTubes{m_DeadChannels[detElId]};
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
                        deadTubes.insert(deadTubeId);
                        ATH_MSG_VERBOSE("adding dead tube "<<m_idHelperSvc->toString(deadTubeId));
                    }
                }
            }
        }
        
    }
    MdtRdoToPrepDataToolMT::ModfiablePrdColl MdtRdoToPrepDataToolMT::setupMdtPrepDataContainer(const EventContext& ctx) const {
        SG::WriteHandle<MdtPrepDataContainer> handle{m_mdtPrepDataContainerKey, ctx};

        // Caching of PRD container
        if (m_prdContainerCacheKey.key().empty()) {
            // without the cache we just record the container
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container at " << m_mdtPrepDataContainerKey.key());
                return ModfiablePrdColl{};
            }
            ATH_MSG_DEBUG("Created container " << m_mdtPrepDataContainerKey.key());
        } else {
            // use the cache to get the container
            SG::UpdateHandle<MdtPrepDataCollection_Cache> update{m_prdContainerCacheKey, ctx};
            if (!update.isValid()) {
                ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
                return ModfiablePrdColl{};
            }
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(update.ptr()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container using cache " << m_prdContainerCacheKey.key() << " - "
                                                                                                  << m_mdtPrepDataContainerKey.key());
                return ModfiablePrdColl{};
            }
            ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
        }
        // Pass the container from the handle
        return ModfiablePrdColl{handle.ptr()};
    }

    xAOD::MdtDriftCircleContainer*
    MdtRdoToPrepDataToolMT::setupxAODMdtPrepDataContainer(SG::WriteHandle<xAOD::MdtDriftCircleContainer>& outputContainer) const {

        if (!m_mdtxAODKey.empty() && 
                outputContainer.record(std::make_unique<xAOD::MdtDriftCircleContainer>(),
                        std::make_unique<xAOD::MdtDriftCircleAuxContainer>())
                .isSuccess()) {
            ATH_MSG_DEBUG(
                "Recorded xAOD container with key: " << outputContainer.key());
            return outputContainer.ptr();
        } 
        return nullptr;
    }

    void Muon::MdtRdoToPrepDataToolMT::printPrepData(const EventContext& ctx ) const {
        SG::ReadHandleKey<Muon::MdtPrepDataContainer> k(m_mdtPrepDataContainerKey.key());
        k.initialize().ignore();
        printPrepDataImpl(SG::makeHandle(k, ctx).get());
    }

}  // namespace Muon
