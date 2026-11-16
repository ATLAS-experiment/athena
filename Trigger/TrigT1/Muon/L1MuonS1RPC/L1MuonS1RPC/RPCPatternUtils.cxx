/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "L1MuonS1RPC/RPCPatternUtils.h"
#include "L1MuonS1RPC/RPCSLStripConverter.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "GaudiKernel/MsgStream.h"

uint32_t L1Muon::RPCPatternUtils::getSLStripId(bool side, int sector, int doubletPhi, Muon::MuonStationIndex::StIndex RPClayer, int doubletR, int stationEta, int doubletZ, int stripNumber, bool measuresPhi) {
    return L1Muon::RPCSLStripConverter::pack( side, static_cast<uint8_t>(sector-1), doubletPhi-1, 
                                RPClayer, static_cast<uint8_t>(doubletR-1), 
                                static_cast<uint8_t>(std::abs(stationEta)),
                                static_cast<uint8_t>(doubletZ-1), 
                                static_cast<uint8_t>(stripNumber-1), measuresPhi);
}

int L1Muon::RPCPatternUtils::getVirtualBIPhiStripNumber(float loc_pos_along_BIstrip, const MuonGMR4::RpcReadoutElement& readoutElement) {
    float halfSectorWidth = readoutElement.getParameters().halfWidth; // half of the sector width in local coordinates
    int nMax_strip = static_cast<int>(2 * halfSectorWidth / s_digitalBiPhiStripWidth); // number of phi strips in the half-sector

    // Since loc_pos_along_BIstrip ranges from 0 to ± halfSectorWidth, we map it to strip numbers accordingly:
    // phiStrip == 1 at loc_pos_along_BIstrip == -halfSectorWidth
    // phiStrip == nMax_strip at loc_pos_along_BIstrip == halfSectorWidth
    // The formula below maps the local position to the corresponding strip number
    int phiStrip = static_cast<int>(1 + std::round(((loc_pos_along_BIstrip + halfSectorWidth) / (2 * halfSectorWidth)) * (nMax_strip - 1)));
    return std::clamp(phiStrip, 1, nMax_strip);
}

uint32_t L1Muon::RPCPatternUtils::getVirtualBIPhiStripCoordinate(const Amg::Vector3D& stripPos, float loc_pos_along_BIstrip) {
    // Just trigonometry to get the phi coordinate
    // The center of the chamber is the BI eta strip at (stripPos.x(), stripPos.y()) --> forms an angle phi_center = atan2(y, x)
    // The local position along the strip gives a shift in phi: dphi = atan(loc_pos_along_BIstrip / R)
    // The final phi coordinate is then: phi = phi_center - dphi
    // We subtract dphi because the local position is defined positive towards decreasing phi (check with geometry)
    // Since we want phi in [0, 2*pi], we add 2*pi if the result is negative.
    float phi_center = stripPos.phi() + M_PI; // convert to [0, 2*pi] range
    float R =stripPos.perp();
    float dPhi = std::atan(loc_pos_along_BIstrip / R);
    float phi = phi_center - dPhi;
    if (phi < 0) {
      phi += 2.0f * static_cast<float>(M_PI);
    }
    if (phi > 2.0f * static_cast<float>(M_PI)) {
      phi -= 2.0f * static_cast<float>(M_PI);
    }

    float normPhi = std::clamp(phi / (2.0f * static_cast<float>(M_PI)), 0.0f, 1.0f);
    uint32_t phi_discrete = static_cast<uint32_t>(std::round(normPhi * 511.0f));
    return phi_discrete;
}

uint32_t L1Muon::RPCPatternUtils::getBMBOPhiStripCoordinate(const Amg::Vector3D& stripPos) {
    // Just trigonometry to get the phi coordinate
    float phi = stripPos.phi() + M_PI; // convert to [0, 2*pi] range
    float normPhi = std::clamp(phi / (2.0f * static_cast<float>(M_PI)), 0.0f, 1.0f);
    uint32_t phi_discrete = static_cast<uint32_t>(std::round(normPhi * 511.0f));
    return phi_discrete;
}

