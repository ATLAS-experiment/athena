/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "TrkVKalVrtCore/Derivt.h"
namespace Trk {

std::ostream& operator<<(std::ostream& out, const VKConstraintBase& cnst) {
  int NTRK = cnst.f0t.size();
  out.precision(7);
  out << std::defaultfloat;
  out << " Base constraint derivatives for NTRK=" << NTRK
      << " CNST dim=" << cnst.NCDim << "\n";
  out << " Momentum derivatives " << "\n";
  for (int ic = 0; ic < cnst.NCDim; ic++) {
    out << "   d(...)/dTheta  d(...)/dPhi   d(...)/dInvR   NC=" << ic
        << "\n";
    for (int i = 0; i < NTRK; i++) {
      out << cnst.f0t[i][ic].X << ", " << cnst.f0t[i][ic].Y << ", "
          << cnst.f0t[i][ic].Z << "\n";
    }
    out << "   d(...)/dXv  d(...)/dYy   d(...)/Zv\n";
    out << cnst.h0t[ic].X << ", " << cnst.h0t[ic].Y << ", " << cnst.h0t[ic].Z
        << "\n";
    out << " aa=" << cnst.aa[ic] << std::endl;
  }
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKMassConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  int NP = cnst.m_usedParticles.size();
  out.precision(7);
  out << std::defaultfloat;
  out << " Mass constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << "\n";
  out << " * target mass: " << cnst.getTargetMass() << "\n";
  out << " * particle indexes: ";
  for (int i = 0; i < NP; i++) {
    out << cnst.m_usedParticles[i] << ", ";
  }
  out << "\n";
  out << " * particle masses: ";
  for (int i = 0; i < NP; i++) {
    out << vk->TrackList[cnst.m_usedParticles[i]]->getMass() << ", ";
  }
  out << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst) << "\n";
  out.precision(6);  // restore default
  return out;
}
std::ostream& operator<<(std::ostream& out, const VKPhiConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  out.precision(7);
  out << std::defaultfloat;
  out << " Phi constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst) << "\n";
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKThetaConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  out.precision(7);
  out << std::defaultfloat;
  out << " Theta constraint  (total NTRK=" << vk->TrackList.size() << ")"
      << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst) << "\n";
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKPointConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  out.precision(7);
  out << std::defaultfloat;
  if (!cnst.onlyZ()) {
    out << " Point constraint  (total NTRK=" << vk->TrackList.size() << ")"
        << "\n";
  } else {
    out << " Z point constraint  (total NTRK=" << vk->TrackList.size() << ")"
        << "\n";
  }
  out << " target vertex=" << cnst.getTargetVertex()[0] << ", "
      << cnst.getTargetVertex()[1] << ", " << cnst.getTargetVertex()[2]
      << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst)  << "\n";
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKPlaneConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  out.precision(7);
  out << std::defaultfloat;
  out << " Vertex in plane constraint  (total NTRK=" << vk->TrackList.size()
      << ")" << "\n";
  out << " Plane(A,B,C,D):" << cnst.getA() << ", " << cnst.getB() << ", "
      << cnst.getC() << ", " << cnst.getD() << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst)  << "\n";
  out.precision(6);  // restore default
  return out;
}

std::ostream& operator<<(std::ostream& out, const VKRadiusConstraint& cnst) {
  const VKVertex* vk = cnst.getOriginVertex();
  out.precision(7);
  out << std::defaultfloat;
  out << " Vertex in radius constraint  (total NTRK=" << vk->TrackList.size()
      << ")\n" ;
  out << " Fixed Radius:" << cnst.getRC() << "\n";
  out << dynamic_cast<const VKConstraintBase&>(cnst) << "\n";
  out.precision(6);  // restore default
  return out;
}

}  // namespace Trk
