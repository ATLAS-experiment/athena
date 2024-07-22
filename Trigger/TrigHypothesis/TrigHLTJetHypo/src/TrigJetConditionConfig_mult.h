/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_MULT_H
#define TRIGJETCONDITIONCONFIG_MULT_H

/*
Condiguration AlgTool for  Mult onditions to be run in FastReduction
PS 
*/

#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_mult:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_mult(const std::string& type,
			      const std::string& name,
			      const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:


  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min multiplicity"};

  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max multiplicity"};

  std::size_t m_min_szt{0};
  std::size_t m_max_szt{0};
  
  StatusCode checkVals()  const;
};
#endif
