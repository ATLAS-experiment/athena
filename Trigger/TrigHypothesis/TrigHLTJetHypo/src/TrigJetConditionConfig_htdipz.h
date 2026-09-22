/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_HTDIPZ_H
#define TRIGJETCONDITIONCONFIG_HTDIPZ_H

/*
Condiguration AlgTool for ht conditions to be run in FastReduction
PS 
*/

#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_htdipz:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_htdipz(const std::string& type,
			      const std::string& name,
			      const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:
  

  Gaudi::Property<std::string>
    m_capacity{this, "capacity", {}, "number of jets considered"};  
  Gaudi::Property<std::string> 
    m_decName_z{this, "decName_z", {}, "dipz z accessor"};
  Gaudi::Property<std::string>
    m_decName_sigma{ this, "decName_sigma", {}, "dipz sigma accessor"};
  Gaudi::Property<bool> m_sigmaIsStdDev{
    this, "sigmaIsStdDev", false,
    "sigma decoration is the standard deviation rather than -2*log(sigma)"};
  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min HT"};
  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max HT"};


  StatusCode checkVals()  const;
};
#endif
