/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef XAOD_STANDALONE
#include "MuonEfficiencyCorrections/MuonSFTestHelper.h"
#include "MuonEfficiencyCorrections/UtilFunctions.h"
#include "MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h"


#include "PATInterfaces/SystematicsUtil.h"

namespace {
    constexpr double MeVtoGeV = 1.e-3;
}
namespace TestMuonSF {
        template<typename T> T getProperty(const asg::IAsgTool* interface_tool, const std::string& prop_name) {
        const asg::AsgTool* asg_tool = dynamic_cast<const asg::AsgTool*>(interface_tool);
        T prop;
        const T* HandlePtr = asg_tool->getProperty < T > (prop_name);
        if (!HandlePtr) Error("getProperty()", "Failed to retrieve property %s ", prop_name.c_str());
        else prop = (*HandlePtr);
        return prop;
    }

    //############################################################
    //                   TriggerSFBranches
    //############################################################
    TriggerSFBranches::TriggerSFBranches(MuonVal::MuonTesterTree& tree, 
                                         const ToolHandle<CP::IMuonTriggerScaleFactors>& Handle, 
                                         const std::string& muonContainer,
                                         const std::string& Trigger):
        MuonVal::MuonTesterBranch{tree, "MuTriggerSF_" +Trigger},
        m_key{muonContainer},            
        m_handle{Handle},
        m_trigger{Trigger}{
    }
    bool TriggerSFBranches::init() {
        if (!declare_dependency(m_key)) return false;
        if (!m_nominal_SF.init()) return false;
        if (!m_stat_up_SF.init()) return false;
        if (!m_stat_down_SF.init()) return false;
        if (!m_sys_up_SF.init()) return false;
        if (!m_sys_down_SF.init()) return false; 
        return true;
    }

