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
#include <map>
#include <memory>

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "MuonCalibStreamCnv/TgcCalibRawDataProvider.h"
#include "MuonIdHelpers/TgcIdHelper.h"
#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"

#include <algorithm>

using namespace LVL2_MUON_CALIBRATION;


TgcCalibRawDataProvider::TgcCalibRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}


//int TgcCalibRawDataProvider::getRodIdFromSectorId(int tmp_sectorId) const { return ((tmp_sectorId % 12) + 1); }

uint16_t TgcCalibRawDataProvider::bcTagCnv(uint16_t bcBitMap) const {
    return (bcBitMap == 4 ? 1 : (bcBitMap == 2 ? 2 : (bcBitMap == 1 ? 3 : 0)));
}


StatusCode TgcCalibRawDataProvider::initialize() {

    ATH_MSG_INFO("TgcCalibRawDataProvider::initialize");

    // retrieve common tools
    ATH_CHECK(m_detectorManagerKey.initialize());    
    ATH_CHECK(m_muonIdHelper.retrieve());

    // retrieve the dataProviderSvc
    ATH_CHECK(m_dataProvider.retrieve());

    // setup output Tgc container keys
    ATH_CHECK(m_rdoContainerKey.initialize());   



    return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute
StatusCode TgcCalibRawDataProvider::execute(const EventContext& ctx) const {

    ATH_MSG_INFO("TgcCalibRawDataProvider::execute");

    const CalibEvent *event = m_dataProvider->getEvent();

    // // setup output write handle for RpcPadContainer
    SG::WriteHandle<TgcRdoContainer> handle{m_rdoContainerKey, ctx};
    ATH_CHECK(handle.record(std::make_unique<TgcRdoContainer>(m_muonIdHelper->tgcIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Created TgcRodContainer " << m_rdoContainerKey.key());    
    // Pass the container from the handle
    TgcRdoContainer *padContainer = handle.ptr();

    ATH_CHECK(decodeImpl(padContainer, event));
    ATH_MSG_DEBUG("TGC core decode processed in MT decode (calibration stream event)");
    
    return StatusCode::SUCCESS;
}


StatusCode TgcCalibRawDataProvider::decodeImpl(TgcRdoContainer *padContainer, const CalibEvent *event) const {

    // decoding process from https://gitlab.cern.ch/atlas-mcp/MdtCalib/mdtcalibframework/-/blob/master/MuonCalibStream/MuonCalibStreamCnv/src/TgcRDOContCalibStreamCnv.cxx
    // to be verified with run3 data

    if (!event->tgc()) {
            ATH_MSG_DEBUG("NO TGC hits!");
            return StatusCode::SUCCESS;
        }

    int l1Id = event->lvl1_id();
    int bcId = 0;  // DUMMY BCID
    int tgc_systemId = (*(event->tgc())).systemId();
    int tgc_subsystemId = (*(event->tgc())).subsystemId();
    // int tgc_roiNumber   = (*(event->tgc())).roiNumber();
    // sectorId absent in new decoding classes
    // int tgc_sectorId    = (*(event->tgc())).sectorId();
    // int tgc_rodId       = getRodIdFromSectorId(tgc_sectorId);
    // to be checked!!!
    int tgc_rodId = ((*(event->tgc())).rdoId() % 12) + 1 ;
    //int tgc_rodId = getRodIdFromSectorId(tgc_rod);
    uint16_t rdoId = TgcRdo::calculateOnlineId(tgc_subsystemId, tgc_rodId);
    TgcRdoIdHash rdoIdHash;
    int idHash = rdoIdHash(rdoId);

    ATH_MSG_DEBUG(std::endl
                  << std::hex << " systemId " << tgc_systemId << " subsystemId "
                  << tgc_subsystemId
                  // <<" sectorId "<< tgc_sectorId
                  << " rodId " << tgc_rodId << " rdoId " << std::hex << rdoId << " idHash " << idHash);

    std::unique_ptr<TgcRdo> newrdo = std::make_unique<TgcRdo>(tgc_subsystemId, tgc_rodId, bcId, l1Id);

    // TgcRdo* newrdo = new TgcRdo(rdoId, idHash);

    // std::list<TgcCalibData> tgcdata = (event->tgc())->data();

    // loop over the list
    // from here onwards you need to insert TGC dependent code
    // to copy the contents of TgcCalibData objects in RDO objects
    // TgcCalibData can be found in
    // Trigger/TrigAlgorithms/TrigmuFast/TrigmuFast/CalibData.h
    // looking at tags from TrigmuFast-00-02-76-34 onward (ONLY IN TrigmuFast-00-02-76 BRANCH!!!)
    // once you have created and filled a TgcRdo it should be added
    // with
    // StatusCode sc = m_rdoContainer->addCollection(newrdo,elementHash );
    // the main problem here on the athena side is to find out
    // the proper ids and hashes to be used when creating the new TgcRdo and
    // when adding it to the container...
    std::list<TgcCalibData> listOfTGCCalibData = (event->tgc())->data();
    std::list<TgcCalibData>::iterator itOfTGCCalibData = listOfTGCCalibData.begin();
    for (; itOfTGCCalibData != listOfTGCCalibData.end(); itOfTGCCalibData++) {
        std::list<TGC_BYTESTREAM_READOUTHIT> listOfBsReadoutHit = (*(itOfTGCCalibData)).readoutHit();
        std::list<TGC_BYTESTREAM_READOUTTRACKLET> listOfBsReadoutTracklet = (*(itOfTGCCalibData)).readoutTracklet();
        std::list<TGC_BYTESTREAM_READOUTTRIPLETSTRIP> listOfBsReadoutTripletStrip = (*(itOfTGCCalibData)).readoutTripletStrip();
        std::list<TGC_BYTESTREAM_HIPT> listOfBsHipt = (*(itOfTGCCalibData)).readoutHipt();
        std::list<TGC_BYTESTREAM_SL> listOfBsSL = (*(itOfTGCCalibData)).readoutSL();

        // Filling Readout Format Hit Data
        std::list<TGC_BYTESTREAM_READOUTHIT>::iterator itOfBsReadoutHit = listOfBsReadoutHit.begin();
        for (; itOfBsReadoutHit != listOfBsReadoutHit.end(); itOfBsReadoutHit++) {
            TGC_BYTESTREAM_READOUTHIT roh = *(itOfBsReadoutHit);

            // printf("raw data %08x \n",roh);
            unsigned int ldbId = roh.ldbId; 
            unsigned int sbId = roh.sbId; 
            unsigned int tracklet = roh.tracklet;
            ATH_MSG_DEBUG(std::hex << "TgcRawData READOUT FORMATTED HIT " << std::endl
                                   << " bcTag " << bcTagCnv(roh.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                                   << newrdo->rodId() << " sswId " << ldbId << " sbId " << sbId << " l1Id " << newrdo->l1Id()
                                   << " bcId " << newrdo->bcId() << " sbType " << (TgcRawData::SlbType)roh.sbType << " adjucent "
                                   << (bool)roh.adj << " associate tracklet " << roh.tracklet << " bitPos "
                                   << roh.channel + 40  // is it fixed or not ? (yasuyuki)
            );
            std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(roh.bcBitmap), newrdo->subDetectorId(), newrdo->rodId(), ldbId, sbId,
                                             newrdo->l1Id(), newrdo->bcId(), (TgcRawData::SlbType)roh.sbType, (bool)roh.adj, tracklet,
                                             roh.channel + 40  // is it fixed or not ? (yasuyuki)
                                            );
            newrdo->push_back(std::move(raw));

        }  // end loop over itOfBsReadoutHit

        // Filling Readout Format Triplet Strip Hit
        std::list<TGC_BYTESTREAM_READOUTTRIPLETSTRIP>::iterator itOfBsReadoutTripletStrip = listOfBsReadoutTripletStrip.begin();
        for (; itOfBsReadoutTripletStrip != listOfBsReadoutTripletStrip.end(); itOfBsReadoutTripletStrip++) {
            TGC_BYTESTREAM_READOUTTRIPLETSTRIP rostrip = *(itOfBsReadoutTripletStrip);

            // printf("raw data %08x \n",rostrip);
            unsigned int ldbId = rostrip.ldbId; 
            unsigned int sbId = rostrip.sbId; 
            unsigned int seg = rostrip.seg;
            unsigned int l1Id = newrdo->l1Id();
            unsigned int bcId = newrdo->bcId();
            unsigned int subc = rostrip.subc;
            unsigned int phi = rostrip.phi;
            ATH_MSG_DEBUG(std::hex
                          //<<"raw data :: "<<(unsigned long)(*rostrip)<<std::endl
                          << "TgcRawData READOUT TRIPLET STRIP " << std::endl
                          << " bcTag " << bcTagCnv(rostrip.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                          << newrdo->rodId() << " sswId " << ldbId << " sbId " << sbId << " l1Id " << newrdo->l1Id()
                          << " bcId " << newrdo->bcId() << " sbType " << TgcRawData::SLB_TYPE_TRIPLET_STRIP << " adjucent "
                          << "0"
                          << " seg " << rostrip.seg << " subc " << rostrip.subc << " phi " << rostrip.phi);

            std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(rostrip.bcBitmap), newrdo->subDetectorId(), newrdo->rodId(), ldbId,
                                             sbId, l1Id, bcId, TgcRawData::SLB_TYPE_TRIPLET_STRIP, 0,
                                             seg, subc, phi);

            newrdo->push_back(std::move(raw));
        }  // end loop over itOfBsReadoutTripletStrip

        // Filling Readout Format Tracklet Data
        std::list<TGC_BYTESTREAM_READOUTTRACKLET>::iterator itOfBsReadoutTracklet = listOfBsReadoutTracklet.begin();
        for (; itOfBsReadoutTracklet != listOfBsReadoutTracklet.end(); itOfBsReadoutTracklet++) {
            TGC_BYTESTREAM_READOUTTRACKLET rotrk = *(itOfBsReadoutTracklet);

            // printf("raw data %08x \n",rotrk);
            unsigned int sbId = rotrk.sbId; 
            unsigned int ldbId = rotrk.ldbId; 
            unsigned int delta = rotrk.delta; 
            unsigned int seg = rotrk.seg; 
            unsigned int rphi = rotrk.rphi; 
            ATH_MSG_DEBUG(std::hex
                          //<<"raw data :: "<<(unsigned long)(*rotrk)<<std::endl
                          << "TgcRawData READOUT TRACKLET " << std::endl
                          << " bcTag " << bcTagCnv(rotrk.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                          << newrdo->rodId() << " sswId " << ldbId << " sbId " << sbId << " l1Id " << newrdo->l1Id() << " bcId "
                          << newrdo->bcId() << " sbType "
                          << ((rotrk.slbType == 4) ? TgcRawData::SLB_TYPE_INNER_STRIP : (TgcRawData::SlbType)rotrk.slbType) << " adjacent "
                          << "0" << " seg " << rotrk.seg << " zero " << "0" << " r phi " << rotrk.rphi);

            std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(rotrk.bcBitmap), newrdo->subDetectorId(), newrdo->rodId(), ldbId, sbId, newrdo->l1Id(),
                               newrdo->bcId(), (rotrk.slbType == 4) ? TgcRawData::SLB_TYPE_INNER_STRIP : (TgcRawData::SlbType)rotrk.slbType,
                               delta, seg, 0, rphi);
            newrdo->push_back(std::move(raw));

        }  // end loop over itOfBsReadoutTracklet

        // Filling Hipt Data
        std::list<TGC_BYTESTREAM_HIPT>::iterator itOfBsHipt = listOfBsHipt.begin();
        for (; itOfBsHipt != listOfBsHipt.end(); itOfBsHipt++) {
            TGC_BYTESTREAM_HIPT hpt = *(itOfBsHipt);

            // printf("raw data %08x \n",hpt);
            unsigned int strip = hpt.strip;
            unsigned int fwd = hpt.fwd;
            unsigned int sector = hpt.sector;
            unsigned int chip = hpt.chip;
            unsigned int cand = hpt.cand;
            unsigned int hipt = hpt.hipt;
            unsigned int hitId = hpt.hitId;
            unsigned int sub = hpt.sub;
            unsigned int delta = hpt.delta;

            ATH_MSG_DEBUG(std::hex
                          //<<"raw data :: "<<(unsigned long)(*hpt)<<std::endl
                          << "TgcRawData HPT " << std::endl
                          << " bcTag " << bcTagCnv(hpt.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                          << newrdo->rodId() << " l1Id " << newrdo->l1Id() << " bcId " << newrdo->bcId() << " strip " << strip
                          << " forward " << fwd << " sector " << hpt.sector << " chip " << hpt.chip << " cand " << hpt.cand << " hipt "
                          << hpt.hipt << " hitId " << hpt.hitId << " sub " << hpt.sub << " delta " << hpt.delta);

            std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(hpt.bcBitmap), newrdo->subDetectorId(), newrdo->rodId(), newrdo->l1Id(), newrdo->bcId(), strip,
                               fwd, sector, chip, cand, hipt, hitId, sub, delta, 0);
            newrdo->push_back(std::move(raw));

        }  // end loop over itOfBsHipt
        // ---------------------

        // Filling SL Data
        std::list<TGC_BYTESTREAM_SL>::iterator itOfBsSL = listOfBsSL.begin();
        for (; itOfBsSL != listOfBsSL.end(); itOfBsSL++) {
            TGC_BYTESTREAM_SL sl = *(itOfBsSL);

            // printf("raw data %08x \n",sl);
            unsigned int cand2plus = sl.cand2plus;
            unsigned int fwd = sl.fwd;
            unsigned int sector = sl.sector;
            unsigned int cand = sl.cand;
            unsigned int sign = sl.sign;
            unsigned int threshold = sl.threshold;
            unsigned int overlap = sl.overlap;
            unsigned int roi = sl.roi;

            ATH_MSG_DEBUG(std::hex << "TgcRawData SL "
                                   //<<"raw data :: "<<(unsigned long)(*sl)
                                   << " bcTag " << bcTagCnv(sl.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                                   << newrdo->rodId() << " l1Id " << newrdo->l1Id() << " bcId " << newrdo->bcId() << " cand2pluse "
                                   << cand2plus << " fwd " << sl.fwd << " sector " << sl.sector << " cand " << sl.cand << " sign "
                                   << sl.sign << " thereshold " << sl.threshold << " overlap " << sl.overlap << " roi " << sl.roi);

            std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(sl.bcBitmap), newrdo->subDetectorId(), newrdo->rodId(), newrdo->l1Id(), newrdo->bcId(),
					         cand2plus, fwd, sector, cand, sign, threshold, overlap, false, roi);

            newrdo->push_back(std::move(raw));
        }  // end loop over itOfBsSL


        TgcRdoContainer::IDC_WriteHandle lock = padContainer->getWriteHandle(idHash);
        ATH_CHECK(lock.addOrDelete(std::move(newrdo)));
    }  // end loop over itOfTGCCalibData

    return StatusCode::SUCCESS;
}  
