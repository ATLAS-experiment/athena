#ifndef JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H
#define JETSUBSTRUCTUREMOMENTTOOLS_LUNDVARIABLESTOOL_H

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"
#include "fastjet/PseudoJet.hh" 
class LundVariablesTool : public JetSubStructureMomentToolsBase {
  ASG_TOOL_CLASS(LundVariablesTool, IJetModifier)

public:
  LundVariablesTool(const std::string& name);
  int modifyJet(xAOD::Jet& injet) const override;
  void print() const override;
  struct Declustering {
    fastjet::PseudoJet jj{}, j1{}, j2{};  
    double pt = -999, m = -999;
    double pt1 = -999, pt2 = -999, delta_R = -999, z = -999, kt = -999, varphi = -999, eta = -999, E = -999;
    bool exclude = false;
    int idp1 = -1, idp2 = -1;
  };

  static std::vector<Declustering> getLundVar(std::vector<fastjet::PseudoJet> v_jcs);

};

#endif
