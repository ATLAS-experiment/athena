/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "TrkVKalVrtCore/Derivt.h"
namespace Trk {

std::ostream& operator<<(std::ostream& out, const VKConstraintBase& cnst) {
  int NTRK = cnst.f0t.size();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  out << " Base constraint derivatives for NTRK=" << NTRK
      << " CNST dim=" << cnst.NCDim << std::endl;
  out << " Momentum derivatives " << std::endl;
  for (int ic = 0; ic < cnst.NCDim; ic++) {
    out << "   d(...)/dTheta  d(...)/dPhi   d(...)/dInvR   NC=" << ic
        << std::endl;
    for (int i = 0; i < NTRK; i++) {
      out << cnst.f0t[i][ic].X << ", " << cnst.f0t[i][ic].Y << ", "
          << cnst.f0t[i][ic].Z << std::endl;
    }
    out << "   d(...)/dXv  d(...)/dYy   d(...)/Zv" << std::endl;
    out << cnst.h0t[ic].X << ", " << cnst.h0t[ic].Y << ", " << cnst.h0t[ic].Z
        << std::endl;
    out << " aa=" << cnst.aa[ic] << std::endl;
  }
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKMassConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  int NP = cnst.m_usedParticles.size();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  out << " Mass constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << std::endl;
  out << " * target mass: " << cnst.getTargetMass() << std::endl;
  out << " * particle indexes: ";
  for (int i = 0; i < NP; i++) {
    out << cnst.m_usedParticles[i] << ", ";
  }
  out << std::endl;
  out << " * particle masses: ";
  for (int i = 0; i < NP; i++) {
    out << vk->TrackList[cnst.m_usedParticles[i]]->getMass() << ", ";
  }
  out << std::endl;
  out << dynamic_cast<const VKConstraintBase&>(cnst) << '\n';
  out.precision(6);  // restore default
  return out;
}
std::ostream& operator<<(std::ostream& out, const VKPhiConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  out << " Phi constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << std::endl;
  out << dynamic_cast<const VKConstraintBase&>(cnst) << '\n';
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKThetaConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  out << " Theta constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << std::endl;
  out << dynamic_cast<const VKConstraintBase&>(cnst) << '\n';
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKPointConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  if (!cnst.onlyZ()) {
    out << " Point constraint  (total NTRK=" << vk->TrackList.size() << ")"
        << std::endl;
  } else {
    out << " Z point constraint  (total NTRK=" << vk->TrackList.size() << ")"
        << std::endl;
  }
  out << " target vertex=" << cnst.getTargetVertex()[0] << ", "
      << cnst.getTargetVertex()[1] << ", " << cnst.getTargetVertex()[2]
      << std::endl;
  out << dynamic_cast<const VKConstraintBase&>(cnst)  << '\n';
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKPlaneConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  // out.setf( std::ios::scientific); out.precision(7); out << std::endl;
  out.precision(7);
  out << std::defaultfloat;
  out << " Vertex in plane constraint  (total NTRK=" << vk->TrackList.size()
      << ")" << std::endl;
  out << " Plane(A,B,C,D):" << cnst.getA() << ", " << cnst.getB() << ", "
      << cnst.getC() << ", " << cnst.getD() << std::endl;
  out << dynamic_cast<const VKConstraintBase&>(cnst)  << '\n';
  out.precision(6);  // restore default
  return out;
}
}  // namespace Trk
