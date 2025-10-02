
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONMEASVIEWALGS_MDTMEASVIEWALG_H
#define XAODMUONMEASVIEWALGS_MDTMEASVIEWALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODMuonPrepData/MdtDriftCircleContainer.h>
#include <xAODMuonPrepData/MdtTwinDriftCircleContainer.h>

#include <StoreGate/WriteHandleKey.h>


/**
 * @brief: The MdtMeasViewAlg takes the MdtMeasurments && the MdtTwinMeasurements and combined them into
 *         a common MdtDriftCircleContainer */

namespace MuonR4 {
    class MdtMeasViewAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            ~MdtMeasViewAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        private:
            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_readKey1D{this, "DriftCircle1DKey", "xMdtDriftCircles", 
                                                                       "Name of the standard 1D measurements"};

            SG::ReadHandleKey<xAOD::MdtTwinDriftCircleContainer> m_readKey2D{this, "DriftCircle2DKey", "xMdtTwinDriftCircles", 
                                                                              "Name of the twin tube 2D measurements"};

            SG::WriteHandleKey<xAOD::MdtDriftCircleContainer> m_writeKey{this, "WriteKey", "xMdtMeasurements"};

    };
}

#endif