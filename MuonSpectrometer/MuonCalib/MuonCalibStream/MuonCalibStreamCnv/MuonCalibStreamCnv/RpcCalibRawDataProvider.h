/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAM_RPCCALIBRAWDATAPROVIDER_H
#define MUONCALIBSTREAM_RPCCALIBRAWDATAPROVIDER_H

#include <string>
#include <stdint.h>
#include <map>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/Converter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/DataHandle.h"
#include "xAODEventInfo/EventInfo.h"

#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"
#include "MuonRDO/RpcCoinMatrix.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "RPC_CondCabling/RpcCablingCondData.h"

class StatusCode;



class RpcCalibRawDataProvider : public AthReentrantAlgorithm {
    
    public:
        //! Constructor.
        RpcCalibRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;
        
        //! Destructur
        ~RpcCalibRawDataProvider() = default;

    private:

 
        StatusCode decodeImpl(const EventContext& ctx, RpcPadContainer *m_padContainer, const LVL2_MUON_CALIBRATION::CalibEvent *event) const ;

        ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider{this,"DataProviderSvc","MuonCalibStreamDataProviderSvc"};

        // /** MuonDetectorManager from the conditions store */
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                    "Key of input MuonDetectorManager condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_muonIdHelper{this, "MuonIdHelper", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                            "Handle to the MuonIdHelperSvc"};

        SG::WriteHandleKey<RpcPadContainer> m_rdoContainerKey{this, "RDOContainer", "RPCPAD"};

        SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};


};
#endif
