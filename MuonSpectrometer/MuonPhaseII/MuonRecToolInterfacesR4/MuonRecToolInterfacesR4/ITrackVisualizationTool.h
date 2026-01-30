/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H
#define MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H


#include <GaudiKernel/IAlgTool.h>
#include <GeoPrimitives/GeoPrimitives.h>
#include <xAODMuon/MuonSegmentContainer.h>
#include <MuonTrackEvent/MsTrackSeed.h>
#include <memory>

#include "Acts/EventData/TrackParameters.hpp"
class EventContext;
class TObject;

namespace MuonR4{
   class MsTrackSeeder;
}

namespace ActsTrk{
    class GeometryContext;
}

namespace MuonValR4{
    /** @brief Helper tool to visualize a pattern recogntion incident or a certain stage of the segment fit. */

    class ITrackVisualizationTool : virtual public IAlgTool {
        public:
            virtual ~ITrackVisualizationTool() = default;
            
            DeclareInterfaceID(ITrackVisualizationTool, 1, 0);

            using PrimitivePtr_t = std::unique_ptr<TObject>;
            using PrimitivesVec_t = std::vector<PrimitivePtr_t>;
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
             *         and draws the markers of the successfully built seeds & truth segments
             *  @param ctx: EventContext to access store gate & conditions
             *  @param seederObj: Configured instance of the track seeder which actually constructed 
             *                    the seeds.
             *  @param segments: Container of all MS segments in the event
             *  @param seeds: The constructed track seeds from the event */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds) const = 0;
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
             *         and draws the markers of the successfully built seeds & truth segments
             *  @param ctx: EventContext to access store gate & conditions
             *  @param seederObj: Configured instance of the track seeder which actually constructed 
             *                    the seeds.
             *  @param segments: Container of all MS segments in the event
             *  @param seeds: The constructed track seeds from the event
             *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      PrimitivesVec_t && extPrimitives) const = 0;
 
            using OptBoundPars_t = Acts::Result<Acts::BoundTrackParameters>;
            /** @brief Visualizes the measurements of the segments on the track seed together
             *         with their predicted local line parameters as an obj file. If parameters
             *         to extrapolate are parsed, then they're extrapolated to the end of the world
             *         and the trajectory is added to the obj
             * @param ctx: EventContext to fetch the conditions data & the event information
             * @param seed: MsTrack to visualize
             * @param parsToExt: Parameters to extrapolate on top
             * @param objName: Extra token to be added to the file name */
            virtual void displayTrackSeedObj(const EventContext& ctx,
                                             const MuonR4::MsTrackSeed& seed,
                                             const OptBoundPars_t& parsToExt,
                                             const std::string& objName = "") const = 0;
  
    };
}
#endif