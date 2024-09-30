
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONMEASVIEWALGS_STGCMEASVIEWALG_H
#define XAODMUONMEASVIEWALGS_STGCMEASVIEWALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODMuonPrepData/sTgcWireContainer.h>
#include <xAODMuonPrepData/sTgcStripContainer.h>
#include <xAODMuonPrepData/sTgcPadContainer.h>
#include <xAODMuonPrepData/sTgcMeasContainer.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>


/**
 * @brief: The sTgcMeasViewAlg takes all sTgcStrip, sTgcWire & sTgcPad measurements and pushes
 *         them into a common sTgcMeasContainer which is a SG::VIEW_ELEMENTS container
 * 
*/

namespace MuonR4 {
    class sTgcMeasViewAlg : public AthReentrantAlgorithm {
        public:
            sTgcMeasViewAlg(const std::string& name, ISvcLocator* pSvcLocator);

            ~sTgcMeasViewAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        private:
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;


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