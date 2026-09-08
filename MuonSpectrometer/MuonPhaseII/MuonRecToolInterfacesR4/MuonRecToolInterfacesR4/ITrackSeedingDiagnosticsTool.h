/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_ITRACKSEEDDIAGNOSTICSTOOL_H
#define MUONRECTOOLINTERFACESR4_ITRACKSEEDDIAGNOSTICSTOOL_H

#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"

#include "MuonTrackEvent/MsTrackSeed.h"

namespace MagField{
  class AtlasFieldCache;
}


namespace MuonR4 {
    /** @brief Extension of the ITrackSeedinTool interface to monitor the performance
     *         of the most crucial methods inside the classical MS track seeding. 
     *         The tool is entirely meant for validation purposes. */
    class ITrackSeedingDiagnosticsTool : virtual public ITrackSeedingTool {
        public:
            /** @brief Default destructor */
            virtual ~ITrackSeedingDiagnosticsTool() = default;
            /** @brief Declare the interface  */
            DeclareInterfaceID(MuonR4::ITrackSeedingDiagnosticsTool, 1, 0);
            /** @brief Take the interface estimateQtimesP method upstream */
            using ITrackSeedingTool::estimateQtimesP;
    
            using Location = MsTrackSeed::Location;
            /** @brief Returns whether the expression on the cylinder is within the surface bounds
              * @param projPos: Projected position on the cylinder
              * @param loc: Surface location: [barrel/endcap] */
            virtual bool withinBounds(const Amg::Vector2D& projPos,
                                      const Location loc) const = 0;

            /** @brief Expresses the passed segment on the virtual cylinder 
              *         constructed by the track seeder. The segment's position
              *         is projected onto the expanded sector plane using the
              *         sensor dircection of the first precision measurement.
              *         Then projected position and the direction vector are 
              *         projected into 2D to intersect with the cylinder surface.
              * @param tgContext: The geometry context to align the measurement surfaces
              *                   associated  with the segment
              * @param segment: The segment that is to be projected.
              * @param loc: Location of the projection [barrel/endcap]
              * @param sector: Phi sector onto which the segment is moved */
            virtual Amg::Vector2D expressOnCylinder(const Acts::GeometryContext& tgContext,
                                                    const xAOD::MuonSegment& segment,
                                                    const Location loc,
                                                    const ExpandedSector sector) const = 0;
            /** @brief Estimate the charge times momentum of a muon track candidate from the 
              *        contained segments. The position and direction of the segments are projected onto a 
              *        given phi plane, defined by the segments with phi information or the sector plane.  
              *        The muon trajectory is approximated as 2D trajectory within this plane to avoid side 
              *        effects from (non)-present phi measurements.
              *  @param tgContext: The geometry context to align the segment w.r.t sector
              *  @param seed: Reference to the seed of interest.
              *  @param fieldCache: The initialized magnetic field map*/
            virtual double estimateQtimesP(const Acts::GeometryContext& tgContext,
                                           const MsTrackSeed& seed,
                                           MagField::AtlasFieldCache& fieldCache) const = 0;
    };
}

#endif