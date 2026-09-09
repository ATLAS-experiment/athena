/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_STRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_STRIPSPACEPOINTFORMATIONTOOL_H

#include "StripSpacePointFormationToolBase.h"

namespace ActsTrk {

    /// @class StripSpacePointFormationTool
    /// Strip space point formation using the Athena implementation of the
    /// intersection between the two strips, assuming a trajectory originating
    /// from the beam spot.
    /// See CoreStripSpacePointFormationTool for the Acts::StripSpacePointBuilder
    /// based alternative.

    class StripSpacePointFormationTool : public StripSpacePointFormationToolBase {
    public:
        using StripSpacePointFormationToolBase::StripSpacePointFormationToolBase;
        virtual ~StripSpacePointFormationTool() = default;

    protected:

        virtual StatusCode makeStripSpacePoint(std::vector<StripSP>& collection,
                                               const StripInformationHelper& firstInfo,
                                               const StripInformationHelper& secondInfo,
                                               const Amg::Vector3D& beamSpotVertex,
                                               bool isEndcap,
                                               double limit,
                                               double slimit) const override;

    private:

        Gaudi::Property< bool > m_useTopSp{this, "useTopSp", false, "SP global position is for second strip module."};
        /// For applying geometric cuts on SPs using beamspot constraint
        Gaudi::Property< bool > m_useBeamSpotConstraint{this, "useBeamSpotConstraint", true, "Reject space points which are not compatible with a particle originating from the beamspot"};

  };

}

#endif
