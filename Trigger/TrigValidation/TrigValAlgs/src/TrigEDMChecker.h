/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGVALALGS_TRIGEDMCHECKER_H
#define TRIGVALALGS_TRIGEDMCHECKER_H

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ToolHandle.h"

#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"
#include "MuonCombinedToolInterfaces/IMuonPrintingTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigNavigation.h"

#include <string>

class TrigEDMChecker : public AthAnalysisAlgorithm  {

 public:
   using AthAnalysisAlgorithm::AthAnalysisAlgorithm;

   virtual StatusCode initialize() override;
   virtual StatusCode execute(const EventContext& ctx) override;

 private:
   Gaudi::Property<bool> m_doDumpAll{this, "doDumpAll", true};

   Gaudi::Property<bool> m_doDumpTrigPassBits{this, "doDumpTrigPassBits", false};
   StatusCode dumpTrigPassBits();

   Gaudi::Property<bool> m_doDumpLVL1_ROI{this, "doDumpLVL1_ROI", false};
   StatusCode dumpLVL1_ROI();

   Gaudi::Property<bool> m_doDumpxAODTrigMissingET{this, "doDumpxAODTrigMissingET", false};
   StatusCode dumpxAODTrigMissingET();

   Gaudi::Property<bool> m_doDumpxAODJetContainer{this, "doDumpxAODJetContainer", false};
   StatusCode dumpxAODJetContainer();

   Gaudi::Property<bool> m_doDumpTrigL2BphysContainer{this, "doDumpTrigL2BphysContainer", false};
   StatusCode dumpTrigL2BphysContainer();

   Gaudi::Property<bool> m_doDumpTrigEFBphysContainer{this, "doDumpTrigEFBphysContainer", false};
   StatusCode dumpTrigEFBphysContainer();

   Gaudi::Property<bool> m_doDumpxAODTrigEMCluster{this, "doDumpxAODTrigEMCluster", false};
   StatusCode dumpxAODTrigEMCluster();

   Gaudi::Property<bool> m_doDumpxAODTrigEMClusterContainer{this, "doDumpxAODTrigEMClusterContainer", false};
   StatusCode dumpxAODTrigEMClusterContainer();

   Gaudi::Property<bool> m_doDumpxAODMuonContainer{this, "doDumpxAODMuonContainer", false};
   StatusCode dumpxAODMuonContainer();

   Gaudi::Property<bool> m_doDumpxAODTrigElectronContainer{this, "doDumpxAODTrigElectronContainer", false};
   StatusCode dumpxAODTrigElectronContainer();
   
   Gaudi::Property<bool> m_doDumpxAODTrigPhotonContainer{this, "doDumpxAODTrigPhotonContainer", false};
   StatusCode dumpxAODTrigPhotonContainer();
   
   Gaudi::Property<bool> m_doDumpxAODElectronContainer{this, "doDumpxAODElectronContainer", false};
   StatusCode dumpxAODElectronContainer();
   
   Gaudi::Property<bool> m_doDumpxAODPhotonContainer{this, "doDumpxAODPhotonContainer", false};
   StatusCode dumpxAODPhotonContainer();
   
   Gaudi::Property<bool> m_doDumpxAODTauJetContainer{this, "doDumpxAODTauJetContainer", false};
   StatusCode dumpxAODTauJetContainer();
 
   Gaudi::Property<bool> m_doDumpxAODTrackParticle{this, "doDumpxAODTrackParticle", false};
   StatusCode dumpxAODTrackParticle();

   Gaudi::Property<bool> m_doDumpxAODVertex{this, "doDumpxAODVertex", false};
   StatusCode dumpxAODVertex();

   Gaudi::Property<bool> m_doDumpStoreGate{this, "doDumpStoreGate", false };
   StatusCode dumpStoreGate();

   Gaudi::Property<bool> m_doTDTCheck{this, "doTDTCheck", false };
   StatusCode dumpTDT(const EventContext& ctx);

   Gaudi::Property<bool> m_doDumpxAODTrigMinBias{this, "doDumpxAODTrigMinBias", false};
   StatusCode dumpxAODTrigMinBias();
   void dumpTrigSpacePointCounts();
   void dumpTrigT2MBTSBits();
   void dumpTrigVertexCounts();
   void dumpTrigTrackCounts();

   Gaudi::Property<bool> m_doDumpAllTrigComposite{this, "doDumpAllTrigComposite", false };
   Gaudi::Property<std::vector<std::string>> m_dumpTrigCompositeContainers{this, "dumpTrigCompositeContainers", false, "List of TCs to dump" };

   Gaudi::Property<bool> m_doDumpNavigation{this, "doDumpNavigation", false };
   StatusCode dumpNavigation(const EventContext& ctx);

   Gaudi::Property<std::string> m_dumpNavForChain {this, "DumpNavigationForChain", "", "Optional chain to restrict navigation dump info."};
   Gaudi::Property<bool> m_excludeFailedHypoNodes {this, "excludeFailedHypoNodes", false,
    "Optional flag to exclude nodes which fail the hypothesis tool for a chain when dumping navigation graphs."};

   /**
    * @brief Dump information on TrigComposite collections
    *
    * Only dumpTrigCompositeContainers are dumped unless doDumpAllTrigComposite is set
    */
   StatusCode dumpTrigComposite();

   /**
    * @brief Dump details on element links within TrigComposites
    *
    * With specific checking of the Run-3 relationships
    */
   StatusCode checkTrigCompositeElementLink(const xAOD::TrigComposite* tc, size_t element); 

   Gaudi::Property<bool> m_doDumpTrigCompsiteNavigation{this, "doDumpTrigCompsiteNavigation",false };

   /**
    * @brief Construct graph of HLT navigation in Run-3
    * @param returnValue String to populate with dot graph.
    * @param pass When using a chain filter, if the chain group passed raw.
    *
    * Navigates all TrigComposite objects in store gate and forms a relational graph in the dot format
    */
   StatusCode TrigCompositeNavigationToDot(std::string& returnValue, bool& pass);

   ToolHandle<Rec::IMuonPrintingTool> m_muonPrinter{this, "MuonPrinter", "Rec::MuonPrintingTool/MuonPrintingTool"};
   ServiceHandle<IClassIDSvc > m_clidSvc{this, "ClassIDSvc", "ClassIDSvc", "Service providing CLID info"};

   SG::ReadHandleKey< xAOD::TrackParticleContainer > m_muonTracksKey{ this, "MuonTracksKey", "HLT_IDTrack_Muon_FTF"};
   SG::ReadHandleKey< xAOD::TrigNavigation > m_navigationHandleKey{ this, "TrigNavigation", "TrigNavigation", "" };
   SG::WriteHandleKey<TrigCompositeUtils::DecisionContainer> m_decisionsKey{ this, "Decisions", "RoIDecisions", "Decisions created from TEs" };
   ToolHandle< HLT::Navigation > m_navigationTool{ this, "NavigationTool", "HLT::Navigation/Navigation", "" };
   PublicToolHandle< Trig::TrigDecisionTool > m_trigDec{ this, "TriggerDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", ""};

};

#endif
