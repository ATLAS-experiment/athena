/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRECTOOLINTERFACESR4_ISEGMENTSELECTIONTOOL_H
#define MUONRECTOOLINTERFACESR4_ISEGMENTSELECTIONTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>

#include <xAODMuon/MuonSegment.h>

namespace MuonR4 {
    /** @brief Tool interface to define segment selection quality. Segments are 
      *        divided into three categories
      * 
      *         1) Seeding quality: The segment contains enough precision and 
      *            phi hits to use it as a seed to start the track seeding procedure
      *         2) Track quality: The segment contains enough precision hits to be
      *            used for a track fit. Its source is less likely to be background
      *         3) Rest: The segment fails the track quality selection
      * 
      *        Further the selection tool defines the interface to decide whether
      *        two segments are compatible enough to be combined for a track
      *        seed candidate */
    class ISegmentSelectionTool: virtual public IAlgTool {
        public:
            DeclareInterfaceID(ISegmentSelectionTool, 1, 0);
            
            virtual ~ISegmentSelectionTool() = default;

            /** @brief Returns whether a segment provides enough mdt & phi measurements
             *         to use it for track finding seeding
             *  @param ctx: Event context to access conditions & geometry data
             *  @param segment: Reference to the segment to consider */
            virtual bool passSeedingQuality(const EventContext& ctx,
                                            const xAOD::MuonSegment& segment) const = 0;
            /** @brief Returns whether a segment passes the base selection quality
             *         in order to be picked up onto a track
             *  @param ctx: Event context to access conditions & geometry data
             *  @param segment: Reference to the segment to consider */
            virtual bool passTrackQuality(const EventContext& ctx,
                                          const xAOD::MuonSegment& segment) const = 0;

            /** @brief Returns whether two segments are compatible enough to be put both
             *         onto the track seed.
              * @param ctx: Event context to access conditions & geometry data
              * @param segment: Reference to the segment to consider */
            virtual bool compatibleForTrack(const EventContext& ctx,
                                           const xAOD::MuonSegment& segA,
                                           const xAOD::MuonSegment& segB) const = 0;                                 
    };

}

#endif
