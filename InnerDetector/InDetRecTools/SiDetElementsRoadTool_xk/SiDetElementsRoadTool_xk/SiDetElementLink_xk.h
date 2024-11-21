/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiDetElementLink_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for detector elements links
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiDetElementLink_xk_H
#define SiDetElementLink_xk_H

#include <utility>

#include "InDetReadoutGeometry/SiDetectorElement.h"

namespace InDet {

class SiDetElementLink_xk {
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

 public:
  class ElementWay {
   public:
    ElementWay(const InDet::SiDetElementLink_xk* link, float way,
               float distance)
        : m_link(link), m_way(way), m_distance(distance) {}

    const InDet::SiDetElementLink_xk* link() const { return m_link; }
    float way() const { return m_way; }
    float distance() const { return m_distance; }

   private:
    const InDet::SiDetElementLink_xk* m_link;
    float m_way;
    float m_distance;
  };

  SiDetElementLink_xk() = default;
  SiDetElementLink_xk(const InDetDD::SiDetectorElement*, const double*,
                      bool isITk = false);
  SiDetElementLink_xk(const SiDetElementLink_xk&) = default;
  SiDetElementLink_xk& operator=(const SiDetElementLink_xk&) = default;
  SiDetElementLink_xk(SiDetElementLink_xk&&) = default;
  SiDetElementLink_xk& operator=(SiDetElementLink_xk&&) = default;
  ~SiDetElementLink_xk() = default;
  ///////////////////////////////////////////////////////////////////
  // Main methods
  ///////////////////////////////////////////////////////////////////

  void set(const double*, bool isITk = false);
  const InDetDD::SiDetectorElement* detElement() const { return m_detelement; }
  float phi() const { return m_phi; }
  float z() const { return m_z; }
  float dz() const { return m_dz; }
  void intersect(const float*, const float*, float*) const;
  bool intersectITk(const float*, const float*, float&) const;

 private:
  const InDetDD::SiDetectorElement* m_detelement = nullptr;  // note owning ptr
  float m_phi = 0;
  float m_z = 0;
  float m_dz = 0;
  float m_geo[6] = {};
  float m_center[2] = {};
  float m_bound[4][3] = {};
};

/////////////////////////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////////////////////////

inline InDet::SiDetElementLink_xk::SiDetElementLink_xk(
    const InDetDD::SiDetectorElement* el, const double* P, bool isITk) {
  m_detelement = el;
  set(P, isITk);
}

}  // namespace InDet

#endif  // SiDetElementLink_xk

