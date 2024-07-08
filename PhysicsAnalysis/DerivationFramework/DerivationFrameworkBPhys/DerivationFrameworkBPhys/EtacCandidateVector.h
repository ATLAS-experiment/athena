/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Xin Chen <xin.chen@cern.ch>
*/
#ifndef ETACCANDIDATEVECTOR_H
#define ETACCANDIDATEVECTOR_H

#include "xAODTracking/TrackParticle.h"
#include <vector>

namespace DerivationFramework {

  struct RhoCandidate {
    const xAOD::TrackParticle* trackParticle1 = nullptr;
    const xAOD::TrackParticle* trackParticle2 = nullptr;
    double mass_hypo1;
    double mass_hypo2;
    double mass_hypo3;
    double ptTot;
    double chi2NDF;
    Amg::Vector3D vtxPos;
  };

  struct EtacCandidate {
    const xAOD::TrackParticle* trackParticle1 = nullptr;
    const xAOD::TrackParticle* trackParticle2 = nullptr;
    const xAOD::TrackParticle* trackParticle3 = nullptr;
    const xAOD::TrackParticle* trackParticle4 = nullptr;
    const xAOD::TrackParticle* trackParticle5 = nullptr;
    const xAOD::TrackParticle* trackParticle6 = nullptr;
    int nTracks = 0;
    double ptTot;
    double chi2NDFSum;
    Amg::Vector3D vtxPos1;
    Amg::Vector3D vtxPos2;
    Amg::Vector3D vtxPos3;
  };

  class RhoCandidateVector {
  public:
    RhoCandidateVector(size_t num, bool orderByPt);
    void AddElement(const RhoCandidate& rho);
    const std::vector<RhoCandidate>& GetVector() const;

  private:
    size_t m_num;
    bool m_orderByPt;
    std::vector<RhoCandidate> m_vector;
  };

  class EtacCandidateVector {
  public:
    EtacCandidateVector(size_t num, bool orderByPt);
    void AddElement(const EtacCandidate& etac);
    const std::vector<EtacCandidate>& GetVector() const;

  private:
    size_t m_num;
    bool m_orderByPt;
    std::vector<EtacCandidate> m_vector;
  };
}

#endif
