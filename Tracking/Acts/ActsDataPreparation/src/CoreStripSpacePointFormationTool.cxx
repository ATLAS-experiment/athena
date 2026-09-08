/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CoreStripSpacePointFormationTool.h"

#include "ActsInterop/TableUtils.h"

namespace ActsTrk {

    StatusCode CoreStripSpacePointFormationTool::initialize()
    {
        ATH_CHECK( StripSpacePointFormationToolBase::initialize() );

        if (m_mode.value() == "Cosmic") {
            m_cosmicMode = true;
        } else if (m_mode.value() == "Constrained") {
            m_cosmicMode = false;
        } else {
            ATH_MSG_FATAL("Unknown Mode '" << m_mode.value() << "', expected 'Constrained' or 'Cosmic'");
            return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("Using Acts::StripSpacePointBuilder in " << m_mode.value() << " mode");

        return StatusCode::SUCCESS;
    }

    StatusCode CoreStripSpacePointFormationTool::finalize()
    {
        ATH_MSG_INFO("Strip space point formation statistics" << std::endl << makeTable(m_stat,
                       std::array<std::string, kNStat>{
                         "Accepted",
                         "ClusterPairDistanceExceeded",
                         "ClusterPairThetaDistanceExceeded",
                         "ClusterPairPhiDistanceExceeded",
                         "CosmicToleranceNotMet",
                         "OutsideLimits",
                         "OutsideRelaxedLimits",
                         "NoSolutionFound",
                         "OtherError"
                       }).columnWidth(12));

        return StatusCode::SUCCESS;
    }

    Acts::StripSpacePointBuilder::StripEnds
      CoreStripSpacePointFormationTool::stripEnds(const StripInformationHelper& info)
    {
        // stripCenter = 0.5*(start+end), stripDirection = start-end
        const Amg::Vector3D half = 0.5 * info.stripDirection();
        return Acts::StripSpacePointBuilder::StripEnds{info.stripCenter() + half,
                                                       info.stripCenter() - half};
    }

    void CoreStripSpacePointFormationTool::countError(int code) const
    {
        ++m_stat[(code > 0 and code < kOtherError) ? code : kOtherError];
    }

    StatusCode CoreStripSpacePointFormationTool::makeStripSpacePoint(
       std::vector<StripSP>& collection,
       const StripInformationHelper& firstInfo,
       const StripInformationHelper& secondInfo,
       const Amg::Vector3D& beamSpotVertex,
       bool isEndcap,
       double limit,
       double slimit) const
    {
        namespace SPB = Acts::StripSpacePointBuilder;

        if (m_cosmicMode) {
            SPB::CosmicOptions options;
            options.tolerance = m_cosmicTolerance;
            options.stripLengthTolerance = limit - 1.;

            const Acts::Result<Acts::Vector3> cosmic =
              SPB::computeCosmicSpacePoint(stripEnds(firstInfo), stripEnds(secondInfo), options);
            if (not cosmic.ok()) {
                countError(cosmic.error().value());
                return StatusCode::SUCCESS;
            }
            ++m_stat[kNAccepted];
            collection.push_back(makeStripSP(*cosmic, firstInfo, secondInfo, isEndcap));
            return StatusCode::SUCCESS;
        }

        // The vertex is already folded into the cached trajectory direction, so
        // options.vertex is not read by this overload.
        SPB::ConstrainedOptions options;
        options.vertex = beamSpotVertex;
        options.stripLengthTolerance = limit - 1.;
        options.stripLengthGapTolerance = slimit;

        Acts::Vector3 position;
        const std::optional<Acts::SpacePointFormationError> error =
          SPB::computeConstrainedSpacePoint(firstInfo.constrainedCache(),
                                            secondInfo.constrainedCache(), options, position);
        if (error.has_value()) {
            countError(static_cast<int>(*error));
            return StatusCode::SUCCESS;
        }

        ++m_stat[kNAccepted];
        collection.push_back(makeStripSP(position, firstInfo, secondInfo, isEndcap));

        return StatusCode::SUCCESS;
    }

}
