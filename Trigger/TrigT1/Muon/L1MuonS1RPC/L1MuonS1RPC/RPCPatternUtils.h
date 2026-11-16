/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Class to provide utility functions for RPC pattern matching,
// including functions to extract strip numbers and coordinates from identifiers 
// and to compute virtual strip coordinates for BI-BOR/BOM stations.

#ifndef RPCPatternUtils_H
#define RPCPatternUtils_H

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "xAODMuonRDO/NRPCRDO.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include <GaudiKernel/PhysicalConstants.h>

#include <map>
#include <vector>
#include <cstdint>

class MsgStream;
class Identifier;
namespace Muon{
    class IMuonIdHelperSvc;
    class RpcCablingData;
}

namespace MuonGMR4{
    class RpcReadoutElement;
    class MuonDetectorManager;
}

namespace L1Muon::RPCPatternUtils {

    using RdoPairs = std::array<const xAOD::NRPCRDO*, 2>;
    uint32_t getSLStripId(bool side, int sector, int doubletPhi, Muon::MuonStationIndex::StIndex RPClayer,
                            int doubletR, int stationEta, int doubletZ, int stripNumber, bool measuresPhi) ;

    int getVirtualBIPhiStripNumber(float loc_pos_along_BIstrip, const MuonGMR4::RpcReadoutElement& readoutElement);

    uint32_t getVirtualBIPhiStripCoordinate(const Amg::Vector3D& stripPos, float loc_pos_along_BIstrip);

    uint32_t getBMBOPhiStripCoordinate(const Amg::Vector3D& stripPos);

    void getStripIdforAlg(const RdoPairs& rdo_pair, std::vector<uint32_t>& stripIDs, std::map<uint32_t, uint32_t>& strip_coordinates, std::map<uint32_t, uint32_t>& strip_etabin,
                                               const Muon::RpcCablingData& cabling, const Identifier& offId, const ActsTrk::GeometryContext& geoCtx,
                                               const Muon::IMuonIdHelperSvc* idHelperSvc, const MuonGMR4::MuonDetectorManager* detMgr);

    inline constexpr double s_digitalBiPhiStripWidth = 150.0; // mm
    inline constexpr double s_propagationVelocity = 2./3. * Gaudi::Units::c_light; // mm/ns
}

#endif // RPCPatternUtils_H