/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASG_ANALYSIS_ALGORITHMS__L1_JFEX_JET_THRESHOLDS_DECORATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__L1_JFEX_JET_THRESHOLDS_DECORATOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTools/ToolHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODTrigger/jFexSRJetRoIContainer.h>
#include <TrigConfInterfaces/ITrigConfigTool.h>

namespace CP
{
  class L1jFexJetThresholdsDecoratorAlg final : public EL::AnaAlgorithm
  {
  public:
    L1jFexJetThresholdsDecoratorAlg(const std::string& name,
                                    ISvcLocator* svcLoc = nullptr);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) override;

   private:
    // Lazily rebuilt in execute() — beginInputFile() fires before xAODConfigSvc publishes the menu (map::at).
    StatusCode rebuildJfexThresholdTable(const EventContext& ctx);

    SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_l1JetsKey{
      this, "l1Jets", "L1_jFexSRJetRoI",
      "Phase-I L1Calo jFEX SR jet RoI container"
    };

    Gaudi::Property<std::string> m_l1ThresholdType{
      this, "l1ThresholdType", "jJ",
      "L1 threshold type for jFEX bit-to-name lookup."
    };

    Gaudi::Property<std::string> m_decorationName{
      this, "DecorationName", "thresholds",
      "Aux key written on each L1 jFEX RoI (vector<string> of the menu "
      "threshold names whose bits are set)."
    };

    SG::WriteDecorHandleKey<xAOD::jFexSRJetRoIContainer> m_thresholdsDecorKey{
        this, "ThresholdsDecorKey", "",
        "Internal: L1 jFEX thresholds decoration key (set in initialize())."};

    ToolHandle<TrigConf::ITrigConfigTool> m_trigConfigTool{
      this, "TrigConfigTool", "TrigConf::xAODConfigTool/xAODConfigTool",
      "Trigger configuration tool (Phase-I L1 menu access)"
    };

    std::vector<std::string> m_jfexThresholdNames;
    bool m_thresholdNamesLoaded{false};
    std::string m_cachedL1MenuName;
  };
}

#endif  // ASG_ANALYSIS_ALGORITHMS__L1_JFEX_JET_THRESHOLDS_DECORATOR_ALG_H