void L1Muon::RPCPatternUtils::getStripIdforAlg(const RdoPairs& rdo_pair, std::vector<uint32_t>& stripIDs, std::map<uint32_t, uint32_t>& strip_coordinates,
                                               std::map<uint32_t, uint32_t>& strip_etabin,
                                               const Muon::RpcCablingData& cabling, const Identifier& offId, const ActsTrk::GeometryContext& geoCtx, 
                                               const Muon::IMuonIdHelperSvc* idHelperSvc, const MuonGMR4::MuonDetectorManager* detMgr) {
    
    // This function should convert the Identifier to the strip ID used in the algorithm
    // The exact implementation depends on how the strip ID is defined

    // Get the RPC station information from the cabling map
    Muon::MuonStationIndex::StIndex RPClayer = idHelperSvc->stationIndex(offId); // RPC[0-3] --> 0:BI, 1:BM, 2:BO, 3:BE
    int stationEta = cabling.eta;             // offline chamber eta index, can be negative or positive
    int sector = idHelperSvc->sector(offId);     // offline sector index [1-16] --> we want [0-15]
    bool side = stationEta > 0;               // side = 1 for positive eta, 0 for negative eta
    int doubletPhi = cabling.doubletPhi;      // it identifies the half-sector within the sector! [1-2] --> we want [0-1]
    int doubletR = cabling.doubletR;          // distinguishes BM1 and BM2 [1-2] --> we want [0-1]
    int doubletZ = cabling.doubletZ;          // group the strips in the Z direction [1-2] --> we want [0-1]
    bool measuresPhi = cabling.measuresPhi(); // true for phi strips, false for eta strips
    int stripId = cabling.strip;

    const MuonGMR4::RpcReadoutElement* readOutEle = detMgr->getRpcReadoutElement(offId);
  
    // Get global position
    Amg::Vector3D stripPos = readOutEle->stripPosition(geoCtx, offId); // in global coordinates


    // get Z coordinate referred to R = center of the chamber (as in agreement with MDTTP)   
    Acts::Transform3 g2l = readOutEle->globalToLocalTransform(geoCtx);
    Amg::Vector3D localStripPos = g2l * stripPos;
    localStripPos.z() = 0.0;
    const Acts::Transform3& l2g = readOutEle->localToGlobalTransform(geoCtx);
    Amg::Vector3D centerPlaneGlobalPos = l2g * localStripPos;
    double z_at_center_plane = std::abs(centerPlaneGlobalPos.z());
    // Pack the Z coordinate into 12 bits [first 2 empty bits, then 10 bits for the range [0, 16368] mm
    // mapped to [0, 1023] (10 bits)]
    z_at_center_plane = std::clamp(z_at_center_plane / 16368.0, 0.0, 1.0); 
    uint32_t z_discrete = static_cast<uint32_t>(std::round(z_at_center_plane * 1023.0f));

    if (!measuresPhi) {
        if (stripId == static_cast<int>(0xFFFFFFFF)) { return; } //
        uint32_t etaStripID = getSLStripId(side, sector, doubletPhi, RPClayer, doubletR, stationEta, doubletZ, stripId, measuresPhi);

        stripIDs.push_back(etaStripID);
        float eta = stripPos.eta();
        uint32_t etaBin_discrete = std::min(static_cast<uint32_t>(std::round( std::abs(eta) / 0.05f)), 22u); // eta bin in the range [-1.05, 1.05] with a step of 0.05
        strip_etabin[etaStripID] = etaBin_discrete; // pack the eta coordinate in the range [-1.05, 1.05] into 8 bits
        strip_coordinates[etaStripID] = z_discrete; // pack the z coordinate in the range [0, 16368] mm into 10 bits(+highest 2 bits empty)

        if (RPClayer == Muon::MuonStationIndex::StIndex::BI || (RPClayer == Muon::MuonStationIndex::StIndex::BO && doubletR == 1 && (sector == 11 || sector == 15))) {
          // if the strip is in the BI-BOR/BOM layer, we do not need to check for the phi strip
          // We create artificial phi strip for BI layer!
          int phiStrip = 0; // default value, will be calculated below
          uint32_t phi_discrete = 0; // default value, will be calculated below

          //TODO: check TOF use when we have just one RDO in the pair
          //.     or set the phi strip to the center of the chamber if we have just one RDO in the pair
          if (rdo_pair[0] && rdo_pair[1]) {
            // determine the local position along the BI strip using the time difference between the two RDOs
            float loc_pos_along_BIstrip = 0.5*s_propagationVelocity*(rdo_pair[0]->time() - rdo_pair[1]->time());
            phiStrip = getVirtualBIPhiStripNumber(loc_pos_along_BIstrip, *readOutEle);
            phi_discrete = getVirtualBIPhiStripCoordinate(stripPos, loc_pos_along_BIstrip);
          } else if (rdo_pair[0]) {
            // if only the first RDO is present, we can estimate the local position along the BI strip using its time and the known half-length of the strip
            float loc_pos_along_BIstrip = 0; //(-readOutEle->getParameters().halfWidth + s_propagationVelocity * (rdo_pair[0]->time() - TOF)); // use the time of flight to estimate the local position along the strip);
            phiStrip = getVirtualBIPhiStripNumber(loc_pos_along_BIstrip, *readOutEle);
            phi_discrete = getVirtualBIPhiStripCoordinate(stripPos, loc_pos_along_BIstrip);
          } else if (rdo_pair[1]) {
            // same as above but using the second RDO in the pair
            float loc_pos_along_BIstrip = 0; // (readOutEle->getParameters().halfWidth - s_propagationVelocity *(rdo_pair[1]->time() - TOF)); // use the time of flight to estimate the local position along the strip);
            phiStrip = getVirtualBIPhiStripNumber(loc_pos_along_BIstrip, *readOutEle);
            phi_discrete = getVirtualBIPhiStripCoordinate(stripPos, loc_pos_along_BIstrip);
          }

          uint32_t phiStripID = getSLStripId(side, sector, doubletPhi, RPClayer, doubletR, stationEta, doubletZ, phiStrip, true);
          stripIDs.push_back(phiStripID);
          strip_coordinates[phiStripID] = phi_discrete;

          // doubling the eta strip so that both instances of the trigger algo can see it.
          // Since all BI have doubletPhi=1, except for special cases (sector 9 and BOR/BOM in sectors 11 and 15 with doubletR == 1,2),
          // we can just flip doubletPhi between 1 and 2 for the second instance of the trigger algo.:
          doubletPhi = (doubletPhi == 2) ? 1 : 2; // flip doubletPhi between 1 and 2

          uint32_t etaStripID_2 = getSLStripId(side, sector, doubletPhi, RPClayer, doubletR, stationEta, doubletZ, stripId, false);
          stripIDs.push_back(etaStripID_2);
          strip_etabin[etaStripID_2] = strip_etabin[etaStripID];
          strip_coordinates[etaStripID_2] = strip_coordinates[etaStripID];
        
          uint32_t phiStripID_2 = getSLStripId(side, sector, doubletPhi, RPClayer, doubletR, stationEta, doubletZ, phiStrip, true);
          stripIDs.push_back(phiStripID_2);
          strip_coordinates[phiStripID_2] = strip_coordinates[phiStripID];
        }
    } else {
      if (stripId == static_cast<int>(0xFFFFFFFF)) { return; } // invalid strip number
      uint32_t phiStripID = getSLStripId(side, sector, doubletPhi, RPClayer, doubletR, stationEta, doubletZ, stripId, measuresPhi);
      
      stripIDs.push_back(phiStripID);
      strip_coordinates[phiStripID] = getBMBOPhiStripCoordinate(stripPos);
    }

}