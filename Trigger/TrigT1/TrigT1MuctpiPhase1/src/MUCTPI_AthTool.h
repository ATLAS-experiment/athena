// This file is really -*- C++ -*-.

/*                                                                                                                      
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1MUCTPIPHASE1_MUCTPI_ATHTOOL_H
#define TRIGT1MUCTPIPHASE1_MUCTPI_ATHTOOL_H

/*
  Tool to perform simulation of PhaseI MUCTPI board
*/

#include "SimController.h"
#include "MUCTPIResults.h"

#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"
#include "TrigT1Interfaces/MuCTPICTP.h"
#include "TrigT1Interfaces/MuCTPIL1Topo.h"
#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"

#include "xAODTrigger/MuonRoIContainer.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "Gaudi/Property.h"

#include <optional>
#include <functional>

namespace LVL1 {
  class TrigThresholdDecisionTool;
}

namespace LVL1MUCTPIPHASE1 {

  class MUCTPI_AthTool : public AthAlgTool
  {
    
  public:
    
    MUCTPI_AthTool(const std::string& type, const std::string& name, 
		   const IInterface* parent);

    virtual ~MUCTPI_AthTool() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode start() override;
    StatusCode execute() const;

  private:

    /// Event loop method for running as part of digitization
    StatusCode executeFromDigi() const;
    /// Event loop method for running on an AOD file
    StatusCode executeFromAOD() const;
    /// Event loop method for running on an RDO file
    StatusCode executeFromRDO() const;
    /// Save the outputs of the simulation into StoreGate
    StatusCode saveOutput(std::optional<std::reference_wrapper<MUCTPIResults>> results, int bcidOffset = 0) const;

    
    static constexpr std::array<int,4> m_bcidOffsetList{-2,-1,1,2};

 
    SG::ReadHandleKey<LVL1MUONIF::Lvl1MuCTPIInputPhase1> m_muctpiPhase1KeyRPC{this, "MuctpiPhase1LocationRPC", "L1MuctpiStoreRPC", "Location of muctpiPhase1 for Rpc"};
    SG::ReadHandleKey<LVL1MUONIF::Lvl1MuCTPIInputPhase1> m_muctpiPhase1KeyTGC{this, "MuctpiPhase1LocationTGC", "L1MuctpiStoreTGC", "Location of muctpiPhase1 for Tgc"};
    SG::WriteHandleKey<LVL1::MuCTPICTP> m_MuCTPICTPWriteKey{this, "MuCTPICTPLocation", LVL1MUCTPI::DEFAULT_MuonCTPLocation, "Location of MuCTPICTP"};
    SG::WriteHandleKeyArray<xAOD::MuonRoIContainer> m_MuCTPI_xAODWriteKeys {
      this, "MUCTPI_xAODLocation", {
        "LVL1MuonRoIsBCm2", "LVL1MuonRoIsBCm1",
        "LVL1MuonRoIs",
        "LVL1MuonRoIsBCp1", "LVL1MuonRoIsBCp2"
      },
      "Output keys for xAOD::MuonRoIContainer, one per time slice"
    };
    SG::WriteHandleKeyArray<LVL1::MuCTPIL1Topo> m_MuCTPIL1TopoKeys {
      this, "L1TopoOutputLocID", {
        LVL1MUCTPI::DEFAULT_MuonL1TopoLocation+"-2", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation+"-1",
        LVL1MUCTPI::DEFAULT_MuonL1TopoLocation,
        LVL1MUCTPI::DEFAULT_MuonL1TopoLocation+"1", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation+"2"
      },
      "Output keys for MuCTPItoL1Topo, one per time slice"
    };


    // Locations of the inputs and outputs of the simulation in StoreGate:
    static const std::string m_DEFAULT_locationMuCTPItoCTP;
    static const std::string m_DEFAULT_locationMuCTPItoRoIB;
    static const std::string m_DEFAULT_L1MuctpiStoreLocationRPC;
    static const std::string m_DEFAULT_L1MuctpiStoreLocationTGC;
    static const std::string m_DEFAULT_AODLocID;
    static const std::string m_DEFAULT_RDOLocID;
    static const std::string m_DEFAULT_barrelRoIFile;
    static const std::string m_DEFAULT_ecfRoIFile;
    static const std::string m_DEFAULT_side0LUTFile;
    static const std::string m_DEFAULT_side1LUTFile;

