/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_UHT1TAU_H
#define TRIGJETCONDITIONCONFIG_UHT1TAU_H


#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_uht1tau:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:

  TrigJetConditionConfig_uht1tau(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:

  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min uht1tau cut value"};

  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max uht1tau cut value"};

  Gaudi::Property<std::string> m_name_ptau{
    this, "namePtau", {}, "ptau accessor"};
  Gaudi::Property<std::string> m_name_pu{
    this, "namePu", {}, "pu accessor"};
  Gaudi::Property<std::string> m_name_pc{
    this, "namePc", {}, "pc accessor"};
  Gaudi::Property<std::string> m_name_pb{
    this, "namePb", {}, "pb accessor"};
  Gaudi::Property<std::string> m_name_valid{
    this, "nameValid", {}, "validity check"};

  StatusCode checkVals()  const;
};
#endif
