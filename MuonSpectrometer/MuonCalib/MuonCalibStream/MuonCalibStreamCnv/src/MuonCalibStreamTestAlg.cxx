#include "MuonCalibStreamCnv/MuonCalibStreamTestAlg.h"

#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/CscRawDataContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

MuonCalibStreamTestAlg::MuonCalibStreamTestAlg(const std::string &name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonCalibStreamTestAlg::initialize() {

    ATH_MSG_INFO("initialize");

    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_MdtPrepDataKey.initialize());
    ATH_CHECK(m_RpcPrepDataKey.initialize());
    ATH_CHECK(m_TgcPrepDataKey.initialize());

    return StatusCode::SUCCESS;
}

//  Dumps MDT and RPC data; CSC, TGC not so much because code unfinished
StatusCode MuonCalibStreamTestAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute");
    // verify EventInfo class
    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfoKey, ctx);

    const xAOD::EventInfo* ei = evtInfo.cptr();
    // Basic info:
    ATH_MSG_DEBUG( "runNumber = " << ei->runNumber());
    ATH_MSG_DEBUG( "eventNumber = " << ei->eventNumber() );
    ATH_MSG_DEBUG( "lumiBlock = " << ei->lumiBlock() );
    ATH_MSG_DEBUG( "timeStamp = " << ei->timeStamp());
    ATH_MSG_DEBUG( "pt = " << ei->timeStampNSOffset() );  // special bit for Muon calibration eventInfo
    ATH_MSG_DEBUG( "bcid = " << ei->bcid() );
    ATH_MSG_DEBUG( "detectorMask = " << ei->eventTypeBitmask());

    // verify MdtPrepDataContainer
    SG::ReadHandle<Muon::MdtPrepDataContainer> mdtPrds(m_MdtPrepDataKey, ctx);
    ATH_MSG_DEBUG("****** MDT Container size: " << mdtPrds->size());
    if (mdtPrds->size()) {
    Muon::MdtPrepDataContainer::const_iterator it = mdtPrds->begin();
    Muon::MdtPrepDataContainer::const_iterator it_end = mdtPrds->end();

    for (; it != it_end; ++it) {
        ATH_MSG_DEBUG("MDT ********** Collections size: " << (*it)->size());
        Muon::MdtPrepDataCollection::const_iterator cit_begin = (*it)->begin();
        Muon::MdtPrepDataCollection::const_iterator cit_end = (*it)->end();
        Muon::MdtPrepDataCollection::const_iterator cit = cit_begin;
        for (; cit != cit_end; ++cit) {
            const Muon::MdtPrepData *mdt = (*cit);
            mdt->dump(msg());
        }
    }
    } // end of MDTPrepDataCollections loop
    
    // verify RpcPrepDataContainer
    SG::ReadHandle<Muon::RpcPrepDataContainer> rpcPrds(m_RpcPrepDataKey, ctx);
    ATH_MSG_DEBUG("****** RPC Container size: " << rpcPrds->size());
    // If dumping by container
    if (rpcPrds->size()) {
        Muon::RpcPrepDataContainer::const_iterator it = rpcPrds->begin();
        Muon::RpcPrepDataContainer::const_iterator it_end = rpcPrds->end();

        for (; it != it_end; ++it) {
            ATH_MSG_DEBUG("********** RPC Collections size: " << (*it)->size());
            Muon::RpcPrepDataCollection::const_iterator cit_begin = (*it)->begin();
            Muon::RpcPrepDataCollection::const_iterator cit_end = (*it)->end();
            Muon::RpcPrepDataCollection::const_iterator cit = cit_begin;
            for (; cit != cit_end; ++cit) {
                const Muon::RpcPrepData *rpc = (*cit);
                rpc->dump(msg());
            }
        }
    }
    
    // verify TGC_Measurements
    SG::ReadHandle<Muon::TgcPrepDataContainer> tgcPrds(m_TgcPrepDataKey, ctx);
    ATH_MSG_DEBUG("****** TGC Container size: " << tgcPrds->size());
    if (tgcPrds->size())  {
        Muon::TgcPrepDataContainer::const_iterator it = tgcPrds->begin();
        Muon::TgcPrepDataContainer::const_iterator it_end = tgcPrds->end();

        for (; it != it_end; ++it) {
            ATH_MSG_DEBUG("********** TGC Collections size: " << (*it)->size());
            Muon::TgcPrepDataCollection::const_iterator cit_begin = (*it)->begin();
            Muon::TgcPrepDataCollection::const_iterator cit_end = (*it)->end();
            Muon::TgcPrepDataCollection::const_iterator cit = cit_begin;
            for (; cit != cit_end; ++cit) {
                const Muon::TgcPrepData *tgc = (*cit);
                tgc->dump(msg());
            }
        }
    }

    return StatusCode::SUCCESS;
}  // MuonCalibStreamTestAlg::execute()

StatusCode MuonCalibStreamTestAlg::finalize() { return StatusCode::SUCCESS; }
