/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOSIMEVENTTPCNV_SRCALOCALIBRATIONHITCONTAINER_P1_H
#define CALOSIMEVENTTPCNV_SRCALOCALIBRATIONHITCONTAINER_P1_H
/**
@class SrCaloCalibrationHitContainer_P1
@brief Persistent represenation of a SrCaloCalibrationContainer,
@author: Frédéric DEJEAN
*/
#include <string>
#include <vector>

class SrCaloCalibrationHitContainer_p1 {
 public:
  /// Default constructor
  SrCaloCalibrationHitContainer_p1();

  // Accessors
  const std::string& name() const;

  std::vector<unsigned long long> m_channelHash;
  std::vector<float> m_energy;
  std::string m_name;
  std::vector<unsigned int> m_particleID;
};

// inlines

inline SrCaloCalibrationHitContainer_p1::SrCaloCalibrationHitContainer_p1() {}

inline const std::string& SrCaloCalibrationHitContainer_p1::name() const {
  return m_name;
}

#endif
