#ifndef MUONCSVDUMP_MuonSimHitCsvDumperAlg_H
#define MUONCSVDUMP_MuonSimHitCsvDumperAlg_H
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

   

#include <AthenaBaseComps/AthAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadHandleKeyArray.h>

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

/** The CsvMuonSimHitDumper reads a Simulation Hit container for muons and dumps information to csv files**/

class MuonSimHitCsvDumperAlg: public AthAlgorithm {

   public:

   MuonSimHitCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator);
   ~MuonSimHitCsvDumperAlg() = default;


    StatusCode initialize() override;
    StatusCode execute() override;

   private:

    
    SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_inSimHitKey{
    this, "MuonSimHitKey",{}, "List of sim hit containers"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    /// Access to the readout geometry
   const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

   size_t m_event{0};
};
#endif
