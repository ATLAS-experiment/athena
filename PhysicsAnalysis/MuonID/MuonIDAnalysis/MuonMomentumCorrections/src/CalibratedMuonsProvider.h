/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMMC_CALIBRATEDMUONSPROVIDER_H
#define MUONMMC_CALIBRATEDMUONSPROVIDER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"

namespace CP {

    /// decorates a muon collection with efficiency and scale factor

    class CalibratedMuonsProvider : public AthAlgorithm {
    public:
        /// Regular Algorithm constructor
        CalibratedMuonsProvider(const std::string& name, ISvcLocator* svcLoc);

        /// Function initialising the algorithm
        virtual StatusCode initialize() override;
        /// Function executing the algorithm
        virtual StatusCode execute() override;

    private:
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};
        /// muon container
        SG::ReadHandleKey<xAOD::MuonContainer> m_inputKey{this, "Input", "Muons", " Name of the muon container to calibrate"};
        SG::WriteHandleKey<xAOD::MuonContainer> m_outputKey{this, "Output", "CalibratedMuons",
                                                            "Name of the final container written to storegate"};
        /// Additionally declare the two decorations made by MCAST
        SG::WriteDecorHandleKeyArray<xAOD::MuonContainer> m_ptDecorKeys{this, "DecorationKeys", {}};

        /// Muon calibration tool
        ToolHandle<IMuonCalibrationAndSmearingTool> m_tool{this, "Tool", ""};
        Gaudi::Property<bool> m_useRndNumber{this, "useRndRunNumber", false};
        SG::ReadDecorHandleKey<xAOD::EventInfo> m_rndNumKey{this, "RandomNumberDecor", m_eventInfo, "RandomRunNumber",
                                                            "Dependency on the random run number"};

    };  // class

}  // namespace CP

#endif  //
