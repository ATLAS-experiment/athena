/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOSIMEVENTTPCNV_CALOCALIBRATIONHITCONTAINER_P4_H
#define CALOSIMEVENTTPCNV_CALOCALIBRATIONHITCONTAINER_P4_H
/**
   @class CaloCalibrationHitContainer_p4
   @brief Persistent represenation of a CaloCalibrationContainer,
   @author: Ilija Vukotic
*/
#include <vector>
#include <string>

class CaloCalibrationHitContainer_p4
{
public:

  /// Default constructor
  CaloCalibrationHitContainer_p4();

  // Accessors
  const std::string&  name() const;

  std::vector<unsigned int> m_channelHash;
  std::vector<unsigned int> m_energy; // 18 bits compressed - all four of them in the same array
  std::string m_name;
  std::vector<unsigned int> m_particleUID;

};

// inlines

inline CaloCalibrationHitContainer_p4::CaloCalibrationHitContainer_p4() {}

inline const std::string& CaloCalibrationHitContainer_p4::name() const {return m_name;}

#endif
