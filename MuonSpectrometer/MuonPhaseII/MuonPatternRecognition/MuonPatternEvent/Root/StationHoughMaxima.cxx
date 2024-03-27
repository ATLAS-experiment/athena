/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternEvent/StationHoughMaxima.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
namespace MuonR4 {
    StationHoughMaxima::StationHoughMaxima(const MuonGMR4::MuonChamber* chamber, 
                                               const std::vector<HoughMaximum> & maxima):
        m_chamber{chamber},
        m_maxima(maxima){
    }
    void StationHoughMaxima::addMaximum(const HoughMaximum & m){
        m_maxima.push_back(m);
    }
    const MuonGMR4::MuonChamber* StationHoughMaxima::chamber() const {
        return m_chamber;
    }
    const std::vector<MuonR4::HoughMaximum> & MuonR4::StationHoughMaxima::getMaxima() const{
        return m_maxima;
    }
    bool MuonR4::StationHoughMaxima::operator< (const StationHoughMaxima & other) const{
        using ChamberSorter = MuonGMR4::MuonDetectorManager::ChamberSorter;
        return ChamberSorter{}(m_chamber, other.m_chamber); 
    }
}
