/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigT1NSWSimTools/MMT_Hit.h"

MMT_Hit::MMT_Hit(const Identifier &id, const std::string& stationName,
                 const int stEta, const int stPhi, const int sectorPhi,
                 const int multiplet, const int gasGap, const int channel,
                 const float stripTime, const int BC,
                 const MuonGM::MuonDetectorManager* detManager)
  : m_time(stripTime),
    m_station_eta(stEta), m_station_phi(stPhi), m_sector_phi(sectorPhi),
    m_strip(channel),
    m_BC_time(BC), m_age(BC),
    m_sector(stationName[2])
 {
  m_plane = (multiplet-1)*4 + gasGap-1;

  switch (m_plane) {
  case 0:
  case 1:
  case 6:
  case 7:
    m_isX = true;
    break;
  case 2:
  case 4:
    m_isU = true;
    break;
  case 3:
  case 5:
    m_isV = true;
    break;
  }

  // here needed the absolute index of the strip on the sector layer (m_strip is only up to 5119)
  const int istrip = (std::abs(m_station_eta)-1) * (64*8*10) + m_strip;

  // region represent the index of the MMFE8 board in the plane
  const int region = int(float(istrip)/(64*8));

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

  m_VMM_chip = int(1. *istrip /64.); // index of the VMM chip on the layer
  // art asic id
  if(!(int(m_plane/2.)%2)){
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
 
  // if Left side add 2 to the ART Asic Index
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

    Identifier tmpId = detManager->mmIdHelper()->channelID(stationName, 1, 1, 1, 1, 1);
    const MuonGM::MMReadoutElement* roEl = detManager->getMMReadoutElement(tmpId);
    int tmpStrip = (roEl->getDesign(tmpId))->nMissedBottomEta + 1;
    tmpId = detManager->mmIdHelper()->channelID(stationName, 1, 1, 1, 1, tmpStrip);
    globalPos = Amg::Vector3D::Zero();
    if(roEl->stripGlobalPosition(tmpId, globalPos)) {
      double index = std::round((std::abs(m_RZslope)-0.1)/5e-04); // 0.0005 is approx. the step in slope achievable with a road size of 8 strips
      m_Rp = distanceFromZAxis + (0.1 + index*((0.6 - 0.1)/1000.))*(std::abs(m_Z) - globalPos.z());
      m_shift = m_Rp / m_Z;
    }
  }
}
