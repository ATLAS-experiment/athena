/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonStationIndex/MuonStationIndex.h"


#include "GeoModelKernel/throwExcept.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/UnitVectors.hpp"
#include "Acts/Definitions/Units.hpp"

#include <cassert>
using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;

namespace{
    static const Muon::MuonSectorMapping sectorMap{};
    using SectorProjector = MuonR4::ExpandedSector::SectorProjector;
    constexpr std::int8_t nExpanded = 2*numberOfSectors();

    inline std::tuple<unsigned, SectorProjector> msSectorAndProj(const std::int8_t expandSector) {
        assert (expandSector >= 0 && expandSector <= nExpanded);
        if (expandSector == 0) {
            return std::make_tuple(numberOfSectors(), SectorProjector::center);
        } else if (expandSector == 1) {
            return std::make_tuple(numberOfSectors(), SectorProjector::rightOverlap);
        }
        const int regSector = expandSector / 2;
        const int backConv = expandSector - 2*regSector;
        switch (backConv) {
            case 1: return std::make_tuple(regSector, SectorProjector::rightOverlap);
            case 0: return std::make_tuple(regSector, SectorProjector::center);
            case -1: return std::make_tuple(regSector, SectorProjector::leftOverlap);
            default:
                THROW_EXCEPTION("Cannot deduce the sector overlap "<<expandSector);
        }
        return std::make_tuple(0, SectorProjector::center);
    }

}
namespace MuonR4 {

    ExpandedSector::ExpandedSector(const std::int8_t expSector):
        m_sector{expSector}{}
    ExpandedSector::ExpandedSector(const unsigned msSector,
                                   const SectorProjector proj) {
        m_sector = (2*msSector + Acts::toUnderlying(proj)) % nExpanded;
    }

    ExpandedSector::ExpandedSector(const double phi){
        std::vector<int> sectors{};
        sectorMap.getSectors(phi, sectors);
        assert(!sectors.empty());
        if (sectors.size() == 1) {
            (*this) = ExpandedSector{static_cast<unsigned>(sectors[0]), SectorProjector::center};
        } else {
            const int dS = (sectors[1] - sectors[0]) % numberOfSectors();
            assert(std::abs(ds) == 1);
            (*this) = ExpandedSector{static_cast<unsigned>(sectors[0]), 
                                     static_cast<SectorProjector>(dS)};
        }
    }

    SectorProjector ExpandedSector::projector() const { 
       return std::get<1>(msSectorAndProj(sector())); 
    }
    unsigned ExpandedSector::msSector() const {
        return std::get<0>(msSectorAndProj(sector()));
    }
    std::int8_t ExpandedSector::sector() const { 
        return m_sector; 
    }
    unsigned ExpandedSector::adjacentMsSector() const {
        const auto [msSec, proj] = msSectorAndProj(sector());
        if (msSec == 1 && proj == SectorProjector::leftOverlap){
            return numberOfSectors();
        } else if (msSec == numberOfSectors() && proj == SectorProjector::rightOverlap) {
            return 1;
        }
        return msSec + Acts::toUnderlying(proj);

    }
    bool ExpandedSector::operator<(const ExpandedSector& other) const {
        return sector() < other.sector();
    }
    bool ExpandedSector::operator==(const ExpandedSector& other) const {
        return sector() == other.sector();
    }
    bool ExpandedSector::operator!=(const ExpandedSector& other) const {
        return sector() != other.sector();
    }
    double ExpandedSector::phi() const {
        return sectorMap.sectorOverlapPhi(msSector(), adjacentMsSector());
    }
    Amg::Vector3D ExpandedSector::radialDir() const {
        return Acts::makeDirectionFromPhiTheta(phi(), 90._degree);
    }
    Amg::Vector3D ExpandedSector::normalDir() const {
        return Acts::makeDirectionFromPhiTheta(phi() + 90._degree, 90._degree);
    }
    bool ExpandedSector::isNeighbour(const ExpandedSector& other) const {
        const int dS = (other.sector() - sector()) % nExpanded;
        return std::abs(dS) <= 1; 
    }
   
    std::string ExpandedSector::toString(const SectorProjector proj) {
        switch (proj){
            using enum SectorProjector;
            case leftOverlap: return "leftOverlap";
            case center: return "center";
            case rightOverlap: return "rightOverlap";
        }
        return "";
    }
    std::ostream& ExpandedSector::toString(std::ostream& ostr)const {
        ostr<<"Expanded sector: "<<static_cast<int>(sector()) <<" ->  "
        <<projector()<<" of sector "<<msSector();
        return ostr;
    }
}
