
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT2CaloCommon_LARRODDECODERTEST_h
#define TrigT2CaloCommon_LARRODDECODERTEST_h

//#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "TrigT2CaloCommon/LArCellCont.h"

#include "AthenaKernel/SlotSpecificObj.h"
#include "LArByteStream/LArRodDecoder.h"
#include "LArRawUtils/LArTT_Selector.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloEvent/CaloBCIDAverage.h"
#include "LArRawConditions/LArMCSym.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArFebRodMapping.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArRecConditions/LArRoIMap.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class TestLArRodDecoder
  : public ::AthAlgorithm
{
 public:

    TestLArRodDecoder( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~TestLArRodDecoder();
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext& context) override;
 private:
    PublicToolHandle<LArRodDecoder> m_larDecoder { this, "LArDecoderTool", "LArRodDecoder/LArRodDecoder", "Tool to decode LAr raw data" };

    //ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Tool to monitor performance of the service" };

    ServiceHandle<IROBDataProviderSvc>  m_robDataProvider{ this, "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc", ""};
    SG::ReadCondHandleKey<LArMCSym> m_mcsymKey {this, "MCSymKey", "LArMCSym", "SG Key of LArMCSym object"} ;
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_onOffIdMappingKey {this, "CablingKey", "LArOnOffIdMap", "SG Key for LArOnOffIdMapping"} ;
    SG::ReadCondHandleKey<LArFebRodMapping> m_febRodMappingKey {this, "RodFebKey", "LArFebRodMap", "SG Key for LArFebRodMapping"} ;
    SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey {this, "LArBadChannelKey", "LArBadChannel", "Key of the LArBadChannelCont CDO" };
    SG::ReadCondHandleKey<LArRoIMap> m_larRoIMapKey {this, "LArRoIMapKey", "LArRoIMap", "Key of the LArRoIMap CDO" };
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey {this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
    Gaudi::Property<std::vector<uint32_t> > m_vrodid32fullDet { this, "ROBIDs",{},"ROD IDs to play with" };
    Gaudi::Property<bool> m_printOut{ this, "PrintDebug", false, "Print Debug" };
    struct EventCache {
        std::unique_ptr<LArCellCont> larContainer;
        int count=0;
    };
    SG::SlotSpecificObj< EventCache > m_testSlot;
    void lateInit(const EventContext& context );
    mutable std::once_flag m_lateInitFlag;

};
 
#endif
