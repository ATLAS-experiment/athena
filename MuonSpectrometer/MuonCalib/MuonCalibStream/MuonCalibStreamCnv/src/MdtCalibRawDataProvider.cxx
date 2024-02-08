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
#include "MuonCalibEvent/MdtCalibHit.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "MuonPrepRawData/MdtPrepDataCollection.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <map>

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonCalibStreamCnv/MdtCalibRawDataProvider.h"

using namespace LVL2_MUON_CALIBRATION;

// function to sort the Mdt hits according to the LVL2 identifier
bool CompareIds(const MdtCalibData data1, const MdtCalibData data2) { return data1.id() > data2.id(); }


MdtCalibRawDataProvider::MdtCalibRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibRawDataProvider::initialize() {
    ATH_MSG_INFO("MdtCalibRawDataProvider::initialize");
    
    ATH_CHECK(m_detectorManagerKey.initialize());
    ATH_CHECK(m_muonIdHelper.retrieve());
    ATH_CHECK(m_dataProvider.retrieve());
    ATH_CHECK(m_mdtPrepDataContainerKey.initialize());   

    return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute
StatusCode MdtCalibRawDataProvider::execute(const EventContext& ctx) const {
    
    ATH_MSG_INFO("MdtCalibRawDataProvider::execute");

    const CalibEvent *event = m_dataProvider->getEvent();

    ATH_MSG_DEBUG("Event Pt : "<<event->pt());

    // // setup output write handle for MdtPrepDataContainer
    SG::WriteHandle<Muon::MdtPrepDataContainer> handle{m_mdtPrepDataContainerKey, ctx};
    
    ATH_CHECK(handle.record(std::make_unique<Muon::MdtPrepDataContainer>(m_muonIdHelper->mdtIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Created container " << m_mdtPrepDataContainerKey.key());    
        // Pass the container from the handle
    Muon::MdtPrepDataContainer *mdtContainer = handle.ptr();

    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgr{m_detectorManagerKey, ctx};
        if (!muDetMgr.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Muon detector manager " << m_detectorManagerKey.fullKey());
            return StatusCode::FAILURE;
        }

    ATH_CHECK(decodeImpl(mdtContainer, event, *muDetMgr));
    
    ATH_MSG_DEBUG("MDT core decode processed in MT decode (calibration stream event)");
    
    return StatusCode::SUCCESS;

    } // end of execute

StatusCode MdtCalibRawDataProvider::decodeImpl(Muon::MdtPrepDataContainer *mdtPrepDataContainer, const CalibEvent *event, const MuonGM::MuonDetectorManager* muDetMgr) const {
    
    // decoding process from https://gitlab.cern.ch/atlas-mcp/MdtCalib/mdtcalibframework/-/blob/master/MuonCalibStream/MuonCalibStreamCnv/src/RpcRDOContCalibStreamCnv.cxx
    // to be verified with run3 data

    // Skip events with no MDT hits (otherwise crash below)
    if (!event->mdt()) {
        ATH_MSG_WARNING(" NO MDT hits in event!");
        return StatusCode::SUCCESS;
    }

    // extract the list of MDT hit tubes
    std::list<MdtCalibData> tubes = (event->mdt())->data();
    // sort the list using CompareIds function
    tubes.sort(CompareIds);
   
    std::vector<std::unique_ptr<Muon::MdtPrepDataCollection>> mdtCollections{};
    
    int mdt_hits(0), mdt_chambers(0);
    int StationName{0}, StationEta{0}, StationPhi{0}, MultiLayer{0}, TubeLayer{0}, Tube{0};
    // loop over MDT hit tubes
    // assumes only leading tdc_counts is present on the data
    const MdtIdHelper& idHelper{m_muonIdHelper->mdtIdHelper()};    
    for (const MdtCalibData& tubeData :  tubes) {
        uint16_t coarse = tubeData.leadingCoarse();
        uint16_t fine = tubeData.leadingFine();
        int tdc_counts = fine | (coarse << 5);
        int adc_counts = tubeData.width();
        WordIntoMdtId(tubeData.id(), StationName, StationEta, StationPhi, MultiLayer, TubeLayer, Tube);

        // convert the tube hit info to an MdtPrepData object
        Muon::MdtDriftCircleStatus digitStatus = Muon::MdtStatusDriftTime;
        Identifier channelId = idHelper.channelID(StationName, StationEta, StationPhi, MultiLayer, TubeLayer, Tube);

        // give a fix radius and err here since the precise radius will be calculated later when MdtDriftCircleOnTrack is created
        const MuonGM::MdtReadoutElement *detEl = muDetMgr->getMdtReadoutElement(channelId);
        /// Assign dummy values for the moment
        constexpr double radius(7.5), errRadius(4.2);
        Amg::Vector2D driftRadius(radius, 0);
        Amg::MatrixX errorMatrix{1, 1};
        errorMatrix(0, 0) = (errRadius * errRadius);

        IdentifierHash mdtHashId = m_muonIdHelper->moduleHash(channelId);

        const unsigned int mdtHashIdx = static_cast<unsigned int>(mdtHashId);
        if (mdtHashIdx >= mdtCollections.size()) mdtCollections.resize(mdtHashIdx + 1);
        std::unique_ptr<Muon::MdtPrepDataCollection>& mdtCollection = mdtCollections[mdtHashIdx];
        if (!mdtCollection) {
            mdtCollection = std::make_unique<Muon::MdtPrepDataCollection>(mdtHashId);
            mdtCollection->setIdentifier(idHelper.elementID(channelId));
        }

        std::unique_ptr<Muon::MdtPrepData> newPrepData = std::make_unique<Muon::MdtPrepData>(channelId, mdtHashId, driftRadius, std::move(errorMatrix), detEl, tdc_counts, adc_counts, digitStatus);

        ATH_MSG_DEBUG(" "<<m_muonIdHelper->toString(channelId)<<" ADC="<<adc_counts<<" TDC="<<tdc_counts<<" mdtHashId : "<<mdtHashId );
        // add the MdtPrepData to the collection
        mdtCollection->push_back(std::move(newPrepData));

    }  // end loop over tubes

    for (std::unique_ptr<Muon::MdtPrepDataCollection>& coll : mdtCollections) {
        if (!coll) continue;
        Muon::MdtPrepDataContainer::IDC_WriteHandle lock = mdtPrepDataContainer->getWriteHandle(coll->identifyHash());
        ATH_CHECK(lock.addOrDelete(std::move(coll)));
    }
    ATH_MSG_DEBUG("Wrote " << mdt_hits << " MDT PRD hits for " << mdt_chambers << " chambers");

    return StatusCode::SUCCESS;

}