    /////////////////////
    // Gaudy properties
    ////////////////////

    // Properties for the overlap treatment
    Gaudi::Property<std::string> m_overlapStrategyName { this, "OverlapStrategyName", "LUT", "Overlap strategy for the MUCTPI. Options: \"LUT\" or \"NULL\" " };
    Gaudi::Property<std::string> m_lutXMLFile { this, "LUTXMLFile", "TrigConfMuctpi/overlapRun3_20201214.xml", "xml file with the overlap handling for the MUCTPI. Default file for Run3: \"TrigConfMuctpi/overlapRun3_20201214.xml\" " };
    
    // Properties for the output generation for L1Topo
    Gaudi::Property<std::string> m_barrelRoIFile { this, "BarrelRoIFile", m_DEFAULT_barrelRoIFile, "" };
    Gaudi::Property<std::string> m_ecfRoIFile { this, "EndcapForwardRoIFile", m_DEFAULT_ecfRoIFile, "" };
    Gaudi::Property<std::string> m_side0LUTFile { this, "Side0LUTFile", m_DEFAULT_side0LUTFile, "" };
    Gaudi::Property<std::string> m_side1LUTFile { this, "Side1LUTFile", m_DEFAULT_side1LUTFile, "" };
    
    // Properties for the input selection
    Gaudi::Property<std::string> m_inputSource { this, "InputSource", "DIGITIZATION", "" };
    Gaudi::Property<std::string> m_aodLocId { this, "AODLocID", m_DEFAULT_AODLocID, "" };
    Gaudi::Property<std::string> m_rdoLocId { this, "RDOLocID", m_DEFAULT_RDOLocID, "" };
    Gaudi::Property<std::string> m_rdoOutputLocId { this, "RDOOutputLocID", m_DEFAULT_RDOLocID, "" };
    Gaudi::Property<std::string> m_roiOutputLocId { this, "RoIOutputLocID", m_DEFAULT_locationMuCTPItoRoIB, "" };
    Gaudi::Property<std::string> m_ctpOutputLocId { this, "CTPOutputLocID", m_DEFAULT_locationMuCTPItoCTP, "" };

    // These are just here for flexibility, normally they should not be changed:
    Gaudi::Property<std::string> m_tgcLocId { this, "TGCLocID", m_DEFAULT_L1MuctpiStoreLocationTGC, "" };
    Gaudi::Property<std::string> m_rpcLocId { this, "RPCLocID", m_DEFAULT_L1MuctpiStoreLocationRPC, "" };

    ServiceHandle<StoreGateSvc> m_detStore { this, "DetectorStore", "StoreGateSvc/DetectorStore", "Detector store to get the menu" };

    SimController m_theMuctpi;
    ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_rpcTool{this, "RPCRecRoiTool", "LVL1::TrigT1RPCRecRoiTool/LVL1__TrigT1RPCRecRoiTool", "Tool to get the eta/phi coordinates in the RPC"};
    ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_tgcTool{this, "TGCRecRoiTool", "LVL1::TrigT1TGCRecRoiTool/LVL1__TrigT1TGCRecRoiTool", "Tool to get the eta/phi coordinates in the TGC"};
    ToolHandle<LVL1::TrigThresholdDecisionTool> m_trigThresholdDecisionTool{this, "TrigThresholdDecisionTool", "LVL1::TrigThresholdDecisionTool/LVL1__TrigThresholdDecisionTool", "Tool to get pass/fail of each trigger threshold"};

    /// Function pointer to the execute function we want to use:
    StatusCode ( LVL1MUCTPIPHASE1::MUCTPI_AthTool::*m_executeFunction )( void ) const{};
    
  };
}

#endif // TRIGT1MUCTPIPHASE1_MUCTPI_ATHTOOL_H
