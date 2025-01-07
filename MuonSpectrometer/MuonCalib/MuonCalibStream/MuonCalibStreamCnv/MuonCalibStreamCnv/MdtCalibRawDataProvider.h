/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAM_MDTCALIBRAWDATAPROVIDER_H
#define MUONCALIBSTREAM_MDTCALIBRAWDATAPROVIDER_H

#include <stdint.h>
#include <string>
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
#include "MuCalDecode/CalibData.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuCalDecode/CalibUti.h"

#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"
#include "MuonPrepRawData/MdtPrepDataCollection.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"


class StatusCode;

class MdtCalibRawDataProvider : public AthReentrantAlgorithm {
    
    public:
        //! Constructor.
        MdtCalibRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;

        //! Destructur
        ~MdtCalibRawDataProvider() = default;

    private:

 
        StatusCode decodeImpl(Muon::MdtPrepDataContainer *mdtPrepDataContainer, const LVL2_MUON_CALIBRATION::CalibEvent *event, const MuonGM::MuonDetectorManager* muDetMgr) const;

        ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider{this,"DataProviderSvc","MuonCalibStreamDataProviderSvc"};

        SG::WriteHandleKey<Muon::MdtPrepDataContainer> m_mdtPrepDataContainerKey{this, "OutputCollection", "MDT_DriftCircles"};
        // /** MuonDetectorManager from the conditions store */
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                    "Key of input MuonDetectorManager condition data"};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_muonIdHelper{this, "MuonIdHelper", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                            "Handle to the MuonIdHelperSvc"};
        Gaudi::Property<int> m_adcCut{this, "ADCCut" , 50, "Minimum threshold on the ADC to contribute to recondition"};
};
#endif
