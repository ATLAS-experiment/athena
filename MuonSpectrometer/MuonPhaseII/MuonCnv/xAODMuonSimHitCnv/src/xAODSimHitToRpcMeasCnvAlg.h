/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_xAODSimHitToRpcMeasurementCnvAlg_H
#define XAODMUONSIMHITCNV_xAODSimHitToRpcMeasurementCnvAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <xAODMuonSimHit/MuonSimHitContainer.h>

#include <xAODMuonPrepData/RpcStripContainer.h>
#include <xAODMuonPrepData/RpcStrip2DContainer.h>


#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include <AthenaKernel/IAthRNGSvc.h>
#include <CLHEP/Random/RandomEngine.h>
#include "CxxUtils/checker_macros.h"
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
        StatusCode finalize() override;
    
    private:
        CLHEP::HepRandomEngine* getRandomEngine(const EventContext& ctx) const;
  
        /** @brief Smears the local simHit position orthogonal to the strip and 
         *         writes a 1D rpc strip measurement 
        */
        void digitizeHit(const double hitTime,
                         const double locPosOnStrip,
                         const MuonGMR4::StripDesignPtr& designPtr,
                         const Identifier& gasGapId,
                         const bool measuresPhi,
                         xAOD::RpcStripContainer& prdContainer,
                         CLHEP::HepRandomEngine* engine) const;
        
        void digitizeHit(const double hitTime,
                         const Amg::Vector2D& locPosOnStrip,
                        const MuonGMR4::StripDesignPtr& designPtr,
                        const Identifier& gasGapId,
                        xAOD::RpcStrip2DContainer& prdContainer,
                        CLHEP::HepRandomEngine* engine) const;
        
        /** @brief Returns the number of the strip that's closest to the hit position on the strip plane
         *          -1 is returned if the hit is outside the strip panel acceptance 
         * */
        int stripNumber(const MuonGMR4::StripDesign& design,
                        const Amg::Vector2D& locHitPosOnPlane) const;


        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_readKey{this, "InputCollection", "xRpcSimHits",
                                                              "Name of the new xAOD SimHit collection"};
        
        SG::WriteHandleKey<xAOD::RpcStripContainer> m_writeKey{this, "OutputContainer", "xRpcStrips", 
                                                                "Output container"};

        SG::WriteHandleKey<xAOD::RpcStrip2DContainer> m_writeKeyBI{this, "OutputContainerBI", "xRpcBILStrips",
                                                                  "Output container of the 2D BIL rpc strips"};
        /// Access to the new readout geometry
        const MuonGMR4::MuonDetectorManager* m_DetMgr{nullptr};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  // Random number service
        Gaudi::Property<std::string> m_streamName{this, "RandomStream", "RpcSimHitForkLifting"};

        mutable std::array<std::atomic<unsigned>, 2> m_allHits ATLAS_THREAD_SAFE{};
        mutable std::array<std::atomic<unsigned>, 2> m_acceptedHits ATLAS_THREAD_SAFE{};

        int m_stIdxBIL{-1};

};

#endif