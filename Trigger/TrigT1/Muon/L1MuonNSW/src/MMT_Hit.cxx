/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "MMT_Hit.h"

#include <array>
#include <string>

namespace L1Muon::L1MM {

MMT_Hit::MMT_Hit(const Identifier& id, const float stripTime, const int BC,
                 const MuonGMR4::MuonDetectorManager* detManager,
                 const ActsTrk::GeometryContext& gctx)
  : m_time(stripTime),
    m_BC_time(BC), m_age(BC)
 {
  const Muon::IMuonIdHelperSvc* idHelperSvc = detManager->idHelperSvc();
  const MmIdHelper& idHelper = idHelperSvc->mmIdHelper();
  const std::string stationName = idHelperSvc->chamberNameString(id);
  m_station_eta = idHelperSvc->stationEta(id);
  m_station_phi = idHelperSvc->stationPhi(id);
  m_sector_phi = idHelperSvc->sector(id);
  m_strip = idHelper.channel(id);
  m_sector = stationName[2];  // 'L' for MML, 'S' for MMS
  m_plane = (idHelper.multilayer(id)-1)*4 + idHelper.gasGap(id)-1;

  const MuonGMR4::MmReadoutElement* readout = detManager->getMmReadoutElement(id);
  const MuonGMR4::StripDesign& design = readout->stripLayer(id).design();

  // Layer type from the strip design: X layers have no stereo angle, U layers +1.5 deg, V layers -1.5 deg
  if (!design.hasStereoAngle()) m_type = LayerType::X;
  else m_type = (design.stereoAngle() > 0.) ? LayerType::U : LayerType::V;

  // Strip index on the sector layer (the channel restarts at each module, up to 5120 for |stationEta| = 1)
  m_globalStrip = (std::abs(m_station_eta)-1) * kStripsEta1 + m_strip;

  // region represent the index of the MMFE8 board in the plane
  const int region = m_globalStrip / kStripsPerMMFE8;

  // map of mmfe8s layer,radius(MMFE8 index on sector)
  std::array<std::array<unsigned int, kMMFE8sPerLayer>, kPlanes> mmfe8s{};
  // loop on layers
  for( unsigned int L=0; L<kPlanes; L++){
    // loop on pcbs
    for(unsigned int p=1; p<=kPCBsPerLayer; p++){
      // loop on sides
      for(unsigned int s=0; s<2; s++){ //loop on 0 (Left) and 1 (Right), same convention used also later
        unsigned int R = (L%2==s) ? (p-1)*2 : (p-1)*2+1;
        mmfe8s[L][R]=s;
      }
    }
  }

  m_VMM_chip = m_globalStrip / kStripsPerVMM; // index of the VMM chip on the layer
  // art asic id: radial half of the layer (0 inner, 1 outer), swapped on the second gas-gap pair
  const int half = region / kMMFE8sPerHalfLayer;
  if(!((m_plane/2)%2)){
    if (mmfe8s[m_plane][region]==1){ //Right
      m_ART_ASIC = 1-half;
    }else{
      m_ART_ASIC = half;
    }
  }else{
    if (mmfe8s[m_plane][region]==0){ //Left
      m_ART_ASIC = 1-half;
    }else{
      m_ART_ASIC = half;
    }
  }

  // if Left side add 2 to the ART Asic Index
  if(mmfe8s[m_plane][region]==0){
    m_ART_ASIC+=2;
  }

  // Geometry: global position of the strip centre, with the alignment of the geometry context
  const Amg::Vector3D globalPos = readout->stripPosition(gctx, id);
  m_R = globalPos.perp();
  m_Z = globalPos.z();
  m_PitchOverZ = design.stripPitch()/m_Z;
  m_RZslope = m_R / m_Z;

  // Reference: the eta=1 module of this sector type (MML/MMS), phi=1, side A, multilayer 1, gas gap 1
  const Identifier refId = idHelper.channelID(stationName, 1, 1, 1, 1, 1);
  const MuonGMR4::MmReadoutElement* roEl = detManager->getMmReadoutElement(refId);
  const double distanceFromZAxis = roEl->center(gctx).perp() - 0.5*roEl->moduleHeight();
  const int firstActiveStrip = roEl->firstStrip(roEl->layerHash(refId));
  const Identifier refStripId = idHelper.channelID(stationName, 1, 1, 1, 1, firstActiveStrip);
  const Amg::Vector3D refPos = roEl->stripPosition(gctx, refStripId);

  m_refR0 = distanceFromZAxis;
  m_refZ0 = refPos.z();

  double index = std::round((std::abs(m_RZslope)-0.1)/5e-04); // 0.0005 is approx. the step in slope achievable with a road size of 8 strips
  m_Rp = distanceFromZAxis + (0.1 + index*((0.6 - 0.1)/1000.))*(std::abs(m_Z) - refPos.z());
  m_shift = m_Rp / m_Z;
}

}  // namespace L1Muon::L1MM
