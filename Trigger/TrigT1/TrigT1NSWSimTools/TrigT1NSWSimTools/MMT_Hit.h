/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MMT_HIT_H
#define MMT_HIT_H

#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include <cmath>

class MMT_Hit {
  public:
    MMT_Hit(const Identifier &id, const std::string& stationName,
            const int stEta, const int stPhi, const int sectorPhi,
            const int multiplet, const int gasGap, const int channel,
            const float stripTime, const int BC,
            const MuonGM::MuonDetectorManager* detManager);

    ~MMT_Hit() = default;

    // Getters
    int getART() const { return m_ART_ASIC; }
    int getAge() const { return m_age; }
    int getBC() const { return m_BC_time; }
    int getChannel() const { return m_strip; }
    int getPlane() const { return m_plane; }
    char getSector() const { return m_sector; }
    double getRZSlope() const { return m_RZslope; }
    int getVMM() const { return m_VMM_chip; }
    double getShift() const { return m_shift; }
    int getStationEta() const { return m_station_eta; }
    int getStationPhi() const { return m_station_phi; }
    int getSectorPhi() const { return m_sector_phi; }
    double getR() const { return m_R; }
    double getRp() const { return m_Rp; }
    double getZ() const { return m_Z; }
    double getPitchOverZ() const { return m_PitchOverZ; }
    float getTime() const { return m_time; }
    bool isNoise() const { return m_isNoise; }
    bool isX() const { return m_isX; }
    bool isU() const { return m_isU; }
    bool isV() const { return m_isV; }
    bool infSlope() const { return std::isinf(m_RZslope); }

    // Setters
    void setAge(int age) { m_age = age; }
    void setAsNoise() { m_isNoise = true; }

  private:
    double m_RZslope{-1}, m_Rp{-1};
    double m_Z{-1}, m_R{-1};
    double m_PitchOverZ{-1}, m_shift{-1};
    float m_time;
    int m_VMM_chip;
    int m_ART_ASIC;
    int m_plane;
    int m_station_eta;
    int m_station_phi;
    int m_sector_phi;
    int m_strip;
    int m_BC_time;
    int m_age;
    char m_sector;
    bool m_isNoise{false};
    bool m_isX{false};
    bool m_isU{false};
    bool m_isV{false};
};
#endif
