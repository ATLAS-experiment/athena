/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHITCNV_xAODSimHitToMmMeasurementCnvAlg_H
#define XAODMUONSIMHITCNV_xAODSimHitToMmMeasurementCnvAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonPrepData/MMClusterContainer.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include <AthenaKernel/IAthRNGSvc.h>
#include <CLHEP/Random/RandomEngine.h>

#include "MuonCondData/NswErrorCalibData.h"

/**
 *  The xAODSimHitTosTGCMasCnvAlg is a short cut towards the  stgc strip measurement
 *  It takes the SimHits and applies a smearing on their radial position according to the uncertainty parametrization derived for Run 3 MC
*/

class xAODSimHitToMmMeasCnvAlg : public AthReentrantAlgorithm {
    public:
        xAODSimHitToMmMeasCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);

        ~xAODSimHitToMmMeasCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override final;
        StatusCode initialize() override final; 
        StatusCode finalize() override final;
    
    private:
        CLHEP::HepRandomEngine* getRandomEngine(const EventContext& ctx) const;
  
        SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_readKey{this, "InputCollection", "xMmSimHits",
                                                              "Name of the new xAOD SimHit collection"};
        
        SG::WriteHandleKey<xAOD::MMClusterContainer> m_writeKey{this, "OutputContainer", "xAODMMClusters", 
                                                                "Output container"};

        /// Access to the new readout geometry
        const MuonGMR4::MuonDetectorManager* m_DetMgr{nullptr};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  // Random number service
        Gaudi::Property<std::string> m_streamName{this, "RandomStream", "MmSimHitForkLifting"};

        SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                         "Key of the parametrized NSW uncertainties"};

        mutable std::atomic<unsigned> m_allHits ATLAS_THREAD_SAFE{};
        mutable std::atomic<unsigned> m_acceptedHits ATLAS_THREAD_SAFE{};

};

#endif