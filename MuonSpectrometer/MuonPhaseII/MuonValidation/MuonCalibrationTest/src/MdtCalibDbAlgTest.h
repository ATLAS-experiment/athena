/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MDTCALIBDBALGTEST_H
#define MUONVALR4_MDTCALIBDBALGTEST_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include <xAODMuonPrepData/MdtDriftCircleContainer.h>
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MuonTesterTree/MuonTesterTree.h"

class TH2D; 

namespace MuonValR4{
    class MdtCalibDbAlgTest : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm; 
            virtual ~MdtCalibDbAlgTest() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            virtual StatusCode finalize() override;

        private:
            //Retrieve the xAODMdtCircles container
            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_MdtKey{this, "MdtKey", "xMdtDriftCircles",
                                                                      "Key to the uncalibrated Drift circle measurements"};
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** pointer to MdtCalibSvc */
            ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};            MuonVal::MuonTesterTree m_tree{"MdtCalibDbAlg", "MdtCalibDbAlgTest"};

            MuonVal::ScalarBranch<float>& m_out_tdcAdj{m_tree.newScalar<float>("tdcAdj")};
            MuonVal::ScalarBranch<float>& m_out_tdc{m_tree.newScalar<float>("tdc")};
            MuonVal::ScalarBranch<float>& m_out_driftRadius{m_tree.newScalar<float>("driftRadius")};
            MuonVal::ScalarBranch<float>& m_out_driftVelocity{m_tree.newScalar<float>("driftVelocity")};
            MuonVal::ScalarBranch<float>& m_out_driftTime{m_tree.newScalar<float>("driftTime")};

            MuonVal::ScalarBranch<uint8_t>& m_out_stIndex{m_tree.newScalar<uint8_t>("stationIndex")};
            MuonVal::ScalarBranch<int16_t>&  m_out_stEta{m_tree.newScalar<int16_t>("stationEta")};
            MuonVal::ScalarBranch<uint8_t>& m_out_stPhi{m_tree.newScalar<uint8_t>("stationPhi")};
            MuonVal::ScalarBranch<uint8_t>& m_out_ml{m_tree.newScalar<uint8_t>("multiLayer")};
            MuonVal::ScalarBranch<uint8_t>& m_out_tl{m_tree.newScalar<uint8_t>("tubeLayer")};
            MuonVal::ScalarBranch<uint8_t>& m_out_tube{m_tree.newScalar<uint8_t>("tube")};


            MuonVal::ScalarBranch<float>& m_out_globalPos{m_tree.newScalar<float>("globalPos")};
            MuonVal::ScalarBranch<float>& m_out_globalPosX{m_tree.newScalar<float>("globalPosX")};
            MuonVal::ScalarBranch<float>& m_out_globalPosY{m_tree.newScalar<float>("globalPosY")};
            MuonVal::ScalarBranch<float>& m_out_globalPosZ{m_tree.newScalar<float>("globalPosZ")};
            MuonVal::ScalarBranch<float>& m_out_tubeLength{m_tree.newScalar<float>("tubeLength")};

    };
}
#endif
