/*
 *   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MMT_HIT_H
#define MMT_HIT_H

#include "MMT_struct.h"

namespace MuonGM {
  class MuonDetectorManager;
}

class MMT_Hit {
  public:
    MMT_Hit(const hitData_entry &entry, const MuonGM::MuonDetectorManager* detManager, const std::shared_ptr<MMT_Parameters> par, const std::vector<ROOT::Math::XYZVector> &planeCoordinates);
    MMT_Hit(const MMT_Hit* hit);
    ~MMT_Hit()=default;

    int getART() const { return m_ART_ASIC; }
    int getAge() const { return m_age; }
    int getBC() const { return m_BC_time; }
    int getChannel() const { return m_strip; }
    int getGasGap() const { return m_gasgap; }
    const std::string& getModule() const { return m_module; }
    int getMultiplet() const { return m_multiplet; }
    int getPlane() const { return m_plane; }
    char getSector() const { return m_sector; }
    double getRZSlope() const { return m_RZslope; }
    double getYZSlope() const { return m_YZslope; }
    int getVMM() const { return m_VMM_chip; }
    int getMMFE8() const { return m_MMFE_VMM; }
    float getShift() const { return m_shift; }
    const std::string& getStationName() const { return m_station_name; }
    int getStationEta() const { return m_station_eta; }
    int getStationPhi() const { return m_station_phi; }
    double getR() const { return m_R; }
    double getRp() const { return m_Rp; }
    double getX() const { return m_localX; }
    double getY() const { return m_Y; }
    double getZ() const { return m_Z; }
    double getPitchOverZ() const { return m_PitchOverZ; }
    float getTime() const { return m_time; }
    bool isNoise() const { return m_isNoise; }
    bool isX() const;
    bool isU() const;
    bool isV() const;
    void printHit() const;
    void setAge(int age) { m_age = age; }
    void setAsNoise() { m_isNoise = true; }
    void setBC(int bc) { m_BC_time = bc; }
    void setRZSlope(double slope) { m_RZslope = slope; }
    void setYZSlope(double slope) { m_YZslope = slope; }
    void setY(double y) { m_Y = y; }
    void setZ(double z) { m_Z = z; }
    bool verifyHit() const;

  private:
    char m_sector;
    std::string m_module, m_station_name;
    int m_VMM_chip;
    int m_MMFE_VMM;
    int m_ART_ASIC;
    int m_plane;
    int m_station_eta;
    int m_station_phi;
    int m_multiplet;
    int m_gasgap;
    int m_strip;
    double m_localX;
    double m_RZslope, m_YZslope;
    int m_BC_time, m_age;
    double m_Y, m_Z, m_PitchOverZ;
    double m_R, m_Rp;
    bool m_isNoise;
    float m_time, m_shift;
};
#endif
