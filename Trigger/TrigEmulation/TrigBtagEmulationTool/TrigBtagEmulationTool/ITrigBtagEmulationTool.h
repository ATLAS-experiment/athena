/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef I_TRIGBTAGEMULATIONTOOL_H
#define I_TRIGBTAGEMULATIONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "TrigBtagEmulationTool/EmulContext.h"
#include "xAODJet/Jet.h"
#include <string>
#include <TLorentzVector.h>

namespace Trig {

  class ITrigBtagEmulationTool 
    : virtual public asg::IAsgTool {
  public:
    virtual const EmulContext& populateJetManagersTriggerObjects() const = 0;
    virtual bool isPassed(const std::string& chain) const = 0;
    virtual bool isPassed(const std::string& chain, const EmulContext&) const = 0;
    virtual std::unordered_map<std::string, std::vector<std::pair<const xAOD::Jet*, bool>>> getEmulatedJets(std::string) const = 0;
  };

}

#endif
