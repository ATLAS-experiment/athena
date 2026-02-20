/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_ITRACKSUMMARYTOOL_H
#define MUONRECTOOLINTERFACESR4_ITRACKSUMMARYTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>

#include "ActsEvent/TrackContainer.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODBase/IParticle.h"


namespace Trk {
    class Track;
}

namespace MuonR4{
    struct HitSummary;
    class MsTrackSeed;
}


namespace MuonR4{
    /** @brief Interface tool to calculate the hit summary of tracks & track seeds in the 
     *         MuonSpectrometer. The hit summary counts the number of hits per MS layer 
     *         (Inner, Middle, Outer, Extended, BarrelExtended), by whether a hit is a precision
     *         or a trigger hit, and whether the hit is on-track outlier or a hole. */
    class ITrackSummaryTool: virtual public IAlgTool {
        public:
            /** @brief Default destructor */
            virtual ~ITrackSummaryTool() = default;
            /** @brief Declare the interface  */
            DeclareInterfaceID(MuonR4::ITrackSummaryTool, 1, 0);
           
            /** @brief Abrivation of the Track proxy */
            using ConstTrack_t = ActsTrk::TrackContainer::ConstTrackProxy;
            /** @brief Creates a summary from the passed track
             *  @param ctx: EventContext to fetch conditions such that holes
             *              associated to dead modules are ignored
             * @param trackProxy: Reconstructed MS track for which the summary
             *                    shall be made. */
            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const ConstTrack_t trackProxy) const = 0;
            /** @brief Creates a summary from a list of passed segments that are associated
             *         to a track seed, a truth particle or a reconstructed track
             *  @param ctx: EventContext to fetch conditions such that holes
             *              associated to dead modules are ignored
             *  @param segments: List of segments from which the summary shall be created*/
            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const std::vector<const xAOD::MuonSegment*>& segments) const = 0;
            /** @brief Creates a summary from a Trk::Track object produced by the legacy reconstruction software.
             *         The associated hits are categorized the same way as for the ActsTrk::Tracks 
             *         (Serves purely validation purposes)
             * @param ctx: EventContext to fetch conditions such that holes
             *             associated to dead modules are ignored
             * @param trk: Reference to the MS track for which the summary shall be produced */
            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const Trk::Track& trk) const = 0;
            /** @brief Decorates the hit summary to the parsed track (xAOD::TrackParticle, xAOD::Muon or xAOD::TruthParticle)
             *         using the categories defined in the xAOD::MuonSummaryType enum */
            virtual void copySummary(const HitSummary& summary,
                                     const xAOD::IParticle& track) const = 0;

    };
}


#endif
