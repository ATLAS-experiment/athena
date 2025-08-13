/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGSTEERMONITOR_DECISIONCOLLECTORTOOL_H
#define TRIGSTEERMONITOR_DECISIONCOLLECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include <string>


/**
 * @class DecisionCollectorTool
 * @brief
 **/
class DecisionCollectorTool : public AthAlgTool {
public:
  using AthAlgTool::AthAlgTool;

  virtual StatusCode initialize() override;

  /// Get decision IDs for the current event
  void getDecisions( std::vector<TrigCompositeUtils::DecisionID>&, const EventContext& ) const;

  /// Get decision IDs and sequences for the current event
  void getDecisions( std::vector<TrigCompositeUtils::DecisionID>&, std::set<std::string>&, const EventContext& ) const;

  /// Get configured sequence names
  void getSequencesNames( std::set<std::string>& ) const;

private:
  SG::ReadHandleKeyArray<TrigCompositeUtils::DecisionContainer> m_decisionsKey{
    this, "Decisions", {}, "Containers from which the decisions need to be read" };

  // in future we will also need a property to filter only the desired decision for combined chains (partial decisions should not be accounted)
};

#endif // TRIGSTEERMONITOR_DECISIONCOLLECTORTOOL_H
