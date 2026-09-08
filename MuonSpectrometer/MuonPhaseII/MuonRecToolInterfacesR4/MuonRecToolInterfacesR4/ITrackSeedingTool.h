/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_ITRACKSEEDINGTOOL_H
#define MUONRECTOOLINTERFACESR4_ITRACKSEEDINGTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

// Must be included before any header that pulls in <Eigen/Core> (e.g. Acts
// below): it arms EIGEN_MATRIXBASE_PLUGIN/EIGEN_TRANSFORM_PLUGIN, which add
// the Amg::Vector3D methods (mag(), unit(), perp(), ...) used throughout
// this interface and its implementations. Eigen headers are include-guarded,
// so if something else includes plain Eigen first, those methods silently
// never exist for the rest of the translation unit.
#include "GeoPrimitives/GeoPrimitives.h"

#include "Acts/EventData/BoundTrackParameters.hpp"

#include "xAODMuon/MuonSegmentContainer.h"

#include <span>

namespace MuonR4 {
    class MsTrackSeed;
}

namespace MuonR4 {
    /** @brief Tool interface to construct TrackSeeds from the MuonSegmentContainer. The track seeds are processed
     *         by the calling algorithm to form MuonSpectrometer track candidates. The interface provides methods to
      *                 
      *          1) Construct a vector of track seed. The input how the track seed is constructed depends
      *             on the actual tool implementation of the interface
      * 
      *          2) Estimate the start parameters for the track parameters for a given track seed.
      *             The technique in use may differ between the tool implementations
      * 
      *          3) Extract an initial estimator of the track momentum given a set of 2 or 3
      *             position-direction pairs + reference plane
      * */
    class ITrackSeedingTool : virtual public IAlgTool {
        public:
            /** @brief Default destructor */
            virtual ~ITrackSeedingTool() = default;
            /** @brief Declare the interface  */
            DeclareInterfaceID(MuonR4::ITrackSeedingTool, 1, 0);
           /** @brief Retrieves the segment container from StoreGate and constructs TrackSeeds
             *        from them. The seed canddiates are pushed to the output seed container
             * @param ctx: EventContext to access the xAOD::MuonSegmentContainer from store
             *             gate and additional conditions data if needed to construct the seed
             * @param outSeeds: Mutable reference to the container to whichh the seeds are pushed to. */
            virtual StatusCode findTrackSeeds(const EventContext& ctx,
                                              std::vector<MsTrackSeed>& outSeeds) const = 0;

            virtual Acts::Result<Acts::BoundTrackParameters> 
                                estimateStartParameters(const EventContext& ctx,
                                                        const MsTrackSeed& seed) const = 0;

            using PosMomPair_t = std::pair<Amg::Vector3D, Amg::Vector3D>;

            virtual double estimateQtimesP(const EventContext& ctx,
                                           const Amg::Vector3D& planeNorm,
                                           std::span<const PosMomPair_t> circlePoints) const = 0;


    };
}
#endif