/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigT1NSWSimTools/MMT_Hit.h"

MMT_Hit::MMT_Hit(const Identifier &id, const std::string_view stName, const int stEta, const int stPhi, const int sectorPhi, const int multiplet, const int gasGap, const int channel, const float stripTime, const int BC, const MuonGM::MuonDetectorManager* detManager) {
  m_sector = stName[2];
  m_station_name = stName;
  m_station_eta = stEta;
  m_station_phi = stPhi;
  m_sector_phi = sectorPhi;
  m_multiplet = multiplet;
  m_gasgap = gasGap;
  m_plane = (multiplet-1)*4 + gasGap-1;
  m_strip = channel;
  m_BC_time = BC;
  m_age = BC;
  m_Z = -1.;
  m_R = -1.;
  m_Rp = -1.;
  m_isNoise = false;
  m_time = stripTime;
  m_RZslope = -1.;
  m_PitchOverZ = -1.;
  m_shift = -1.;

  int istrip = (std::abs(m_station_eta)-1) * (64*8*10) + m_strip; //here needed the absolute index of the strip on the sector layer (m_strip is only up to 5119)

  // region represent the index of the mmfe8 in the plane
  int region = int(float(istrip)/(64*8));
  // map of mmfe8s layer,radius(MMFE8 index on sector)
  unsigned int mmfe8s[8][16];
  // loop on layers
  for( unsigned int L=0; L<8; L++){
    // loop on pcbs
    for(unsigned int p=1; p<9; p++){
      // loop on sides
      for(unsigned int s=0; s<2; s++){ //loop on 0 (Left) and 1 (Right), same convention used also later
        unsigned int R = (L%2==s) ? (p-1)*2 : (p-1)*2+1;
        mmfe8s[L][R]=s;
      }
    }
  }

  m_MMFE_VMM = region; // index of the MMFE8 board on the layer
  m_VMM_chip = int(1. *istrip /64.); // index of the VMM chip on the layer
  // art asic id
  if(!int(m_plane/2.)%2){
    if (mmfe8s[m_plane][region]==1){ //Right
      m_ART_ASIC = 1-int(region/8);
    }else{
      m_ART_ASIC = int(region/8);
    }
  }else{
    if (mmfe8s[m_plane][region]==0){ //Left
      m_ART_ASIC = 1-int(region/8);
    }else{
      m_ART_ASIC = int(region/8);
    }
  }
 
  // if Right side add 2 to the ART Asic Index
  if(mmfe8s[m_plane][region]==0){
    m_ART_ASIC+=2;
  }

  const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(id);
  Amg::Vector3D globalPos(0.0, 0.0, 0.0);
  if(readout->stripGlobalPosition(id, globalPos)) {
    m_R = globalPos.perp();
    m_Z = globalPos.z();
    m_PitchOverZ = (readout->getDesign(id))->inputPitch/m_Z;
    m_RZslope = m_R / m_Z;
    const double distanceFromZAxis = readout->absTransform().translation().perp() - 0.5*readout->getRsize();

    Identifier tmpId = detManager->mmIdHelper()->channelID(m_station_name, 1, 1, 1, 1, 1);
    const MuonGM::MMReadoutElement* roEl = detManager->getMMReadoutElement(tmpId);
    int tmpStrip = (roEl->getDesign(tmpId))->nMissedBottomEta + 1;
    tmpId = detManager->mmIdHelper()->channelID(m_station_name, 1, 1, 1, 1, tmpStrip);
    globalPos = Amg::Vector3D::Zero();
    if(roEl->stripGlobalPosition(tmpId, globalPos)) {
      double index = std::round((std::abs(m_RZslope)-0.1)/5e-04); // 0.0005 is approx. the step in slope achievable with a road size of 8 strips
      m_Rp = distanceFromZAxis + (0.1 + index*((0.6 - 0.1)/1000.))*(std::abs(m_Z) - globalPos.z());
      m_shift = m_Rp / m_Z;
    }
  }
}

MMT_Hit::MMT_Hit(const MMT_Hit* hit)
  : m_sector (hit->m_sector),
    m_station_name (hit->m_station_name),
    m_VMM_chip (hit->m_VMM_chip),
    m_MMFE_VMM (hit->m_MMFE_VMM),
    m_ART_ASIC (hit->m_ART_ASIC),
    m_plane (hit->m_plane),
    m_station_eta (hit->m_station_eta),
    m_station_phi (hit->m_station_phi),
    m_sector_phi (hit->m_sector_phi),
    m_multiplet (hit->m_multiplet),
    m_gasgap (hit->m_gasgap),
    m_strip (hit->m_strip),
    m_RZslope (hit->m_RZslope),
    m_BC_time (hit->m_BC_time),
    m_age (hit->m_age),
    m_Z (hit->m_Z),
    m_PitchOverZ (hit->m_PitchOverZ),
    m_R (hit->m_R),
    m_Rp (hit->m_Rp),
    m_isNoise (hit->m_isNoise),
    m_time (hit->m_time),
    m_shift (hit->m_shift)
{
}

bool MMT_Hit::isX() const {
  return (m_plane == 0 || m_plane == 1 || m_plane == 6 || m_plane == 7);
}

bool MMT_Hit::isU() const {
  return (m_plane == 2 || m_plane == 4);
}

bool MMT_Hit::isV() const {
  return (m_plane == 3 || m_plane == 5);
}

bool MMT_Hit::infSlope() const {
  return std::isinf(m_RZslope);
}
