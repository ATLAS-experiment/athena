/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRPC_CNVTOOLSR4_RPCRDOTOPREPDATACNVTOOL_H
#define MUONRPC_CNVTOOLSR4_RPCRDOTOPREPDATACNVTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <xAODMuonRDO/NRPCRDOContainer.h>
#include <xAODMuonPrepData/RpcStrip2DContainer.h>
#include <xAODMuonPrepData/RpcStripContainer.h>
#include <MuonCablingData/RpcCablingMap.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <GaudiKernel/PhysicalConstants.h>

namespace MuonR4{
    class RpcRdoToRpcPrepDataTool: public extends<AthAlgTool, Muon::IMuonRdoToPrepDataTool> {
        public:

            using base_class::base_class;

            ~RpcRdoToRpcPrepDataTool() = default;

            StatusCode initialize() override final;

            StatusCode decode(const EventContext& ctx, 
                              const std::vector<IdentifierHash>& idVect) const override final;

            StatusCode decode(const EventContext& ctx,
                              const std::vector<uint32_t>& robIds) const override final;

            /// Method to create the empty containers.. Only used for seeded decoding
            StatusCode provideEmptyContainer(const EventContext& ctx) const override final;

        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_rdoKey{this, "RpcRdoContainer", "NRPCRDO"};

            SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap",
                                                                    "Key of MuonNRPC_CablingMap"};


            SG::WriteHandleKey<xAOD::RpcStripContainer> m_writeKey{this, "OutputContainer", "xRpcStrips", 
                                                                  "Output container"};

            SG::WriteHandleKey<xAOD::RpcStrip2DContainer> m_writeKeyBI{this, "OutputContainerBI", "xRpcBILStrips",
                                                                      "Output container of the 2D BIL rpc strips"};
    
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};


            Gaudi::Property<bool> m_decode2DStrips{this, "decode2DStrips", true};

            Gaudi::Property<double> m_propagationVelocity{this, "propSpeed", 0.5 * Gaudi::Units::c_light,
                                                         "Propagation speed of the signal inside the strip"}; // in mm/ns
    
            Gaudi::Property<double> m_stripTimeResolution{this, "timeResolution", 0.6 * Gaudi::Units::nanosecond,
                                                          "Estimated time resolution of the strip readout"};

    };
}
#endif