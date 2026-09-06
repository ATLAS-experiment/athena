/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_CORESTRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_CORESTRIPSPACEPOINTFORMATIONTOOL_H

#include "StripSpacePointFormationToolBase.h"

#include "Acts/SpacePointFormation/SpacePointFormationError.hpp"
#include "Acts/SpacePointFormation/StripSpacePointBuilder.hpp"
#include "Acts/Utilities/Result.hpp"

#include "CxxUtils/checker_macros.h"

#include <array>
#include <atomic>
#include <optional>

namespace ActsTrk {

    /// @class CoreStripSpacePointFormationTool
    /// Strip space point formation using Acts::StripSpacePointBuilder for the
    /// intersection of the two strips. Two modes are available:
    ///  - Constrained: the trajectory is assumed to originate from the vertex,
    ///    equivalent to the Athena implementation in StripSpacePointFormationTool
    ///  - Cosmic: no vertex hypothesis, the space point is the point of closest
    ///    approach of the two strip lines, kept only if it lies on both strips
    /// Everything else is inherited from StripSpacePointFormationToolBase.

    class CoreStripSpacePointFormationTool : public StripSpacePointFormationToolBase {
    public:
        using StripSpacePointFormationToolBase::StripSpacePointFormationToolBase;
        virtual ~CoreStripSpacePointFormationTool() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

    protected:

        virtual StatusCode makeStripSpacePoint(std::vector<StripSP>& collection,
                                               const StripInformationHelper& firstInfo,
                                               const StripInformationHelper& secondInfo,
                                               const Amg::Vector3D& beamSpotVertex,
                                               bool isEndcap,
                                               double limit,
                                               double slimit) const override;

    private:

        /// Recover the two ends of the strip from the centre and direction cached
        /// in the helper. Acts::StripEnds::top matches the Athena strip start.
        static Acts::StripSpacePointBuilder::StripEnds stripEnds(const StripInformationHelper& info);

        /// Book a rejected pair against the slot of its Acts::SpacePointFormationError
        void countError(int code) const;

        Gaudi::Property<std::string> m_mode{this, "Mode", "Constrained",
          "Space point formation mode: Constrained (vertex hypothesis) or Cosmic (closest approach)"};
        Gaudi::Property<double> m_cosmicTolerance{this, "CosmicTolerance", 1e-6,
          "Reject strip pairs whose sin^2 of the opening angle is below this, Cosmic mode only"};

        bool m_cosmicMode{false};

        /// Outcome counters. The failure slots are indexed by the value of
        /// Acts::SpacePointFormationError, which starts at 1.
        enum EStat {
          kNAccepted = 0,
          kClusterPairDistanceExceeded,
          kClusterPairThetaDistanceExceeded,
          kClusterPairPhiDistanceExceeded,
          kCosmicToleranceNotMet,
          kOutsideLimits,
          kOutsideRelaxedLimits,
          kNoSolutionFound,
          kOtherError,
          kNStat
        };

        static_assert(static_cast<int>(Acts::SpacePointFormationError::ClusterPairDistanceExceeded)
                      == kClusterPairDistanceExceeded);
        static_assert(static_cast<int>(Acts::SpacePointFormationError::NoSolutionFound)
                      == kNoSolutionFound);

        mutable std::array<std::atomic<unsigned int>, kNStat> m_stat ATLAS_THREAD_SAFE {};
  };

}

#endif