    bool TriggerSFBranches::fill(const EventContext& ctx) {
        SG::ReadHandle<xAOD::MuonContainer> readHandle{m_key, ctx};
        if (!readHandle.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve muon container "<<m_key.fullKey());
            return false;
        }
        const xAOD::MuonContainer* muons{readHandle.cptr()};
        if (getSF(muons, m_nominal_SF, CP::SystematicVariation("", 0)) == CP::CorrectionCode::Error) {
            return false;
        } if (getSF(muons, m_stat_up_SF, CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", +1)) == CP::CorrectionCode::Error) {
            return CP::CorrectionCode::Error;
        } if (getSF(muons, m_stat_down_SF, CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", -1)) == CP::CorrectionCode::Error) {
            return CP::CorrectionCode::Error;
        } if (getSF(muons, m_sys_up_SF, CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", +1)) == CP::CorrectionCode::Error) {
            return CP::CorrectionCode::Error;
        } if (getSF(muons, m_sys_down_SF, CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", -1)) == CP::CorrectionCode::Error) {
            return CP::CorrectionCode::Error;
        }
        return true;
    }
    CP::CorrectionCode TriggerSFBranches::getSF(const xAOD::MuonContainer* muons, 
                                                MuonVal::ScalarBranch<double> &branch, 
                                                const CP::SystematicVariation &syst) {
        if (muons->empty()) {
            return CP::CorrectionCode::Ok;
        }
        CP::SystematicSet syst_set;
        syst_set.insert(syst);
        if (m_handle->applySystematicVariation(syst_set) != StatusCode::SUCCESS) {
            return CP::CorrectionCode::Error;
        }
        double Var{1.};
        CP::CorrectionCode code = m_handle->getTriggerScaleFactor(*muons, Var, m_trigger);
        branch = Var;
        return code; 
    }


    //############################################################
    //                   MuonSFBranches
    //############################################################

    MuonSFBranches::MuonSFBranches(MuonVal::MuonTesterTree& tree, 
                                  const ToolHandle<CP::IMuonEfficiencyScaleFactors> &handle, 
                                  const std::string& relName) :
        MuonVal::MuonTesterBranch{tree, relName + (relName.empty() ? "" : "_") + 
                                        getProperty<std::string>(handle.get(), "WorkingPoint")},
        m_handle{handle} {}

    bool MuonSFBranches::init(){
        auto mesf = dynamic_cast<const CP::MuonEfficiencyScaleFactors*>(m_handle.get());
        if (!mesf) {
            ATH_MSG_ERROR("Given tool not MuonCP::MuonEfficiencyScaleFactors");
            return false;
        }
        m_uncorrelate_sys = mesf->uncorrelate_sys();

        for (const auto& set : CP::make_systematics_vector(m_handle->recommendedSystematics())) {
            m_SFs.emplace_back(std::make_unique<SFSet>(set, *this));
            if (!m_SFs.back()->init()) return false;
        }
        return true;
    }

    bool MuonSFBranches::fill(const EventContext& ctx) {
        for (auto& sys : m_SFs) {
            if (!sys->fill(ctx)) return false;
        }
        return true;
    }


    void MuonSFBranches::setMuon(const xAOD::Muon& muon) {
        /// Only the raw systematic sets have been activated
        /// We can loop over each set and fill it properly        
        if (!m_uncorrelate_sys) {
            for (auto& Syst_SF : m_SFs) {            
                CP::CorrectionCode cc = fillSystematic(muon, *Syst_SF);
                if (cc == CP::CorrectionCode::Error){
                    return;
                }
            }
        }  else {            
            int bin =  m_handle->getUnCorrelatedSystBin(muon);
            if (bin < 0){
                ATH_MSG_WARNING("Did not find a valid bin for muon  with pT: "<<muon.pt() *MeVtoGeV <<" GeV, eta:"<<
                        muon.eta()<<", phi: "<<muon.phi());
                return;
            }        
            for (auto& Syst_SF: m_SFs){
                /// Only process the nominal set or the set matching the syst bin
                bool process =  Syst_SF->cpSet.name().empty() ||
                                std::find_if(Syst_SF->cpSet.begin(), Syst_SF->cpSet.end(), 
                                            [bin](const CP::SystematicVariation& t){
                                                return t.isToyVariation() && t.getToyVariation().first == 
                                                                            static_cast<unsigned>(bin);
                                            }) != Syst_SF->cpSet.end();
                if (!process) continue;
                CP::CorrectionCode cc = fillSystematic(muon, *Syst_SF);
                if (cc == CP::CorrectionCode::Error) {
                    return;
                }                
            }           
        } 
    }

    CP::CorrectionCode MuonSFBranches::fillSystematic(const xAOD::Muon& muon, 
                                                      SFSet& Syst_SF) {
        if (m_handle->applySystematicVariation(Syst_SF.cpSet) != StatusCode::SUCCESS) {
            ATH_MSG_ERROR("Failed to apply variation "<< Syst_SF.cpSet.name()<<" for "<< name());
            return CP::CorrectionCode::Error;
        }  
        float sf{1.f};
        CP::CorrectionCode cc = m_handle->getEfficiencyScaleFactor(muon, sf);
        if (cc == CP::CorrectionCode::Error) {
            ATH_MSG_ERROR("Failed to retrieve the scale factor variation "<< Syst_SF.cpSet.name()<<" for "<< name());
            return CP::CorrectionCode::Error;
        }
        Syst_SF.scaleFactor = sf;
       
        /// No data-mc efficiencies provided for eta beyond 2.5
        if (std::abs(muon.eta()) > 2.5) {
            Syst_SF.dataEff = -1;
            Syst_SF.mcEff = -1;
            return CP::CorrectionCode::Ok;
        }            
        cc = m_handle->getDataEfficiency(muon, sf);
        if (cc == CP::CorrectionCode::Error) {
            ATH_MSG_ERROR("Failed to retrieve the data efficiency variation "<< Syst_SF.cpSet.name()<<" for "<< name());
            return cc;
        }
        Syst_SF.dataEff = sf;          
        cc = m_handle->getMCEfficiency(muon, sf);
        if (cc == CP::CorrectionCode::Error) {
            ATH_MSG_ERROR("Failed to retrieve the mc efficiency variation "<< Syst_SF.cpSet.name()<<" for "<< name());
            return CP::CorrectionCode::Error;
        }
        Syst_SF.mcEff = sf;
        return CP::CorrectionCode::Ok;        
    }
    
    
    //############################################################
    //                   MuonReplicaBranches
    //############################################################
    MuonReplicaBranches::MuonReplicaBranches(MuonVal::MuonTesterTree& tree, 
                                             const ToolHandle<CP::IMuonEfficiencyScaleFactors> &handle, 
                                             const std::string& relName) :
        MuonVal::MuonTesterBranch{tree, relName + (relName.empty() ? "" : "_") + 
                                 getProperty<std::string>(handle.get(), "WorkingPoint") + "SFReplica"},
        m_handle{handle} {
    }
    bool MuonReplicaBranches::init() {
        for (auto set : CP::make_systematics_vector(m_handle->recommendedSystematics())) {
            auto br =std::make_shared<MuonVal::VectorBranch<float>>(tree(), name() + MuonSFBranches::systName(set)); 
            m_SFs[set] = br;
            if(!br->init()) return false;
        }
        return true;
    }
    bool MuonReplicaBranches::fill(const EventContext& ctx) {
        for (auto& [sys, br] : m_SFs) {
            if (!br->fill(ctx)) {
                ATH_MSG_ERROR("Failed to fill "<<sys.name());
                return false;
            }
        }
        return true;
    }

    void MuonReplicaBranches::setMuon(const xAOD::Muon& muon) {
        for (auto& [cpSys, br] : m_SFs) {
            if (m_handle->applySystematicVariation(cpSys) != StatusCode::SUCCESS) {
                return;
            }
            std::vector<float> replica{};
            CP::CorrectionCode cc = m_handle->getEfficiencyScaleFactorReplicas(muon, replica);
            if (cc == CP::CorrectionCode::Error) {
                return;
            }
            for (auto repl : replica) {
                br->push_back(repl);
            }
        }
    }
}
#endif
