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
    MMT_Hit(const Identifier &id, const std::string_view stName, const int stEta, const int stPhi, const int sectorPhi, const int multiplet, const int gasGap, const int channel, const float stripTime, const int BC, const MuonGM::MuonDetectorManager* detManager);
    MMT_Hit(const MMT_Hit* hit);
    ~MMT_Hit()=default;

    int getART() const { return m_ART_ASIC; }
    int getAge() const { return m_age; }
    int getBC() const { return m_BC_time; }
    int getChannel() const { return m_strip; }
    int getGasGap() const { return m_gasgap; }
    int getMultiplet() const { return m_multiplet; }
    int getPlane() const { return m_plane; }
    char getSector() const { return m_sector; }
    double getRZSlope() const { return m_RZslope; }
    int getVMM() const { return m_VMM_chip; }
    int getMMFE8() const { return m_MMFE_VMM; }
    float getShift() const { return m_shift; }
    std::string getStationName() const { return m_station_name; }
    int getStationEta() const { return m_station_eta; }
    int getStationPhi() const { return m_station_phi; }
    int getSectorPhi() const { return m_sector_phi; }
    double getR() const { return m_R; }
    double getRp() const { return m_Rp; }
    double getZ() const { return m_Z; }
    double getPitchOverZ() const { return m_PitchOverZ; }
    float getTime() const { return m_time; }
    bool isNoise() const { return m_isNoise; }
    bool isX() const;
    bool isU() const;
    bool isV() const;
    void setAge(int age) { m_age = age; }
    void setAsNoise() { m_isNoise = true; }
    void setBC(int bc) { m_BC_time = bc; }
    void setRZSlope(double slope) { m_RZslope = slope; }
    void setZ(double z) { m_Z = z; }
    bool infSlope() const;

  private:
    char m_sector;
    std::string m_station_name;
    int m_VMM_chip;
    int m_MMFE_VMM;
    int m_ART_ASIC;
    int m_plane;
    int m_station_eta;
    int m_station_phi;
    int m_sector_phi;
    int m_multiplet;
    int m_gasgap;
    int m_strip;
    double m_RZslope;
    int m_BC_time, m_age;
    double m_Z, m_PitchOverZ;
    double m_R, m_Rp;
    bool m_isNoise;
    float m_time, m_shift;
};
#endif
