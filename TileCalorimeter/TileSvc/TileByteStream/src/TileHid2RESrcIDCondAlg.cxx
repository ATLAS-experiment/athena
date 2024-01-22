/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileHid2RESrcIDCondAlg.h"
#include "TileIdentifier/TileHWID.h"

// Athena includes

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "ByteStreamCnvSvcBase/ROBDataProviderSvc.h"

StatusCode TileHid2RESrcIDCondAlg::initialize() {

  ATH_MSG_DEBUG( "In initialize()" );

  // Retrieve TileHWID helper from det store
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_hid2ReSrcIdKey.initialize() );

  m_initFromDB = !m_rodStatusProxy.empty();
  ATH_CHECK( m_rodStatusProxy.retrieve(EnableTool{m_initFromDB}) );

  // Initilize from event if length of ROD2ROBmap is and odd value
  m_initFromEvent = (!m_initFromDB && (m_ROD2ROBmap.value().size() % 2 == 1));
  if (m_initFromEvent) {
    ATH_CHECK(m_robSvc.retrieve());
  }

  return StatusCode::SUCCESS;
}


StatusCode TileHid2RESrcIDCondAlg::execute(const EventContext& ctx) const {

  SG::WriteCondHandle<TileHid2RESrcID> hid2ReSrcId{m_hid2ReSrcIdKey, ctx};

  if (hid2ReSrcId.isValid()) {
    ATH_MSG_DEBUG("Found valid TileHid2RESrcID: " << hid2ReSrcId.key());
    return StatusCode::SUCCESS;
  }

  const uint32_t run = ctx.eventID().run_number();

  EventIDRange eventRange;
  std::unique_ptr<TileHid2RESrcID> hid2re;

  if (m_initFromDB) {
    hid2re = std::make_unique<TileHid2RESrcID>(m_tileHWID, run);

    auto calibData = std::make_unique<TileCalibData<TileCalibDrawerInt>>();
    ATH_CHECK( m_rodStatusProxy->fillCalibData(*calibData, eventRange) );

    std::vector<std::vector<uint32_t>> allMap(TileCalibUtils::MAX_DRAWERIDX);

    const unsigned int adc = 0u;
    const unsigned int channel = 0u;
    for (unsigned int drawerIdx = TileCalibUtils::MAX_DRAWR0; drawerIdx < TileCalibUtils::MAX_DRAWERIDX; ++ drawerIdx) {
      const TileCalibDrawerInt* calibDrawer = calibData->getCalibDrawer(drawerIdx);
      unsigned int nValues = calibDrawer->getObjSizeUint32();
      std::vector<uint32_t>& drawerMap = allMap[drawerIdx];
      drawerMap.reserve(nValues);
      for (unsigned int i = 0; i < nValues; ++i) {
        drawerMap.push_back( calibDrawer->getData(channel, adc, i) );
      }
    }

    hid2re->initialize(allMap);

  } else if (m_forHLT) {
    hid2re = std::make_unique<TileHid2RESrcID>(m_tileHWID, run);
  } else {

    unsigned int fullMode = (m_fullTileRODs > 0) ? m_fullTileRODs.value() : run;
    hid2re = std::make_unique<TileHid2RESrcID>(m_tileHWID, fullMode);

    // Check whether we want to overwrite default ROB IDs
    if (m_initFromEvent) {
      ATH_MSG_DEBUG( "Length of ROD2ROBmap is and odd value, "
                     << " means that we'll scan event for all fragments to create proper map" );

      bool of2Default{true};
      const eformat::FullEventFragment<const uint32_t*> * event = m_robSvc->getEvent(ctx);
      try {
        event->check_tree();
        hid2re->setROD2ROBmap(event, of2Default, msg());
      } catch (...) {
        ATH_MSG_DEBUG( "Bad event, mapping might be incomplete! " );
        // Bad event, but process anyhow (well, till next bug report )
        hid2re->setROD2ROBmap(event, of2Default, msg());
      }
    } else if (m_ROD2ROBmap.empty()) {
      ATH_MSG_DEBUG( "Length of ROD2ROBmap vector is zero, "
                     << " means that predefined mapping for run " << fullMode << " will be used " );
    } else {
      ATH_MSG_DEBUG( "Apply additional remapping for " << m_ROD2ROBmap.value().size()/2 << " fragments from jobOptions ");
      hid2re->setROD2ROBmap(m_ROD2ROBmap.value(), msg());
    }

  }

  if (!m_initFromDB) {
    // Set validity of TileHid2RESrcID for current run
    eventRange = EventIDRange(EventIDBase{run, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, 0},
                              EventIDBase{run + 1, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, 0});
  }

  hid2re->printSpecial(msg());

  ATH_CHECK( hid2ReSrcId.record(eventRange, std::move(hid2re)) );

  ATH_MSG_VERBOSE("Recorded TileHid2RESrcID object with "
                  << hid2ReSrcId.key()
                  << " with EventRange " << eventRange
                  << " into Conditions Store");

  return StatusCode::SUCCESS;
}
