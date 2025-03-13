/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_NNJVT_H
#define TRIGJETCONDITIONCONFIG_NNJVT_H


#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_nnjvt:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_nnjvt(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:

 Gaudi::Property<std::string>
 m_strmin{this, "min", {}, "Dummy"};

Gaudi::Property<std::string>
 m_strmax{this, "max", {}, "Dummy"};

 Gaudi::Property<std::string>
 m_nnJvtName{this, "nnJvtName", {}, "Name of jvt value & pass decoration to be read"};

  StatusCode checkVals()  const;
};
#endif
