/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MDT_Hid2RESrcID.h"

#include "AthenaKernel/getMessageSvc.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "eformat/SourceIdentifier.h"

using eformat::helper::SourceIdentifier;

// implementation of RpcPad to RESrcID conversion.
// Ketevi A. Assamagan
// Jan-16-2003, BNL
//
// adapted for MdtCms by Michela Biglietti <biglietti@na.infn.it>

MDT_Hid2RESrcID::MDT_Hid2RESrcID() : m_mdtIdHelper(nullptr), m_specialROBNumber(0), m_readKey("MuonMDT_CablingMap") {}

StatusCode MDT_Hid2RESrcID::set(const MdtIdHelper* mdtId) {
    // Initialize the cabling Service
    m_mdtIdHelper = mdtId;
    ATH_CHECK(m_readKey.initialize());
    return StatusCode::SUCCESS;
}

uint32_t MDT_Hid2RESrcID::getRodID(const Identifier& offlineId) const {
    // this method returns a RESrcID for the ROD, for a
    // given MDT ID offline ID

    MsgStream log(Athena::getMessageSvc(), "MDT_Hid2RESrcID::getRodID");

    log << MSG::DEBUG << "Getting RODId of the Station " << MSG::hex << offlineId << MSG::dec << endmsg;

    SG::ReadCondHandle<MuonMDT_CablingMap> readHandle{m_readKey};
    const MuonMDT_CablingMap* readCdo{*readHandle};
    if (!readCdo) {
        // ATH_MSG_ERROR("Null pointer to the read conditions object");
        log << MSG::ERROR << "Null pointer to the read conditions object" << endmsg;
        return 0;
    }
    MuonMDT_CablingMap::CablingData cabling_data{};
    if (!readCdo->convert(offlineId, cabling_data)) {
        log << MSG::ERROR << " Invalid identifier given " << endmsg;
        return 0;
    }
    bool online = readCdo->getOnlineId(cabling_data, log);

    if (!online) {
        log << MSG::DEBUG << "ROD Id of the Station " << MSG::hex << "0x" << offlineId << " not found" << endmsg;
        cabling_data.mrod = 0xff;
    } else {
        log << MSG::DEBUG << "The ROD ID of the station "
            << "is " << MSG::hex << "0x" << cabling_data.mrod << MSG::dec << endmsg;
    }

    eformat::SubDetector detid = (eformat::SubDetector)cabling_data.subdetectorId;

    // if (SubsystemId == eformat::MUON_MDT_BARREL_A_SIDE) {
    //  detid = eformat::MUON_MDT_BARREL_A_SIDE;
    //}
    // else if (SubsystemId == eformat::MUON_MDT_BARREL_C_SIDE) {
    //  detid = eformat::MUON_MDT_BARREL_C_SIDE;
    //}
    // else if (SubsystemId == eformat::MUON_MDT_ENDCAP_A_SIDE) {
    //  detid = eformat::MUON_MDT_ENDCAP_A_SIDE;
    //}
    // else if (SubsystemId == eformat::MUON_MDT_ENDCAP_C_SIDE) {
    //  detid = eformat::MUON_MDT_ENDCAP_C_SIDE;
    //}

    // uint8_t m = (uint8_t) MrodId;
    SourceIdentifier sid(detid, cabling_data.mrod);

    uint32_t rod_id = sid.code();

    return rod_id;
}

/** mapping SrcID from ROD to ROB
 */
uint32_t MDT_Hid2RESrcID::getRobID(uint32_t rod_id) {
    SourceIdentifier id = SourceIdentifier(rod_id);
    SourceIdentifier id2 = SourceIdentifier(id.subdetector_id(), id.module_id());
    // return id2.code();

    // FIXME ! the rob ID in the first data taken is wrong !
    // return 0x610003;

    // After data will be fixed the correct version should be:
    return rod_id;
}

/** mapping SrcID from ROB to ROS
 */
uint32_t MDT_Hid2RESrcID::getRosID(uint32_t rob_id) {
    //  Change Module Type to ROS, moduleid = 0

    SourceIdentifier id(rob_id);
    SourceIdentifier id2(id.subdetector_id(), 0);
    return id2.code();
}

/** mapping SrcID from ROS to Det
 */
uint32_t MDT_Hid2RESrcID::getDetID(uint32_t ros_id) {
    //  ROS to DET

    SourceIdentifier id(ros_id);
    SourceIdentifier id2(id.subdetector_id(), 0);
    return id2.code();
}
