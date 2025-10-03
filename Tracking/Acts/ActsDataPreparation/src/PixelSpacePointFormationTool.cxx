/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkSurfaces/Surface.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "PixelSpacePointFormationTool.h"

namespace ActsTrk {

    PixelSpacePointFormationTool::PixelSpacePointFormationTool(const std::string& type,
                                                               const std::string& name,
                                                               const IInterface* parent)
    : base_class(type, name, parent)
    {}

    StatusCode PixelSpacePointFormationTool::initialize() {

        ATH_CHECK(detStore()->retrieve(m_pixelId,"PixelID"));

        return StatusCode::SUCCESS;
    }

    StatusCode
    PixelSpacePointFormationTool::producePixelSpacePoint(const xAOD::PixelCluster& cluster,
                                                         xAOD::SpacePoint& sp,
                                                         const InDetDD::SiDetectorElement& element) const
    {
      // this is the width expressed in mm
      float width = cluster.widthInEta();

      // using width to scale the cluster covariance for space points
      float covTerm = width*width*s_oneOverTwelve;
      auto localCov = cluster.localCovariance<2>();
      if( covTerm < localCov(1, 1) )
          covTerm = localCov(1, 1);

      // use xz, yz, zz terms of rotation matrix to scale the covariance term
      const Amg::Transform3D &Tp = element.surface().transform();
      float cov_z = 6.f*covTerm*static_cast<float>(Tp(0, 2)*Tp(0, 2)+Tp(1, 2)*Tp(1, 2));
      float cov_r = 6.f*covTerm*static_cast<float>(Tp(2, 2)*Tp(2, 2));

      if (m_useMaxVariance) {
        cov_z = std::min(cov_z, m_maxVarianceZ.value());
        cov_r = std::min(cov_r, m_maxVarianceR.value());
      }

	sp.setSpacePoint(cluster.identifierHash(),
			 cluster.globalPosition(),
			 cov_r, 
			 cov_z,
			 std::vector< const xAOD::UncalibratedMeasurement* >({&cluster}));
	
        return StatusCode::SUCCESS;
    }
}
