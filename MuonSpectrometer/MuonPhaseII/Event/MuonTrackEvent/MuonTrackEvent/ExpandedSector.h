/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_EXPANDEDSECTOR_H
#define MUONTRACKEVENT_EXPANDEDSECTOR_H

#include <GeoPrimitives/GeoPrimitives.h>
//
#include "Acts/Utilities/OstreamFormatter.hpp"

#include <cstdint>
#include <tuple>
#include <ostream>


/** @brief Helper functions to describe the expanded sector concept. The expanded
 *         sectors are based on the 16 fold symmetry of the MS, but also take into
 *         account the overlap between 2 sectors. 
 *        
 *         The regular msSector is multiplied by 2 and then the sectorProjector is added
 *         which can be either -1 to indicate that the overlap between the current sector
 *         and the left sector is of interest, or 0 to indicate that the sector center is
 *         of interest and finally 1 to indicate that the sector to the right is of interest.
 */
namespace MuonR4 {

    class ExpandedSector {
        public:
            /** @brief Enumeration to select the sector projection of the
             *         regular MS sector */
            enum class SectorProjector : std::int8_t {
                leftOverlap = -1,   /// Project the segment onto the overlap with the previous sector
                center = 0,         /// Project the segment onto the sector centre
                rightOverlap = 1    /// Project the segment on the overlap with the next sector
            };
            /** @brief Return the projector as a string */
            static std::string toString(const SectorProjector proj);
            /** @brief Define the ostream operator */
            friend std::ostream& operator<<(std::ostream& ostr, const SectorProjector proj) {
                return ostr<<(toString(proj));
            }
            friend std::ostream& operator<<(std::ostream& ostr, const ExpandedSector& sec){
                return sec.toString(ostr);
            }
            /** @brief Constructor of the expanded sector taking the
             *         regular MS sector number and the projector
             * @param msSector: Number of the ms reference sector [1-16]
             * @param proj: Splitting of the sector to the overlap with the
             *              left / right adjacent sector or the sector center */
            explicit ExpandedSector(const unsigned msSector,
                                    const SectorProjector proj);
            /** @brief Constructor from an arbitrary phi angle. The angle is
             *         assigned to the msSectors and then the expanded sector
             *         is deduced 
             *  @param phi: Angle from [-pi, pi] */
            explicit ExpandedSector(const double phi);
            /** @brief Constructor from a expanded sector number
             *  @param expSector: Raw expanded sector number */
            explicit ExpandedSector(const std::int8_t expSector);
            /** @brief Define the ordering operator */
            bool operator<(const ExpandedSector& other) const;
            /** @brief Define the equal operator */
            bool operator==(const ExpandedSector& other) const;
            /** @brief Define the unequal operator */
            bool operator!=(const ExpandedSector& other) const;
            /** @brief Returns the ms sector corresponding to the
             *         expanded sector. 
             *  @note If the expanded sector is constructed with the 
             *        left / right overlap. The msSector number might be
             *        the adjacent msSector */
            unsigned msSector() const;
            /** @brief Returns the neighbouring msSector number constructed from
             *         the primary sector and the sector overlap projector */
            unsigned adjacentMsSector() const;
            /** @brief Returns the projector in the corresponding MS sector */
            SectorProjector projector() const;
            /** @brief Returns the expanded sector number */
            std::int8_t sector() const;
            /** @brief Returns the phi angle of the expanded sector */
            double phi() const;
            /** @brief Returns the vector pointing radially along the sector plane  */
            Amg::Vector3D radialDir() const;
            /** @brief Returns the vector that is normal to the plane spanned 
             *         by the expanded sector */
            Amg::Vector3D normalDir() const;
            /** @brief */
            bool isNeighbour(const ExpandedSector& other) const;
        private:
            /** @brief Pipe the object to an ostream  */
            std::ostream& toString(std::ostream& ostr)const;
            /** @brief the sector number stored */
            std::int8_t m_sector{0};
    };
   
}
ACTS_OSTREAM_FORMATTER(MuonR4::ExpandedSector::SectorProjector);

#endif