/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include "MuCalDecode/CalibData.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuCalDecode/CalibUti.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"

#include <iostream>
#include <list>
#include <algorithm>
#include <map>

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "MuonCalibStreamCnv/RpcCalibRawDataProvider.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"

using namespace LVL2_MUON_CALIBRATION;


RpcCalibRawDataProvider::RpcCalibRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode RpcCalibRawDataProvider::initialize() {

    ATH_MSG_INFO("RpcCalibRawDataProvider::initialize");

    // retrieve common tools
    ATH_CHECK(m_detectorManagerKey.initialize());    
    ATH_CHECK(m_muonIdHelper.retrieve());
    ATH_CHECK(m_rpcReadKey.initialize());

    // retrieve the dataProviderSvc
    ATH_CHECK(m_dataProvider.retrieve());

    // setup output RPC container keys
    ATH_CHECK(m_rdoContainerKey.initialize());   

    return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute
StatusCode RpcCalibRawDataProvider::execute(const EventContext& ctx) const {

    ATH_MSG_INFO("RpcCalibRawDataProvider::execute");

    const CalibEvent *event = m_dataProvider->getEvent();

    // // setup output write handle for RpcPadContainer
    SG::WriteHandle<RpcPadContainer> handle{m_rdoContainerKey, ctx};
    ATH_CHECK(handle.record(std::make_unique<RpcPadContainer>(m_muonIdHelper->rpcIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Created container " << m_rdoContainerKey.key());    
        // Pass the container from the handle
    RpcPadContainer *padContainer = handle.ptr();

    ATH_CHECK(decodeImpl(ctx, padContainer, event));
    ATH_MSG_DEBUG("RPC core decode processed in MT decode (calibration stream event)");
    
    return StatusCode::SUCCESS;
}

StatusCode RpcCalibRawDataProvider::decodeImpl(const EventContext& ctx,  RpcPadContainer *padContainer, const CalibEvent *event) const {

    // decoding process from https://gitlab.cern.ch/atlas-mcp/MdtCalib/mdtcalibframework/-/blob/master/MuonCalibStream/MuonCalibStreamCnv/src/RpcRDOContCalibStreamCnv.cxx
    // to be verified with run3 data

    if (!event->rpc()) {
            ATH_MSG_DEBUG("NO RPC hits!");
            return StatusCode::SUCCESS;
        }
        SG::ReadCondHandle<RpcCablingCondData> readHandle{m_rpcReadKey, ctx};
        const RpcCablingCondData *cabling{*readHandle};

        //  RpcPadIdHash hashF;

        // extract the list of RPC pads
        ATH_MSG_DEBUG("extract the list of RPC pads");
        std::list<RpcCalibData> calib_pad = (event->rpc())->data();

        ATH_MSG_DEBUG("fillCollections: number of PADS= " << (event->rpc())->size());
        ATH_MSG_DEBUG("fillCollections: number of matrices= " << calib_pad.size());

        // std::vector<RpcPad*>* m_rpcpads;
        // Iterate on the readout PADS
        std::vector<std::unique_ptr<RpcPad>> rpcPadCollections{};

        ATH_MSG_DEBUG("Iterate on the readout PADS");
        int max_pad = 1;
        for (int j = 0; j < max_pad; ++j) {
            // Identifier elements
            int name{0}, eta{0}, phi{0}, doublet_r{0}, doublet_z{0}, doublet_phi{0}, gas_gap{0}, measures_phi{0}, strip{0};

            unsigned int sector = (event->rpc())->sectorId();
            unsigned int subsys = (event->rpc())->subsystemId();
            unsigned int pad_id = (event->rpc())->padId();
            unsigned int status = (event->rpc())->status();
            unsigned int errorCode = (event->rpc())->error_code();

            /// Where is this bit documented?
            if (subsys == 0x65) sector = sector + 32;

            ATH_MSG_DEBUG("fillCollections: pad no= " << j << " sector= " << sector << " subsys= " << subsys << " pad id= " << pad_id
                                                      << " status= " << status << " error code= " << errorCode);

            int side = (sector < 32) ? 0 : 1;
            int logic_sector = sector % 32;
            int key = side * 10000 + logic_sector * 100 + pad_id;

            // Retrieve the identifier elements from the map
            const RpcCablingCondData::RDOmap &pad_map = cabling->give_RDOs();
            RDOindex index = (*pad_map.find(key)).second;
            index.offline_indexes(name, eta, phi, doublet_r, doublet_z, doublet_phi, gas_gap, measures_phi, strip);

            // Build the pad offline identifier
            Identifier id = m_muonIdHelper->rpcIdHelper().padID(name, eta, phi, doublet_r, doublet_z, doublet_phi);

            std::unique_ptr<RpcPad> newpad = std::make_unique<RpcPad>(id, index.hash(), pad_id, status, errorCode, sector);

            // iterate on the matrices
            ATH_MSG_DEBUG("Iterate on the matrices");
            for (RpcCalibData &calib_matrix : calib_pad) {
                unsigned int cmid = calib_matrix.onlineId();
                unsigned int crc = calib_matrix.crc();
                unsigned int fel1id = calib_matrix.fel1Id();
                unsigned int febcid = calib_matrix.febcId();
                ATH_MSG_DEBUG("fillCollections: matrix no= " << cmid << " crc= " << crc << " fel1id= " << fel1id << " febcid= " << febcid);
                std::unique_ptr<RpcCoinMatrix> matrix = std::make_unique<RpcCoinMatrix>(id, cmid, crc, fel1id, febcid);

                // iterate on the fired channels
                ATH_MSG_DEBUG("Iterate on the fired channels");
                for (short unsigned int i = 0; i < calib_matrix.hitNum(); ++i) {
                    std::unique_ptr<RpcFiredChannel> rpcfiredchannel = nullptr ;
                    uint16_t bcid{0}, time{0}, ijk{0}, channel{0}, ovl{0}, thr{0};
                    calib_matrix.giveHit(i, bcid, time, ijk, channel, ovl, thr);

                    ATH_MSG_DEBUG("Check BIS78 updates? " << __FILE__ << " " << __LINE__);
                    if (ijk < 7) {
                        rpcfiredchannel = std::make_unique<RpcFiredChannel>(bcid, time, ijk, channel);

                    } else if (ijk == 7) {
                        rpcfiredchannel = std::make_unique<RpcFiredChannel>(bcid, time, ijk, thr, ovl);

                    }
                    ATH_MSG_DEBUG("fillCollections: hit no= " << i << " bcid= " << bcid << " time= " << time << " ijk= " << ijk
                                                              << " channel=" << channel << " ovl= " << ovl << " thr= " << thr);
                    matrix->push_back(std::move(rpcfiredchannel));
                    ATH_MSG_DEBUG("fillCollections: hit added to the matrix");
                }  // end iterate on the fired channels
                // add the matrix to the pad
                newpad->push_back(std::move(matrix));
                ATH_MSG_DEBUG("fillCollections: matrix added to the pad");
            }  // end iterate on the matrices

            // Push back the decoded pad in the vector

            rpcPadCollections.push_back(std::move(newpad));            

            //      m_rpcpads->push_back(newpad);
            ATH_MSG_DEBUG("fillCollections: pad added to the pad vector");
        }  // end iterate on the readout PADS

        for (auto& coll : rpcPadCollections) {
            if (!coll) continue;
            const Identifier padId = coll->identify();
            IdentifierHash elementHash = m_muonIdHelper->moduleHash(padId);
            RpcPadContainer::IDC_WriteHandle lock = padContainer->getWriteHandle(elementHash);
            ATH_CHECK(lock.addOrDelete(std::move(coll)));
        }
        ATH_MSG_DEBUG("Wrote " << rpcPadCollections.size() << " RPC PAD");

        return StatusCode::SUCCESS;
    }
