/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternEvent/HoughMaximum.h"

MuonR4::HoughMaximum::HoughMaximum(double x, double y,  double counts,  std::vector<HoughHitType > && hits):
    m_x(x),
    m_y(y),
    m_counts(counts),
    m_hitsInMax(hits){

};
double MuonR4::HoughMaximum::getX() const{
    return m_x;
}

double MuonR4::HoughMaximum::getY() const{
    return m_y;
}
double MuonR4::HoughMaximum::getCounts() const{
    return m_counts;
}
const std::vector<MuonR4::HoughHitType> & MuonR4::HoughMaximum::getHitsInMax() const{
    return m_hitsInMax; 
}
