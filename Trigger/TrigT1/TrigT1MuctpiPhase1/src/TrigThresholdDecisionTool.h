// This file is really -*- C++ -*-.

/*                                                                                                                      
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1MUCTPIPHASE1_TRIGTHRESHOLDECISIONTOOL_H
#define TRIGT1MUCTPIPHASE1_TRIGTHRESHOLDECISIONTOOL_H

/*
  Tool to help perform decision about trigger threshold given ROI data
*/

#include "TrigConfData/L1ThrExtraInfo.h"
#include "TrigConfData/L1Threshold.h"
#include "TrigConfData/L1Menu.h"
#include "TrigConfInterfaces/ITrigConfigSvc.h"

#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"
#include "TrigT1Interfaces/ITrigThresholdDecisionTool.h"

#include "HLTSeeding/IRoIThresholdsTool.h"
#include "xAODTrigger/MuonRoI.h"
#include "xAODTrigger/MuonRoIContainer.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <utility>

namespace LVL1 {

  namespace MURoIThresholdsToolParams {
    extern const char ContainerName[];
    extern const char ThresholdType[];
    using BaseClass = RoIThresholdsTool<xAOD::MuonRoI, xAOD::MuonRoIContainer, ContainerName, ThresholdType>;
  }

  class TrigThresholdDecisionTool : public extends<MURoIThresholdsToolParams::BaseClass, ITrigThresholdDecisionTool>
  {
  public:
    using parsedFlagsMap = std::unordered_map<std::string, std::vector<std::vector<std::string_view> > >;
    TrigThresholdDecisionTool(const std::string& type, 
			      const std::string& name, 
			      const IInterface* parent);
    
    virtual StatusCode initialize() override;
    virtual StatusCode start() override;

    virtual uint64_t getPattern(const EventContext& ctx,
                                const xAOD::MuonRoI& roi,
                                const ThrVec& menuThresholds,
                                const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const override;

    virtual uint64_t getPattern(uint32_t dataWord,
                                const ThrVec& menuThresholds,
                                const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const;

    virtual
    std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >
    getThresholdDecisions(uint32_t dataWord,
                          const EventContext& eventContext) const override;
    virtual
    std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >
    getThresholdDecisions(uint32_t dataWord,
                          const ThrVec& menuThresholds,
                          const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const override;

    virtual
    std::pair<std::string, double> getMinThresholdNameAndValue(const std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >& decisions,
							       const double& eta = 0) const override;

    static std::string getShapedFlags(std::string_view flags);

  protected:

    struct TgcDecisionMask {
        uint8_t mask{0};

        // Computes the 0-7 bit position from flags
        static constexpr unsigned int bitPosition(bool F, bool C, bool H) {
            return F + (2 * C) + (4 * H);
        }

        void setPassed(bool F, bool C, bool H) {
            mask |= (1 << bitPosition(F, C, H));
        }

        bool isPassed(bool F, bool C, bool H) const {
            return (mask & (1 << bitPosition(F, C, H))) != 0;
        }
    };

    struct RpcDecisionMask {
        uint8_t mask{0};

        void setPassed(bool M) {
            mask |= (1 << static_cast<unsigned int>(M));
        }

        bool isPassed(bool M) const {
            return (mask & (1 << static_cast<unsigned int>(M))) != 0;
        }
    };
    //Function that performs the actual initialization of the tool. 
    StatusCode configureToolFromMenu(const TrigConf::L1Menu& l1Menu) const;


    bool isExcludedRPCROI(const TrigConf::L1ThrExtraInfo_MU& menuExtraInfo,
                          const std::string& rpcExclROIList,
                          unsigned roi,
                          unsigned sectorID,
                          bool isSideC) const;

    bool getTGCDecision(const std::string& tgcFlags, bool F, bool C, bool H) const;
    void makeTGCDecision(const std::string& tgcFlags, bool F, bool C, bool H, const parsedFlagsMap& parsed_flags) const;

    bool getRPCDecision(const std::string& rpcFlags, bool M) const;
    void makeRPCDecision(const std::string& rpcFlags, bool M, const parsedFlagsMap& parsed_flags) const;

    void parseFlags(const std::string& flags, parsedFlagsMap& parsed_flags) const;

    ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_rpcTool{this, "RPCRecRoiTool", "LVL1::TrigT1RPCRecRoiTool/LVL1__TrigT1RPCRecRoiTool", "Tool to get the eta/phi coordinates in the RPC"};
    ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_tgcTool{this, "TGCRecRoiTool", "LVL1::TrigT1TGCRecRoiTool/LVL1__TrigT1TGCRecRoiTool", "Tool to get the eta/phi coordinates in the TGC"};


    //buffered set of decisions for words that have been checked for each TGC/RPC flag
    mutable std::unordered_map<std::string, TgcDecisionMask > m_tgcFlag_decisions ATLAS_THREAD_SAFE{};
    mutable std::unordered_map<std::string, RpcDecisionMask > m_rpcFlag_decisions ATLAS_THREAD_SAFE{};
    
    //configuration that toddle the L1menu loading sfrom xAOD as metadata or from det store
    ServiceHandle<TrigConf::ITrigConfigSvc> m_configSvc{this, "TrigConfigSvc", "TrigConf::xAODConfigSvc"};
    Gaudi::Property<bool> m_MenuFromxAOD {this, "MenuFromxAOD", false, "Flag to enable loading the L1 menu from xAOD as metadata instead of the detector store"};
    mutable std::atomic<bool> m_isInitialized ATLAS_THREAD_SAFE{false};
    mutable std::mutex m_mutex ATLAS_THREAD_SAFE{};
  };

}


#endif
