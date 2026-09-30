/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTrackEvent/ExpandedSector.h>
#include <MuonStationIndex/MuonStationIndex.h>
#include <FourMomUtils/P4Helpers.h>
#include "Acts/Utilities/MathHelpers.hpp"
#include <stdlib.h>
#include <iostream>
#undef NDEBUG

using namespace MuonR4;
using namespace Muon::MuonStationIndex;
using namespace P4Helpers;


#define LOG_MSG(msg)\
    std::cout<<__func__<<" "<<__LINE__<<" - "<<msg<<std::endl;

#define ERROR_MSG(msg) \
    std::cerr<<__func__<<" "<<__LINE__<<" - ERROR: "<<msg<<std::endl; \
    ret_code = EXIT_FAILURE;


std::pair<unsigned, unsigned> minMax(const unsigned a, const unsigned b) {
    return std::make_pair(std::min(a,b), std::max(a,b));
}

int main() {
    int ret_code = EXIT_SUCCESS;
    using SectorProjector = ExpandedSector::SectorProjector;
    for (unsigned sector= 1; sector<= numberOfSectors(); ++sector) {
        for (const auto proj : {SectorProjector::leftOverlap,
                                SectorProjector::center,
                                SectorProjector::rightOverlap}) {
            const ExpandedSector expandSector{sector, proj};
            std::cout<<std::endl;
            LOG_MSG("Test sector: "<<sector<<", projector: "<<proj<<" === built: "<<expandSector);
            
            /// Construct the neighbouring sector
            unsigned neighbourSector = (sector + toInt(proj));
            if (neighbourSector < 1) neighbourSector = numberOfSectors();
            else if (neighbourSector > numberOfSectors()) neighbourSector = 1;

            const auto [s1, s2] = minMax(sector, neighbourSector);
            const auto [s3 ,s4] = minMax(expandSector.msSector(), expandSector.adjacentMsSector());
            if ( s1 != s3 || s2 != s4) {
                ERROR_MSG("The sector pairing does not match...  sector: "<<sector<<", neighbour sector: "
                        <<neighbourSector<<" vs. "<<expandSector.msSector()<<", "<<expandSector.adjacentMsSector());
            }
            SectorProjector neighProj{proj};
            if (proj == SectorProjector::leftOverlap) {
                neighProj = SectorProjector::rightOverlap;
            } else if (proj == SectorProjector::rightOverlap) {
                neighProj = SectorProjector::leftOverlap;
            }

            const ExpandedSector expandNeighbour{neighbourSector, neighProj};
            LOG_MSG("Associated neighbour sector "<<neighbourSector<<" projector: "<<neighProj
                    <<" ==> "<<expandNeighbour);
            /// Ensure that the neighbouring sector is mapped onto the same expanded sector
            if (expandNeighbour != expandSector) {
                ERROR_MSG("The expanded sectors don't match "<<expandSector
                           <<" vs. "<<expandNeighbour);
            }
            /// Test that the expanded sector is back converted to the 
            /// sector + proj or the neighbour + associated projector
            const unsigned backSector = expandSector.msSector();
            const auto backProj = expandSector.projector();
            LOG_MSG("The back conversion is "<<backSector<<", projector: "<<backProj);
            if ( !( (backSector == sector && backProj == proj ) || 
                    (backSector == neighbourSector && backProj == neighProj) )) {
                ERROR_MSG("Back conversion failed");
            }
            const double phi = expandSector.phi();
            const double cenPhi = ExpandedSector{sector, SectorProjector::center}.phi();
            if (proj != SectorProjector::center) {
                const double dPhi = deltaPhi(phi, cenPhi);
                LOG_MSG("Delta phi: "<<dPhi);
                const int side = Acts::copySign(1, proj);
                if (dPhi * side < 0) {
                    ERROR_MSG("The sign of the delta phi does not match the expectations");           
                }
            }
            ExpandedSector expandSecFromPhi{phi};
            if (expandSecFromPhi != expandSector) {
                ERROR_MSG("The phi value "<<phi<<" does not map back to the same expanded sector "
                          <<expandSecFromPhi<<" vs. "<<expandSector);
            }
        }
    }

    // Test the neigbor function for the expanded sector
    const int8_t nExpanded = numberOfSectors() * 2;
    for(int8_t sector{0}; sector < nExpanded; ++sector) {
        const ExpandedSector expandSector{sector};
        LOG_MSG("Testing expanded sector: "<<expandSector);
        int8_t neighbourSector = (sector +  1) == nExpanded ? 0 : sector + 1;
        const ExpandedSector expandNeighbour{neighbourSector};
        if (!expandSector.isNeighbour(expandNeighbour)) {
            ERROR_MSG("The expanded sector "<<expandSector<<" is not neighbour to "<<expandNeighbour);
        }
        if(!expandNeighbour.isNeighbour(expandSector)) {
            ERROR_MSG("The expanded sector "<<expandNeighbour<<" is not neighbour to "<<expandSector);
        }
    }


    return ret_code;
}
