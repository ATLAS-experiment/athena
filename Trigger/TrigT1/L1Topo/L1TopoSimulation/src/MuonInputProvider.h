/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulation_MuonInputProvider
#define L1TopoSimulation_MuonInputProvider

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/LockedHandle.h"
#include "TrigT1Interfaces/MuCTPIL1Topo.h"
#include "TrigT1Result/RoIBResult.h"
#include "TrigT1Interfaces/MuCTPIToRoIBSLink.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"
#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"

#include "xAODTrigger/MuonRoIContainer.h"

#include <vector>

namespace TrigConf
{
   class L1Menu;
} // namespace TrigConf

namespace TCS {
   class MuonTOB;
   class LateMuonTOB;
}

namespace LVL1 {

   class MuCTPIL1TopoCandidate;

   class MuonInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      MuonInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual StatusCode initialize() override;

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const override;

   private:
      TCS::MuonTOB createMuonTOB(const xAOD::MuonRoI & muonRoI, const std::vector<unsigned int> & rpcPtValues, const std::vector<unsigned int> & tgcPtValues) const;
      TCS::MuonTOB createMuonTOB(const MuCTPIL1TopoCandidate & roi) const;
      TCS::LateMuonTOB createLateMuonTOB(const xAOD::MuonRoI & muonRoI, const std::vector<unsigned int> & rpcPtValues, const std::vector<unsigned int> & tgcPtValues) const;
      TCS::LateMuonTOB createLateMuonTOB(const MuCTPIL1TopoCandidate & roi) const;

      /* 
         @brief use L1Topo convention for muon flags (1 = true/positive, -1 = false/negative, 0 = undefined)
      */
      int topoFlag(bool flag) const;

      ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_recRPCRoiTool{this, "RecRpcRoiTool", "LVL1::TrigT1RPCRecRoiTool/TrigT1RPCRecRoiTool", "RPC RoI reconstruction tool"};
      ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_recTGCRoiTool{this, "RecTgcRoiTool", "LVL1::TrigT1TGCRecRoiTool/TrigT1TGCRecRoiTool", "TGC RoI reconstruction tool"};
      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey<LVL1::MuCTPIL1Topo> m_MuCTPItoL1TopoLocation { this, "locationMuCTPItoL1Topo", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation, "Storegate key for MuCTPItoL1Topo "};
      SG::ReadHandleKey<LVL1::MuCTPIL1Topo> m_MuCTPItoL1TopoLocationPlusOne { this, "locationMuCTPItoL1Topo1", LVL1MUCTPI::DEFAULT_MuonL1TopoLocation, "Storegate key for MuCTPItoL1TopoPlusOne"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonL1RoILocation {this, "locationMuonRoI", "LVL1MuonRoIs", "Empty=Use Muctpi, LVL1MuonRoIs=Use reading from xAOD L1 RoI"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonL1RoILocationPlusOne {this, "locationMuonRoI1", "LVL1MuonRoIsBCp1", "Empty=Use Muctpi, LVL1MuonRoIs=Use reading from xAOD L1 RoI"};
    
   };
}

#endif
