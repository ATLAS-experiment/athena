/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef L1MuonMDTTools_L0MDTSEGMENT_H
#define L1MuonMDTTools_L0MDTSEGMENT_H

namespace L0MDT {

/**
 * @class Segment
 * @brief Class describing a reconstructed MDT segment used by the L0Muon trigger.
 *
 * The class stores the fitted segment parameters in the global (z, R) plane,
 * together with representative quantities derived from the hits used in the fit.
 */
class Segment {
public:
  Segment() = default;
  Segment(float m, float b) : m_m(m), m_b(b) {}
  ~Segment() = default;

  // Setters
  void setM(float x) { m_m = x; }
  void setB(float x) { m_b = x; }
  void setChi2(float x) { m_chi2 = x; }
  void setNHits(unsigned int x) { m_nHits = x; }

  void setZMin(float x) { m_zMin = x; }
  void setZMax(float x) { m_zMax = x; }
  void setZRef(float x) { m_zRef = x; }
  void setRRef(float x) { m_rRef = x; }

  // Getters
  float m() const { return m_m; }
  float b() const { return m_b; }
  float chi2() const { return m_chi2; }
  unsigned int nHits() const { return m_nHits; }

  float zMin() const { return m_zMin; }
  float zMax() const { return m_zMax; }
  float zRef() const { return m_zRef; }
  float rRef() const { return m_rRef; }

private:
  float m_m{0.f};
  float m_b{0.f};
  float m_chi2{0.f};
  unsigned int m_nHits{0};

  float m_zMin{0.f};
  float m_zMax{0.f};
  float m_zRef{0.f};
  float m_rRef{0.f};
};

} // end of namespace L0MDT

#endif
