/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAM_TGCCALIBRAWDATAPROVIDER_H
#define MUONCALIBSTREAM_TGCCALIBRAWDATAPROVIDER_H

#include <stdint.h>

#include <map>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
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
#include "MuonRDO/TgcRdoContainer.h"

class StatusCode;

#include <string>


class TgcCalibRawDataProvider : public AthReentrantAlgorithm {
    
    public:
        //! Constructor.
        TgcCalibRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;
        
        //! Destructur
        ~TgcCalibRawDataProvider() = default;

    private:

        //int getRodIdFromSectorId(int tmp_sectorId) const;
        uint16_t bcTagCnv(uint16_t bcBitMap) const;

        // main decode function (ClaibEvent to TgcRdoContainer)
        StatusCode decodeImpl(TgcRdoContainer *m_tgcRdoContainer, const LVL2_MUON_CALIBRATION::CalibEvent *event) const ;


        ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider{this,"DataProviderSvc","MuonCalibStreamDataProviderSvc"};

        // /** MuonDetectorManager from the conditions store */
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                    "Key of input MuonDetectorManager condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_muonIdHelper{this, "MuonIdHelper", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                            "Handle to the MuonIdHelperSvc"};

        // output TgcRdoContainer writeHandle
        SG::WriteHandleKey<TgcRdoContainer> m_rdoContainerKey{this, "RDOContainer", "TGCRDO" ,"TgcRdoContainer to save"};

};
#endif
