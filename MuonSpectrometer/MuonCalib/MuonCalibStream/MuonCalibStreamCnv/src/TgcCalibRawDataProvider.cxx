/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  
  if (!event->tgc()) {
    ATH_MSG_DEBUG("NO TGC hits!");
    return StatusCode::SUCCESS;
  }
  
  int l1Id = event->lvl1_id();
  int bcId = 0;  // DUMMY BCID
  int subsystemId = (*(event->tgc())).subsystemId(); // 0x67 or 0x68
  int rodId = (*(event->tgc())).rdoId(); // This returns sector number since Run 2 (1..12 for legacy ROD, 17..19 for new trigger SROD), +1 for Run 1 data
  uint16_t rdoId = TgcRdo::calculateOnlineId(subsystemId,rodId); // 0-23 : ROD, 24-29 : SROD
  TgcRdoIdHash rdoIdHash;
  int idHash = rdoIdHash(rdoId);
  
  std::unique_ptr<TgcRdo> newrdo = std::make_unique<TgcRdo>(subsystemId,rodId,bcId,l1Id);
  
  for(auto const& calib_tgcRdo : (event->tgc())->data()){
    for(auto const& roh : calib_tgcRdo.readoutHit()){
      
      if(roh.sbType==TgcRawData::SLB_TYPE_INNER_WIRE || roh.sbType==TgcRawData::SLB_TYPE_INNER_STRIP) {
	    continue;
      } // discard EIL4 TGC hits 

      uint16_t slbId = roh.sbId;
      std::unique_ptr<TgcRawData> raw = std::make_unique<TgcRawData>(bcTagCnv(roh.bcBitmap),
								     subsystemId,
								     rodId,
								     roh.ldbId,
								     slbId,
								     l1Id,
								     bcId,
								     static_cast<TgcRawData::SlbType>(roh.sbType),
								     roh.adj,
								     roh.tracklet,
								     roh.channel+40);
      
      ATH_MSG_DEBUG(std::hex << "TgcRawData READOUT FORMATTED HIT " << std::endl
                    << " bcTag " << bcTagCnv(roh.bcBitmap) << " subDetectorId " << newrdo->subDetectorId() << " rodId "
                    << newrdo->rodId() << " sswId " << roh.ldbId << " sbId " << slbId << " l1Id " << newrdo->l1Id()
                    << " bcId " << newrdo->bcId() << " sbType " << static_cast<TgcRawData::SlbType>(roh.sbType) << " adjucent "
                    << roh.adj << " associate tracklet " << roh.tracklet << " bitPos "
                    << roh.channel + 40);

      newrdo->push_back(std::move(raw));
    }
  }
  
  TgcRdoContainer::IDC_WriteHandle lock = padContainer->getWriteHandle(idHash);
  ATH_CHECK(lock.addOrDelete(std::move(newrdo)));
  
  return StatusCode::SUCCESS;
}  
