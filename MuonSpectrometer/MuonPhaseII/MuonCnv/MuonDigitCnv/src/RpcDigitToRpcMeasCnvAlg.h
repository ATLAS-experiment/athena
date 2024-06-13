/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONDIGICNV_RPCDIGITTORPCMEASCNVALG_H
#define MUONDIGICNV_RPCDIGITTORPCMEASCNVALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <MuonDigitContainer/RpcDigitContainer.h>
#include <xAODMuonPrepData/RpcStripContainer.h>
#include <xAODMuonPrepData/RpcStrip2DContainer.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

namespace MuonR4{
    /***
     *  @brief Algorithm converting the rpc digit container directly into xAOD::Measurements. 
     *         Use case: Fast digitization as long as the cabling maps of the RDOs are incomplete
     */
    class RpcDigitToRpcMeasCnvAlg: public AthReentrantAlgorithm {
        public:
           
            RpcDigitToRpcMeasCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

            ~RpcDigitToRpcMeasCnvAlg() = default;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            void convert(const RpcDigit& digit,
                         xAOD::RpcStripContainer& strips) const;

            void convert(const RpcDigit& digit,
                        xAOD::RpcStrip2DContainer& biStrips) const;
            
            SG::ReadHandleKey<RpcDigitContainer> m_digitKey{this, "DigitKey", "RPC_DIGITS",
                                                            "Digits to convert."};

            SG::WriteHandleKey<xAOD::RpcStripContainer> m_stripKey{this, "StripKey", "xRpcStrips"};

            SG::WriteHandleKey<xAOD::RpcStrip2DContainer> m_stripBIKey{this, "Strip2DKey", "xRpcBILStrips", ""};

            Gaudi::Property<bool> m_writeBIClust{this, "WriteBI", true};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /// BIL station index
            int m_stIdx_BIL{-1};
    };
}
#endif