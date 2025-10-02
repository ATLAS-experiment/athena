/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkPixSimulationParameters.h
 * @author Shaun Roe
 * @date April, 2025
 * @brief Simple data class for holding constants for use in the ITkPixV2 Chip simulation
 */
 
#ifndef ITkPixSimulationParameters_h
#define ITkPixSimulationParameters_h

#include <vector>
#include <iosfwd>
 
class ITkPixSimulationParameters{
  public:
    //As more information is obtained, these methods may take arguments indicating the
    //channel number.
    //As they grow in complexity, the implementation should be moved to the .cxx file.
    int totThreshold() const { return m_totThreshold;}
    double crossTalk() const { return m_crossTalk;}
    double disableProbability() const {return m_disableProbability;}
    double noiseOccupancy() const { return m_noiseOccupancy;}
    const std::vector<float> & noiseShape() const{ return m_noiseShape;}
  private:
    //As more information is obtained, the single numbers may become vectors or arrays.
    std::vector<float> m_noiseShape{0.f,1.f};
    double m_disableProbability{9e-3};
    double m_noiseOccupancy{5e-8};
    double m_crossTalk{0.06};
    int m_totThreshold{-1};
};

//simple output of the parameters for debugging
std::ostream & operator<<(std::ostream & os, const ITkPixSimulationParameters & chipParam);
//no CLASS_DEF or CONDCONT_DEF until the DB is used to retrieve these numbers
#endif
