/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXELSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_PIXELSPACEPOINTFORMATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IPixelSpacePointFormationTool.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

#include <string>

class PixelID;

namespace ActsTrk {
  /// @class PixelSpacePointFormationTool
  /// Tool to produce pixel space points.
  /// Pixel space points are obtained directly from the clusters,
  /// with needed evaluation of the space point covariance terms
  /// Space points are then recorded to storegate as ActsTrk::SpacePoint
  /// into an ActsTrk::SpacePointContainer in the PixelSpacePointFormationAlgorithm

  class PixelSpacePointFormationTool: public extends<AthAlgTool, ActsTrk::IPixelSpacePointFormationTool> {
  public:
    /// @name AthAlgTool methods
    //@{
    PixelSpacePointFormationTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent);
    virtual ~PixelSpacePointFormationTool() = default;
    virtual StatusCode initialize() override;
    //@}

    /// @name Production of space points
    //@{
    virtual StatusCode producePixelSpacePoint(const xAOD::PixelCluster& cluster,
					      xAOD::SpacePoint& sp,
					      const InDetDD::SiDetectorElement& element) const override;
    //@}

  private:
    /// @name Id helpers
    //@{
    const PixelID* m_pixelId{};
    //@}

    /// Whether to use maximum variance for space point covariance
    /// If true, the covariance terms will be capped at the values specified by MaxVarianceZ and MaxVarianceR.
    /// If false, the covariance terms will be calculated based on the cluster width and the rotation of the detector element.
    Gaudi::Property<bool> m_useMaxVariance{this, "UseMaxVariance", false};
    /// Maximum variance for the z component of the space point covariance.
    /// Default value was around the minimum observed value in the pixel barrel.
    Gaudi::Property<float> m_maxVarianceZ{this, "MaxVarianceZ", 0.0014f};
    /// Maximum variance for the r component of the space point covariance.
    /// Default value was around the minimum observed value in the pixel barrel.
    Gaudi::Property<float> m_maxVarianceR{this, "MaxVarianceR", 0.015f};

    /// @name Static constant expression
    /// @brief Values used in calculating covariance terms
    static constexpr double s_oneOverTwelve{0.08333};

  };

}

#endif // ACTSTRKSPACEPOINTFORMATION_PIXELSPACEPOINTFORMATIONALG_H
