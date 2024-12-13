/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <algorithm>

#include "GeoPrimitives/GeoPrimitives.h"
#include "xAODTracking/TrackParticle.h"

class TrigMuonEFTrack;
class MuonFeature;

namespace Muon {
    class MuonSegment;
}
namespace Trk {
    class TrackSurfaceIntersection;
    class Track;
}  // namespace Trk

namespace MuGirlNS {
    class Intersection;

    enum StauHitTechnology { RPC_STAU_HIT = 0, RPCETA_STAU_HIT, MDTT_STAU_HIT, CALO_STAU_HIT, CSC_STAU_HIT };

    inline std::string hitTechnologyName(StauHitTechnology eTech) {
        switch (eTech) {
            case RPC_STAU_HIT: return "RPC";
            case RPCETA_STAU_HIT: return "RPCETA";
            case MDTT_STAU_HIT: return "MDTT";
            case CALO_STAU_HIT: return "CALO";
            case CSC_STAU_HIT: return "CSC";
            default: return std::string();
        }
    }

    struct StauHit {
        StauHitTechnology eTech{};
        double mToF{-1.};
        double x{0.};
        double y{0.};
        double z{0.};
        Identifier id{};
        double e{-1.};
        double error{-1.};
        double shift{0.};
        bool measuresEta{false};
        double propagationTime{0.};

        StauHit() = default;
        StauHit(StauHitTechnology tech, double tof, double ix, double iy, double iz, Identifier iid, double ie, double er = -1,
                double sh = 0, bool isEta = false, double propTime = 0) :
            eTech(tech),
            mToF(tof),
            x(ix),
            y(iy),
            z(iz),
            id(iid),
            e(ie),
            error(er),
            shift(sh),
            measuresEta(isEta),
            propagationTime(propTime) {}
    };
    using StauHits =  std::vector<StauHit>;

    struct StauExtras {
        double ann{0.};
        double betaAll{0.};
        double betaAllt{0.};
        unsigned int numRpcHitsInSeg{0};
        unsigned int numCaloCells{0};

        double rpcBetaAvg{0.};
        double rpcBetaRms{0.};
        double rpcBetaChi2{0.};
        unsigned int rpcBetaDof{0};
        double mdtBetaAvg{0.};
        double  mdtBetaRms{0.};
        double mdtBetaChi2{0.};
        unsigned int mdtBetaDof{0};
        double caloBetaAvg{0.}; 
        double caloBetaRms{0.};
        double caloBetaChi2{0.};
        int caloBetaDof{0};
        StauHits hits;

        void addHits(StauHits stauHits) { std::copy(stauHits.begin(), stauHits.end(), std::back_inserter(hits)); }
    };

}  // namespace MuGirlNS
