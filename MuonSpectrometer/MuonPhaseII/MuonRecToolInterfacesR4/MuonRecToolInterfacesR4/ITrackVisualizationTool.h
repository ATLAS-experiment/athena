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

class EventContext;
class TObject;

namespace MuonR4{
   class MsTrackSeeder;
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
             *  @param seeds: The constructed track seeds from the event
             *  @param extraLabel: Extra label to be put onto the top of the shown canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      const std::string& extraLabel) const = 0;
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
             *         and draws the markers of the successfully built seeds & truth segments
             *  @param ctx: EventContext to access store gate & conditions
             *  @param seederObj: Configured instance of the track seeder which actually constructed 
             *                    the seeds.
             *  @param segments: Container of all MS segments in the event
             *  @param seeds: The constructed track seeds from the event
             *  @param extraLabel: Extra label to be put onto the top of the shown canvases
             *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      const std::string& extraLabel,
                                      PrimitivesVec_t && extPrimitives) const = 0;
  
    };
}
#endif