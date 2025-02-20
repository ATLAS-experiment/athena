/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/ALinePar.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GaudiKernel/SystemOfUnits.h"

std::ostream& operator<<(std::ostream& ostr, const ALinePar& par) {
  using Parameter = ALinePar::Parameter;
  ostr<<"ALine AMDB id (name,eta,phi,job)=(";
  ostr<<par.AmdbStation()<<",";
  ostr<<par.AmdbEta()<<",";
  ostr<<par.AmdbPhi()<<",";
  ostr<<par.AmdbJob()<<"), ";
  ostr<<"translation (S/Y,Z,T/X)= (";
  ostr<<par.getParameter(Parameter::transS)<<",";
  ostr<<par.getParameter(Parameter::transZ)<<",";
  ostr<<par.getParameter(Parameter::transT)<<"), ";
  ostr<<"rotation angle";
  ostr<<" around S="<<par.getParameter(Parameter::rotS)<<",";
  ostr<<" around Z="<<par.getParameter(Parameter::rotZ)<<",";
  ostr<<" around T="<<par.getParameter(Parameter::rotT)<<" ";
  return ostr;
}
void ALinePar::setParameters(float s, float z, float t, float rotS, float rotZ, float rotT) {
    m_payload[static_cast<unsigned int>(Parameter::transS)] = s;
    m_payload[static_cast<unsigned int>(Parameter::transZ)] = z;
    m_payload[static_cast<unsigned int>(Parameter::transT)] = t;
    m_payload[static_cast<unsigned int>(Parameter::rotS)  ] = rotS;
    m_payload[static_cast<unsigned int>(Parameter::rotZ)  ] = rotZ;
    m_payload[static_cast<unsigned int>(Parameter::rotT)  ] = rotT;
}

Amg::Transform3D ALinePar::delta() const {
    return Amg::getTranslateX3D(getParameter(Parameter::transS)) * 
           Amg::getTranslateY3D(getParameter(Parameter::transZ)) * 
           Amg::getTranslateZ3D(getParameter(Parameter::transT)) * 
           Amg::getRotateX3D(getParameter(Parameter::rotS)) *
           Amg::getRotateY3D(getParameter(Parameter::rotZ)) *
           Amg::getRotateZ3D(getParameter(Parameter::rotT));
}