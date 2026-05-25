/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef CPTOOLTESTS_MuonEfficiencyCorrections_TESTALG
#define CPTOOLTESTS_MuonEfficiencyCorrections_TESTALG

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

#include "MuonEfficiencyCorrections/MuonSFTestHelper.h"

#include "MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"

#include "MuonTesterTree/MuonTesterTree.h"

namespace CP {

/// small test algorithm to quickly test/demonstrate the usage of the MuonEfficiencyCorrections code within athena

    class MuonScaleFactorTestAlg: public AthHistogramAlgorithm {

        public:
            /// Regular Algorithm constructor
            MuonScaleFactorTestAlg(const std::string& name, ISvcLocator* svcLoc);

            /// Function initialising the algorithm
            StatusCode initialize() override;
            /// Function finalizing the algortihm
            StatusCode finalize() override;
            /// Function executing the algorithm
            StatusCode execute() override;
            virtual ~MuonScaleFactorTestAlg() = default;

        private:
            SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};
            /// muon container
            SG::ReadHandleKey<xAOD::MuonContainer> m_sgKey{this, "SGKey", "Muons"};

            ToolHandleArray<IMuonEfficiencyScaleFactors> m_effiTools{this, "EfficiencyTools", {}};
            ToolHandleArray<IMuonEfficiencyScaleFactors> m_comparisonTools{this, "EfficiencyToolsForComparison", {}};

            /// Scale factor tool
            ToolHandle<IPileupReweightingTool> m_prw_Tool{this, "PileupReweightingTool", ""};
            ToolHandle<IMuonSelectionTool> m_sel_tool{this, "MuonSelectionTool", ""};

            


            Gaudi::Property<std::string> m_defaultRelease{this, "DefaultRelease", ""};
            Gaudi::Property<std::string> m_validRelease{this, "ValidationRelease", ""};
            
            Gaudi::Property<float> m_pt_cut{this, "MinPt", -1.};
            Gaudi::Property<float> m_eta_cut{this, "MaxEta", -1.};
            Gaudi::Property<int> m_muon_quality{this, "MinQuality", 
                                                static_cast<int>(xAOD::Muon::Quality::Loose)};

            MuonVal::MuonTesterTree m_tree{"MuonSFTester", "MUONEFFTESTER"};

            MuonVal::ScalarBranch<float>& m_muonPt{m_tree.newScalar<float>("muon_pt")};
            MuonVal::ScalarBranch<float>& m_muonEta{m_tree.newScalar<float>("muon_eta")};
            MuonVal::ScalarBranch<float>& m_muonPhi{m_tree.newScalar<float>("muon_phi")};
            MuonVal::ScalarBranch<int>& m_muonQ{m_tree.newScalar<int>("muon_q")};

            std::vector<std::shared_ptr<TestMuonSF::MuonEffiBranch>> m_sfBranches{};

            

    };

} // namespace CP

#endif //CPTOOLTESTS_MuonEfficiencyCorrections_TESTALG
