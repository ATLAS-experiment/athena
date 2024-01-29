/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_xAODSimHitToRpcMeasurementCnvAlg_H
#define XAODMUONSIMHITCNV_xAODSimHitToRpcMeasurementCnvAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonPrepData/RpcStripContainer.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>

#include <AthenaKernel/IAthRNGSvc.h>
#include <CLHEP/Random/RandomEngine.h>
/**
 *  The xAODSimHitToRpcMasCnvAlg is a short cut towards the RpcStrip measurement
 *  The RpcSimHits are taken and expressed w.r.t. eta & phi gas gaps.
 *  The local strip position is then smeared using a Gaussian with 
 *               sigma^{2} = stripWidth / std::sqrt(12)
*/

class xAODSimHitToRpcMeasCnvAlg : public AthReentrantAlgorithm {
    public:
        xAODSimHitToRpcMeasCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~xAODSimHitToRpcMeasCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override; 
    
    private:
        CLHEP::HepRandomEngine* getRandomEngine(const EventContext& ctx) const;
  
        SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_readKey{this, "InputCollection", "xRpcSimHits",
                                                              "Name of the new xAOD SimHit collection"};
        
        SG::WriteHandleKey<xAOD::RpcStripContainer> m_writeKey{this, "OutputContainer", "xRpcStrips", 
                                                                "Output container"};

        /// Access to the new readout geometry
        const MuonGMR4::MuonDetectorManager* m_DetMgr{nullptr};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  // Random number service
        Gaudi::Property<std::string> m_streamName{this, "RandomStream", "RpcSimHitForkLifting"};

        PublicToolHandle<MuonGMR4::IMuonStationLayerSurfaceTool> m_surfaceProvTool{this, "LayerGeoTool", ""};

};

#endif