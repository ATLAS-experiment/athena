/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_JFEXINPUTMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_JFEXINPUTMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTrigL1Calo/jFexTowerContainer.h"
#include "xAODTrigL1Calo/jFexTower.h"

#include "LArRecConditions/LArBadChannelCont.h"

#include <atomic>

class JfexInputMonitorAlgorithm : public AthMonitorAlgorithm {
    public:
        JfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        virtual ~JfexInputMonitorAlgorithm()=default;
        virtual StatusCode initialize() override;
        virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

    private:

        StringProperty m_Grouphist{this,"Grouphist","JfexInputMonitor","group name for histograming"};

        // container keys including steering parameter and description
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_jFexDataTowerKey    {this, "jFexDataTower","L1_jFexDataTowers","SG key of the input jFex Tower container"};
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_jFexEmulatedTowerKey{this, "jFexEmulatedTower","L1_jFexEmulatedTowers","SG key of the emulated jFex Tower container"};

        SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArMaskedChannelKey", "LArMaskedSC", "Key of the OTF-Masked SC" };


        unsigned int m_InvalidCode = 4095;
        unsigned int m_EmptyCode = 0;

};
#endif
