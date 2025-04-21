/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRECTOOLINTERFACESR4_ISEGMENTSELECTIONTOOL_H
#define MUONRECTOOLINTERFACESR4_ISEGMENTSELECTIONTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>

namespace MuonR4 {
    class Segment;

    class ISegmentSelectionTool: virtual public IAlgTool {
        public:
            DeclareInterfaceID(ISegmentSelectionTool, 1, 0);
            
            virtual ~ISegmentSelectionTool() = default;

            /** @brief Returns whether a segment provides enough mdt & phi measurements
             *         to use it for track finding seeding
             *  @param ctx: Event context to access conditions & geometry data
             *  @param segment: Reference to the segment to consider */
            virtual bool passSeedingQuality(const EventContext& ctx,
                                            const Segment& segment) const = 0;
            /** @brief Returns whether a segment passes the base selection quality
             *         in order to be picked up onto a track
             *  @param ctx: Event context to access conditions & geometry data
             *  @param segment: Reference to the segment to consider */
            virtual bool passTrackQuality(const EventContext& ctx,
                                          const Segment& segment) const = 0;

            /** @brief Returns whether a segment passes the base selection quality
             *         in order to be picked up onto a track
             *  @param ctx: Event context to access conditions & geometry data
             *  @param segment: Reference to the segment to consider */
            virtual bool compatibleForTrack(const EventContext& ctx,
                                           const Segment& segA,
                                           const Segment& segB) const = 0;                                 
    };

}

#endif
