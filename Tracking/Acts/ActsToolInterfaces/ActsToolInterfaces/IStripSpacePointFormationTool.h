/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H

// Athena
#include "GaudiKernel/IAlgTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"

namespace ActsTrk {
  struct StripSP {
    StripSP() = default;

    std::vector<unsigned int> idHashes {};
    Eigen::Matrix<float,3,1> globPos {0, 0, 0};
    float cov_r {0};
    float cov_z {0};
    std::array<std::size_t,2> measurementIndexes {};
    float topHalfStripLength {0};
    float bottomHalfStripLength {0};
    Eigen::Matrix<float,3,1> topStripDirection {0, 0, 0};
    Eigen::Matrix<float,3,1> bottomStripDirection {0, 0, 0};
    Eigen::Matrix<float,3,1> stripCenterDistance {0, 0, 0};
    Eigen::Matrix<float,3,1> topStripCenter {0, 0, 0};
  };

    /// @class IPixelSpacePointFormationTool
    /// Base class for strip space point formation tool

    class IStripSpacePointFormationTool : virtual public IAlgTool {
    public:
      DeclareInterfaceID(IStripSpacePointFormationTool, 1, 0);


      virtual StatusCode produceSpacePoints(const EventContext& ctx,
					    const xAOD::StripClusterContainer& clusterContainer,
					    const InDet::SiElementPropertiesTable& properties,
					    const InDetDD::SiDetectorElementCollection& elements,
					    const Amg::Vector3D& beamSpotVertex,
					    std::vector<StripSP>& spacePoints,
					    std::vector<StripSP>& overlapSpacePoints,
					    bool processOverlaps,
					    const std::vector<IdentifierHash>& hashesToProcess,
					    const ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>& stripAccessor) const = 0;

    };

} // ACTSTOOLINTERFACES_ISTRIPSPACEPOINTFORMATIONTOOL_H

#endif
