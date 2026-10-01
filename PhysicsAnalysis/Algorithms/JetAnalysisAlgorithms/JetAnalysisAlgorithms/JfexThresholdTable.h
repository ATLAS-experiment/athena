/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_ANALYSIS_ALGORITHMS__JFEX_THRESHOLD_TABLE_H
#define JET_ANALYSIS_ALGORITHMS__JFEX_THRESHOLD_TABLE_H

#include <AsgMessaging/MsgStream.h>
#include <AsgMessaging/StatusCode.h>
#include <AsgTools/ToolHandle.h>
#include <TrigConfInterfaces/ITrigConfigTool.h>
#include <xAODTrigger/jFexSRJetRoIContainer.h>

#include <string>
#include <vector>

namespace CP
{
  /// @brief the Phase-I L1 jFEX threshold bit-to-name table, taken from the
  /// L1 menu, shared by the algorithms decoding jFEX `thresholdPatterns`
  ///
  /// The table has to be (re)built lazily from execute(), as beginInputFile()
  /// fires before xAODConfigSvc publishes the menu (map::at). It is rebuilt
  /// whenever the L1 menu name changes.
  class JfexThresholdTable final
  {
  public:
    /// @brief build the table from the current L1 menu, unless it is already
    /// built for a menu of the same name
    StatusCode update (ToolHandle<TrigConf::ITrigConfigTool>& trigConfigTool,
                       const std::string& thresholdType,
                       const EventContext& ctx, MsgStream& msg);

    /// @brief the names of the menu thresholds whose bits are set in the
    /// RoI's `thresholdPatterns` (empty if the pattern is not available)
    std::vector<std::string> decode (const xAOD::jFexSRJetRoI& roi) const;

  private:
    std::vector<std::string> m_names;
    bool m_loaded {false};
    std::string m_menuName;
  };
}

#endif
