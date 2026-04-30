/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/BLinePar.h"
#include <iostream>
std::ostream& operator<<(std::ostream& ostr, const BLinePar& par) {
  using Parameter = BLinePar::Parameter;
  ostr<<"Muon B-Line deformation AMDB id (name,eta,phi,job)=(";
  ostr<<par.AmdbStation()<<",";
  ostr<<par.AmdbEta()<<",";
  ostr<<par.AmdbPhi()<<",";
  ostr<<par.AmdbJob()<<"), ";  
  ostr<<" tube bow in plane bz="<<par.getParameter(Parameter::bz)<<",";
  ostr<<" tube bow out of plane bp="<<par.getParameter(Parameter::bp)<<",";
  ostr<<" bn="<<par.getParameter(Parameter::bn)<<",";
  ostr<<" cross plate sage out of plane sp="<<par.getParameter(Parameter::sp)<<",";
  ostr<<" sn="<<par.getParameter(Parameter::sn)<<",";
  ostr<<" twist tw="<<par.getParameter(Parameter::tw)<<",";
  ostr<<" parallelogram pg="<<par.getParameter(Parameter::pg)<<",";
  ostr<<" trapezoid tz="<<par.getParameter(Parameter::tr)<<",";
  ostr<<" global expansion eg="<<par.getParameter(Parameter::eg)<<",";
  ostr<<" local expansion ep="<<par.getParameter(Parameter::ep)<<",";
  ostr<<" en="<<par.getParameter(Parameter::en);
  return ostr;
}
void BLinePar::setParameters(float bz, float bp, float bn, float sp, float sn, float tw, float pg, float tr, float eg, float ep, float en) {
    m_payload[static_cast<unsigned int>(Parameter::bz)] = bz;
    m_payload[static_cast<unsigned int>(Parameter::bp)] = bp;
    m_payload[static_cast<unsigned int>(Parameter::bn)] = bn;
    m_payload[static_cast<unsigned int>(Parameter::sp)] = sp;
    m_payload[static_cast<unsigned int>(Parameter::sn)] = sn;
    m_payload[static_cast<unsigned int>(Parameter::tw)] = tw;
    m_payload[static_cast<unsigned int>(Parameter::pg)] = pg;
    m_payload[static_cast<unsigned int>(Parameter::tr)] = tr;
    m_payload[static_cast<unsigned int>(Parameter::eg)] = eg;
    m_payload[static_cast<unsigned int>(Parameter::ep)] = ep;
    m_payload[static_cast<unsigned int>(Parameter::en)] = en;
}