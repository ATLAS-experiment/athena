/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H
#define JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H


#include "JetInterface/IJetDecorator.h"
#include "AsgTools/AsgTool.h"

#include "fastjet/PseudoJet.hh"

#include <string>
#include <vector>

class LundVariablesTool : public asg::AsgTool, virtual public IJetDecorator {
  ASG_TOOL_CLASS(LundVariablesTool, IJetDecorator)

  public:
    LundVariablesTool(const std::string& name);
    StatusCode decorate(const xAOD::JetContainer& jets) const override;
    struct Declustering {
      fastjet::PseudoJet jj{}, j1{}, j2{};  
      double pt = -999, m = -999;
      double pt1 = -999, pt2 = -999, delta_R = -999, z = -999, kt = -999, varphi = -999, eta = -999, E = -999;
      bool exclude = false;
      int idp1 = -1, idp2 = -1;
    };

    static std::vector<Declustering> getLundVar(std::vector<fastjet::PseudoJet> v_jcs);

    private:
     std::string m_prefix;

  };

#endif
