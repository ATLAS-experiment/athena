/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_xAODSimHitTosTGCMeasurementCnvAlg_H
#define XAODMUONSIMHITCNV_xAODSimHitTosTGCMeasurementCnvAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonPrepData/sTgcStripContainer.h>
#include <xAODMuonPrepData/sTgcPadContainer.h>
#include <xAODMuonPrepData/sTgcWireContainer.h>


#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include <AthenaKernel/IAthRNGSvc.h>
#include <CLHEP/Random/RandomEngine.h>

#include "MuonCondData/NswErrorCalibData.h"

/**
 *  The xAODSimHitTosTGCMasCnvAlg is a short cut towards the  stgc strip measurement
 *  It takes the SimHits and applies a smearing on their radial position according to the uncertainty parametrization derived for Run 3 MC
*/

class xAODSimHitTosTGCMeasCnvAlg : public AthReentrantAlgorithm {
    public:
        xAODSimHitTosTGCMeasCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~xAODSimHitTosTGCMeasCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override; 
    
    private:
        CLHEP::HepRandomEngine* getRandomEngine(const EventContext& ctx) const;

        void digitizeStrip(const EventContext& ctx,
                           const xAOD::MuonSimHit& simHit,
                           xAOD::sTgcStripContainer& stripContainer,
                           CLHEP::HepRandomEngine* rndEngine) const;
  
        void digitizeWire(const EventContext& ctx,
                          const xAOD::MuonSimHit& simHit,
                          xAOD::sTgcWireContainer& wireContainer,
                          CLHEP::HepRandomEngine* rndEngine) const;

        void digitizePad(const EventContext& ctx,
                         const xAOD::MuonSimHit& simHit,
                         xAOD::sTgcPadContainer& padContainer) const;
        
        SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_readKey{this, "InputCollection", "xStgcSimHits",
                                                              "Name of the new xAOD SimHit collection"};

        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        SG::WriteHandleKey<xAOD::sTgcStripContainer> m_writeKeyStrip{this, "StripWriteKey", "xAODsTgcStrips", 
                                                                     "Write strip key"};

        SG::WriteHandleKey<xAOD::sTgcWireContainer> m_writeKeyWire{this, "WireWriteKey", "xAODsTgcWires", 
                                                                     "Write wire key"};
        SG::WriteHandleKey<xAOD::sTgcPadContainer> m_writeKeyPad{this, "PadWriteKey", "xAODsTgcPads", 
                                                                 "Write pad key"};


        /// Access to the new readout geometry
        const MuonGMR4::MuonDetectorManager* m_DetMgr{nullptr};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  // Random number service
        Gaudi::Property<std::string> m_streamName{this, "RandomStream", "sTGCSimHitForkLifting"};

        SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                                  "Key of the parametrized NSW uncertainties"};

};

#endif