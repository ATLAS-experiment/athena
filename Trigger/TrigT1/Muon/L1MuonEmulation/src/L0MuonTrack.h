/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONEMULATION_L0MUONTRACK_H
#define L1MUONEMULATION_L0MUONTRACK_H

#include <iostream>

namespace L1Muon {

class L0MuonTrack {
 public:
  L0MuonTrack() = default;
  ~L0MuonTrack() = default;

  void setTrack(const double invpt, const float eta, const float phi);

  double invpt() const {return m_invpt;}
  float eta() const {return m_eta;}
  float phi() const {return m_phi;}

 protected:
  double m_invpt{std::numeric_limits<double>::lowest()};
  float m_eta{0.};
  float m_phi{0.};
};

inline void L0MuonTrack::setTrack(const double invpt, const float eta, const float phi) {
  m_invpt = invpt;
  m_eta = eta;
  m_phi = phi;
}

}

inline std::ostream& operator << (std::ostream& s, const L1Muon::L0MuonTrack& t) {
  s << "L0MuonTrack Candidate:  q/pt=" << t.invpt() * 1000.<< " (1/GeV) eta=" << t.eta() << " phi=" << t.phi() << "\n";
  return s;
}


#endif   // L1MUONEMULATION_L0MUONTRACK_H

