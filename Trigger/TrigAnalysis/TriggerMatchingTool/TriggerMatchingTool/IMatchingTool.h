// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IMATCHINGTOOL_H
#define IMATCHINGTOOL_H


// Framework include(s):
#include "AsgTools/IAsgTool.h"
#include <string_view>
#include <vector>
namespace xAOD{
  class IParticle;
}



namespace Trig {

class MatchingImplementation;

class IMatchingTool : virtual public asg::IAsgTool {
  ASG_TOOL_INTERFACE(IMatchingTool)
public:

  ///single object trigger matching. matchThreshold is typically the deltaR requirement to obtain positive matching
  virtual bool match(const xAOD::IParticle& recoObject, std::string_view chain, double matchThreshold=0.1, bool rerun=false) const = 0;
  ///multi-object trigger matching
  virtual bool match(const std::vector<const xAOD::IParticle*>& recoObjects, std::string_view chain, double matchThreshold=0.1, bool rerun=false) const = 0;

protected:
  virtual const MatchingImplementation* impl() const = 0;
};

}

#endif
