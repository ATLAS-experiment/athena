/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_PILEUPRM_H
#define TRIGJETCONDITIONCONFIG_PILEUPRM_H

#include "ITrigJetConditionConfig.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "./ConditionsDefs.h"

class TrigJetConditionConfig_pileuprm:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_pileuprm(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:
  
  Gaudi::Property<std::string>
    m_min{this, "min", {}, "single jet max EMF"};
  
  Gaudi::Property<std::string>
    m_max{this, "max", {}, "single jet max EMF before pileupremoval"};
  
  StatusCode checkVals()  const;
 
};
#endif
