/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAM_EVENTINFOCALIBRAWDATAPROVIDER_H
#define MUONCALIBSTREAM_EVENTINFOCALIBRAWDATAPROVIDER_H

#include <stdint.h>
#include <string>
#include <map>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/DataHandle.h"
#include "MuCalDecode/CalibData.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuCalDecode/CalibUti.h"
#include "GaudiKernel/Converter.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"
//#include "MuonCalibStreamCnvSvc/MuonCalibRunLumiBlockCoolSvc.h"

class StatusCode;
//class MuonCalibRunLumiBlockCoolSvc;


class EventInfoCalibRawDataProvider : public AthReentrantAlgorithm {
    
    public:
        //! Constructor.
        EventInfoCalibRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;

        //! Destructur
        ~EventInfoCalibRawDataProvider() = default;

    private:

        ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider{this,"DataProviderSvc","MuonCalibStreamDataProviderSvc"};
        
        //ServiceHandle<MuonCalibRunLumiBlockCoolSvc> m_lumiBlockCoolSvc{this,"MuonCalibRunLumiBlockCoolSvc", "MuonCalibRunLumiBlockCoolSvc"};

        SG::WriteHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "OutputEventInfo", "MuonCalibStreamxAODEventInfo"};

        Gaudi::Property<bool> m_lumi_block_number_from_cool {this, "lb_from_cool", true}; 

};
#endif
