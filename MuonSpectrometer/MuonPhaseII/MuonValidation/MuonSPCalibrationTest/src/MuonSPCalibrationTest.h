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

#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"

#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"

class TH2D; 

namespace MuonValR4{
    class MuonSPCalibrationTest : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm; 
            virtual ~MuonSPCalibrationTest() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            // virtual StatusCode finalize() override;

        private:
            //Retrieve the xAODMdtCircles container
            SG::ReadHandleKeyArray<MuonR4::SpacePointContainer> m_spKeyArray{this, "SpacePointKeyArray", {"NswSpacePoints"}};
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** pointer to MdtCalibSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
           
            ToolHandle<MuonR4::ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };

    };
}
#endif
