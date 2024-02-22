/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_GNTAU_H
#define TRIGJETCONDITIONCONFIG_GNTAU_H


#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_gntau:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:

  TrigJetConditionConfig_gntau(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:

  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min gntau cut value"};

  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max gntau cut value"};

  Gaudi::Property<std::string> m_name_ptau{
    this, "namePtau", {}, "ptau accessor"};
  Gaudi::Property<std::string> m_name_pu{
    this, "namePu", {}, "pu accessor"};
  Gaudi::Property<std::string> m_name_valid{
    this, "nameValid", {}, "validity check"};

  StatusCode checkVals()  const;
};
#endif
