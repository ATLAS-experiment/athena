
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONMEASVIEWALGS_STGCMEASVIEWALG_H
#define XAODMUONMEASVIEWALGS_STGCMEASVIEWALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODMuonPrepData/sTgcWireContainer.h>
#include <xAODMuonPrepData/sTgcStripContainer.h>
#include <xAODMuonPrepData/sTgcPadContainer.h>
#include <xAODMuonPrepData/sTgcMeasContainer.h>


#include <StoreGate/WriteHandleKey.h>


/**
 * @brief: The sTgcMeasViewAlg takes all sTgcStrip, sTgcWire & sTgcPad measurements and pushes
 *         them into a common sTgcMeasContainer which is a SG::VIEW_ELEMENTS container
 * 
*/

namespace MuonR4 {
    class sTgcMeasViewAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            
            ~sTgcMeasViewAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        private:
            SG::ReadHandleKey<xAOD::sTgcStripContainer> m_readKeyStrip{this, "StripKey", "xAODsTgcStrips", 
                                                                       "Name of the xAOD::sTgcStripContainer"};

            SG::ReadHandleKey<xAOD::sTgcWireContainer> m_readKeyWire{this, "WireKey", "xAODsTgcWires", 
                                                                     "Name of the xAOD::sTgcWireContainer"};
            
            SG::ReadHandleKey<xAOD::sTgcPadContainer> m_readKeyPad{this, "PadKey", "xAODsTgcPads", 
                                                                   "Name of the xAOD::sTgcPadContainer"};

            SG::WriteHandleKey<xAOD::sTgcMeasContainer> m_writeKey{this, "WriteKey", "xAODsTgcMeasurements"};
    };
}

#endif