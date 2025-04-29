/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDTEST_MDTCALIBTESTALG_H
#define MUONCONDTEST_MDTCALIBTESTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/SystemOfUnits.h"
namespace Muon{
    class MdtCalibTestAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual ~MdtCalibTestAlg() = default;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_readKey{this, "CalibKey",  "MdtCalibConstants",
                                                                              "Conditions object containing the calibrations"};   

            /** @brief Step width to scan the compatibility of the R-t relation */
            Gaudi::Property<double> m_stepR{this, "StepR", 0.05};
            /** @brief Compatibility window. The back & forth mapping may not be
             *         100% accurate. Define a tolerance*/
            Gaudi::Property<double> m_mapTolerance{this, "Tolerance", 10. * Gaudi::Units::micrometer};
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}
#endif