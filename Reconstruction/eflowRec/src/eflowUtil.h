/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * eflowUtil.h
 *
 *  Created on: 09.08.2013
 *      Author: tlodd
 */

#ifndef EFLOWREC_EFLOWUTIL_H
#define EFLOWREC_EFLOWUTIL_H

#include <cmath>
#include <string>

/**
 eflowAzimuth represents phi and has kinematic functions which correctly deal with phi wraparound etc. 
*/
class eflowAzimuth {
public:
eflowAzimuth(double phi): m_value(phi) { if (phi != -999. && !std::isnan(phi)) adjustRange(); }
  eflowAzimuth(const eflowAzimuth& other): m_value(other.m_value) { }
  eflowAzimuth& operator=(const eflowAzimuth& other) { if (this == &other) return *this; else { m_value = other.m_value; return *this; } }
  ~eflowAzimuth() { }

  inline double operator ()() const { return m_value; }
  inline double operator =(double phi) {
    m_value = phi;
    adjustRange();
    return m_value;
  }

  inline eflowAzimuth operator +=(double deltaPhi) {
    m_value += deltaPhi;
    adjustRange();
    return *this;
  }
  inline eflowAzimuth operator -=(double deltaPhi) {
    m_value -= deltaPhi;
    adjustRange();
    return *this;
  }

  inline double getAbsDifference(const eflowAzimuth& other) const {
    double plainAbsDifference = std::abs(m_value - other.m_value);
    return plainAbsDifference <= M_PI ? plainAbsDifference : 2*M_PI - plainAbsDifference;
  }

  inline double cycle(const eflowAzimuth& other) { return cycle(other.m_value); }
  inline double cycle(double phi) {
    double plainDifference = phi-m_value;
    if (plainDifference > M_PI) {
      return m_value+2.0*M_PI;
    } else if (plainDifference < -M_PI) {
      return m_value-2.0*M_PI;
    } else {
      return m_value;
    }
  }

private:
  double m_value;

  inline double adjustRange(double a) {
    if (a <= -M_PI) {
      return a+(2*M_PI*std::floor(-(a-M_PI)/(2*M_PI)));
    } else if (a > M_PI) {
      return a-(2*M_PI*std::floor((a+M_PI)/(2*M_PI)));
    } else {
      return a;
    }
  }
  inline void adjustRange() {
    if (m_value <= -M_PI) {
      m_value+=(2*M_PI*std::floor(-(m_value-M_PI)/(2*M_PI)));
    } else if (m_value > M_PI) {
      m_value-=(2*M_PI*std::floor((m_value+M_PI)/(2*M_PI)));
    }
  }

};

class eflowEtaPhiPosition {
public:
  eflowEtaPhiPosition() = default;
  eflowEtaPhiPosition(double eta, double phi): m_eta(eta), m_phi(phi) {}
 
  inline double getEta() const { return m_eta; }
  inline eflowAzimuth getPhi() const { return m_phi; }
  inline double getPhiD() const { return m_phi(); }

  inline double dRSq(const eflowEtaPhiPosition& other) const {
    double dEta(m_eta-other.m_eta);
    double dPhi(m_phi.getAbsDifference(other.m_phi));
    return dEta*dEta + dPhi*dPhi;
  }
  inline double dR(const eflowEtaPhiPosition& other) const { return std::sqrt(this->dRSq(other)); }

private:
  double m_eta{NAN};
  eflowAzimuth m_phi{NAN};
};

/**
eflowRangeBase is an object to represent a length in eta or phi, and this is used in eflowCellIntegrator as a variable in the integrations over eta and phi.
*/
template <class T>
class eflowRangeBase{
public:
  eflowRangeBase() = default;
  eflowRangeBase(const T& min, const T& max): m_min(min), m_max(max) { }

  inline void setCenterAndWidth(T center, double width) { m_min = center - width/2; m_max = m_min + width; }
  inline void shift(double shift) { m_min += shift; m_max += shift; }

  inline T getMax() const { return m_max; }
  inline T getMin() const { return m_min; }

  inline T getCenter() const { return (m_max + m_min)/2; }
  inline T getWidth() const { return (m_max - m_min); }

  bool contains(const T& x) { return ( (m_min < x) && (m_max > x) ); }

  std::string print() const {
    std::string result = "[";
    result += std::to_string(m_min);
    result += ", ";
    result += std::to_string(m_max);
    result += ']';
    return result;
  }

private:
  T m_min{NAN};
  T m_max{NAN};
};
typedef eflowRangeBase<double> eflowRange;

#endif /* EFLOWREC_EFLOWUTIL_H */
