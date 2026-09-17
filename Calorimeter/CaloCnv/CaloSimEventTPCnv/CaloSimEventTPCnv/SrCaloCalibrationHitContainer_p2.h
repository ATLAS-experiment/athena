/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOSIMEVENTTPCNV_SRCALOCALIBRATIONHITCONTAINER_P2_H
#define CALOSIMEVENTTPCNV_SRCALOCALIBRATIONHITCONTAINER_P2_H
/**
@class SrCaloCalibrationHitContainer_P2
@brief Persistent represenation of a SrCaloCalibrationContainer,
@author: Frédéric DEJEAN
*/
#include <string>
#include <vector>

class SrCaloCalibrationHitContainer_p2 {
 public:
  /// Default constructor
  SrCaloCalibrationHitContainer_p2();

  // Accessors
  const std::string& name() const;

  std::vector<unsigned long long> m_channelHash;
  std::vector<float> m_energy;
  std::string m_name;
  std::vector<unsigned int> m_particleUID;
};

// inlines

inline SrCaloCalibrationHitContainer_p2::SrCaloCalibrationHitContainer_p2() {}

inline const std::string& SrCaloCalibrationHitContainer_p2::name() const {
  return m_name;
}

#endif
