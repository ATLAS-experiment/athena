/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#ifndef XAOD_STANDALONE 
#ifndef MUONEFFICIENCYCORRECTION_MUONSFTESTHELPER_H
#define MUONEFFICIENCYCORRECTION_MUONSFTESTHELPER_H

// EDM include(s):
#include "xAODMuon/Muon.h"

// Tool Includes
#include <MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h>
#include <MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h>
#include <MuonAnalysisInterfaces/IMuonSelectionTool.h>

#include <MuonTesterTree/MuonTesterBranch.h>
#include <MuonTesterTree/ScalarBranch.h>
#include <MuonTesterTree/MuonTesterTree.h>

#include <GaudiKernel/ToolHandle.h>

//General includes
#include <memory>
#include <map>
#include <TTree.h>
#include <TH1.h>
#include <TFile.h>
//Helper class to test the Muon efficiency SFs plus their systematics
namespace TestMuonSF {
    //####################################################
    //      Helper class to write ntuples to test the    #
    //      MuonTriggerScaleFactors                      #
    //####################################################
   
    class TriggerSFBranches: public MuonVal::MuonTesterBranch {
        public:
            TriggerSFBranches(MuonVal::MuonTesterTree& tree, 
                              const ToolHandle<CP::IMuonTriggerScaleFactors>& Handle, 
                              const std::string& muonContainer,
                              const std::string& Trigger);
            virtual ~TriggerSFBranches() = default;
            
            bool fill(const EventContext& ctx) override final;
            bool init() override final;
          
        private:
            CP::CorrectionCode getSF(const xAOD::MuonContainer* muons, 
                                     MuonVal::ScalarBranch<double> &Var, 
                                     const CP::SystematicVariation &syst);

            SG::ReadHandleKey<xAOD::MuonContainer> m_key{};
            
            ToolHandle<CP::IMuonTriggerScaleFactors> m_handle{};
            std::string m_trigger{};
            MuonVal::ScalarBranch<double> m_nominal_SF{tree(), name(), 1.};
            MuonVal::ScalarBranch<double> m_stat_up_SF{tree(), name()+"_STAT_UP", 1.};
            MuonVal::ScalarBranch<double> m_stat_down_SF{tree(), name()+"_STAT_DN", 1.};
            MuonVal::ScalarBranch<double> m_sys_up_SF{tree(), name()+"_SYS_UP", 1.};
            MuonVal::ScalarBranch<double> m_sys_down_SF{tree(), name()+"_SYS_DN", 1.};
    };

    
    class MuonEffiBranch{
        public:
            virtual void setMuon(const xAOD::Muon& muon) = 0;
    };
    //###################################################################
    //      Helper class to write the scale-factor ntuples to test the  #
    //      MuonReconstruction/ Isolation/ TTVA scalefactors            #
    //###################################################################
    class MuonSFBranches: public MuonVal::MuonTesterBranch, public MuonEffiBranch {
        public:
             MuonSFBranches(MuonVal::MuonTesterTree& tree, 
                            const ToolHandle<CP::IMuonEfficiencyScaleFactors> &handle, 
                            const std::string& rel_name = "");
             
             virtual ~MuonSFBranches() = default;


             void setMuon(const xAOD::Muon& muon) override final;

             bool fill(const EventContext& ctx) override final;

             bool init() override final;

            /// Dummy helper function that encodes the systname set
            static std::string systName(const CP::SystematicSet& set) {
                return set.name().empty() ? std::string("") : std::string("__") + set.name();
            }

        private:
            ToolHandle<CP::IMuonEfficiencyScaleFactors> m_handle{};
            bool m_uncorrelate_sys{false};

            /// Helper struct to store scale factor, data efficiency + mc efficiency
            /// for each systematic variation
            struct SFSet {
                    SFSet(const CP::SystematicSet& cpSet_,
                          MuonSFBranches& parent_):
                    cpSet{cpSet_}, parent{parent_}{}
                    /// Initialization of the branches
                    bool init() {
                        return scaleFactor.init() && mcEff.init() && dataEff.init();
                    }
                    /// check that all branches are set properly
                    bool fill(const EventContext& ctx) {
                        return scaleFactor.fill(ctx) && mcEff.fill(ctx) && dataEff.fill(ctx);
                    }
                    /// Systematic set
                    const CP::SystematicSet cpSet{};
                    /// Parent from which the TTree pointer & name is retrieved
                    MuonSFBranches& parent;
                    /// Actual branches storing scaleFactor & mcEff & dataEff
                    MuonVal::ScalarBranch<float> scaleFactor{parent.tree(),parent.name()+"SF" + systName(cpSet), 1.f};
                    MuonVal::ScalarBranch<float> mcEff{parent.tree(),parent.name()+"MCEff" + systName(cpSet), 1.f};
                    MuonVal::ScalarBranch<float> dataEff{parent.tree(),parent.name()+"DataEff" + systName(cpSet),1.f};
            };
            
            CP::CorrectionCode fillSystematic(const xAOD::Muon& muon, SFSet& set);
    
            std::vector<std::unique_ptr<SFSet>> m_SFs{};

    };
    //#######################################################
    //          Helper class for writing                    #
    //          scale-factor replicas to the test-ntuples   #
    //#######################################################
    
    class MuonReplicaBranches: public MuonVal::MuonTesterBranch, public MuonEffiBranch {
        public:
            MuonReplicaBranches(MuonVal::MuonTesterTree& tree, 
                                const ToolHandle<CP::IMuonEfficiencyScaleFactors> &handle, 
                                const std::string& rel_name = "");
            
            void setMuon(const xAOD::Muon& muon) override final;

            bool init() override;
            
            bool fill(const EventContext& ctx) override;

        private:
            ToolHandle<CP::IMuonEfficiencyScaleFactors> m_handle{};
            
            std::map<CP::SystematicSet, std::shared_ptr<MuonVal::VectorBranch<float>>> m_SFs{};
    };
}
#endif
#endif
