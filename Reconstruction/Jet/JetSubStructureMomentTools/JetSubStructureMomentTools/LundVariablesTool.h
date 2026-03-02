#ifndef JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H
#define JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H

#include "fastjet/PseudoJet.hh"
#include "JetInterface/IJetDecorator.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AthContainers/AuxElement.h"

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
