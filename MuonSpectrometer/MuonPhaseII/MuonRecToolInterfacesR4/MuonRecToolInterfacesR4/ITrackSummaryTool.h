/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_ITRACKSUMMARYTOOL_H
#define MUONRECTOOLINTERFACESR4_ITRACKSUMMARYTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>

#include "ActsEvent/TrackContainer.h"
#include "xAODBase/IParticle.h"

namespace MuonR4{
    class HitSummary;
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
            /** @brief Creates a summary from the passed track seed. the summary values from
             *         the associated segments are copied as on-track values for the corresponding layer
             *  @param ctx: EventContext to fetch conditions such that holes
             *              associated to dead modules are ignored
             *  @param seed: Reference to the MS track seed */
            virtual HitSummary makeSummary(const EventContext& ctx,
                                            const MsTrackSeed& seed) const = 0;
            /** @brief Decorates the hit summary to the parsed track (xAOD::TrackParticle, xAOD::Muon or xAOD::TruthParticle)
             *         using the categories defined in the xAOD::MuonSummaryType enum */
            virtual void copySummary(const HitSummary& summary,
                                    xAOD::IParticle& track) const = 0;

    };
}


#endif