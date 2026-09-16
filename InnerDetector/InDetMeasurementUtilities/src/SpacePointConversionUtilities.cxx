/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMeasurementUtilities/SpacePointConversionUtilities.h"

#include "InDetPrepRawData/SiCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

namespace TrackingUtilities {

  StatusCode convertTrkToXaodPixelSpacePoint(const InDet::PixelSpacePoint& trkSpacePoint,
                                             xAOD::SpacePoint& xaodSpacePoint)
  {
    unsigned int idHash = trkSpacePoint.elementIdList().first;
    const auto& globPos = trkSpacePoint.globalPosition();
    
    const InDet::SiCluster* c  = static_cast<const InDet::SiCluster*>(trkSpacePoint.clusterList().first);
    const InDetDD::SiDetectorElement *de = c->detectorElement();
    const Amg::Transform3D &Tp = de->surface().transform();
    
    float r_3 = static_cast<float>( Tp(0,2) );
    float r_4 = static_cast<float>( Tp(1,2) );
    float r_5 = static_cast<float>( Tp(2,2) );
    
    const Amg::MatrixX& v = c->localCovariance();
    float f22 = static_cast<float>( v(1,1) );
    float wid = static_cast<float>( c->width().z() );
    float cov = wid*wid*.08333;
    if(cov < f22) cov = f22;
    float covr = 6 * cov * (r_5*r_5);
    float covz = 6 * cov * (r_3*r_3 + r_4*r_4);
    
    xaodSpacePoint.setSpacePoint(idHash,
				 globPos.cast<float>(),
				 covr,
				 covz,
				 {});

    return StatusCode::SUCCESS;
  }

  StatusCode convertTrkToXaodStripSpacePoint(const InDet::SCT_SpacePoint& trkSpacePoint,
					     xAOD::SpacePoint& xaodSpacePoint) 
  {
    std::pair<unsigned int, unsigned int> idHashes = trkSpacePoint.elementIdList();
    const auto& globPos = trkSpacePoint.globalPosition();
    
    const InDet::SiCluster *c0 = static_cast<const InDet::SiCluster *>(trkSpacePoint.clusterList().first);
    const InDet::SiCluster *c1 = static_cast<const InDet::SiCluster *>(trkSpacePoint.clusterList().second);
    const InDetDD::SiDetectorElement *d0 = c0->detectorElement();
    const InDetDD::SiDetectorElement *d1 = c1->detectorElement();
    
    Amg::Vector2D lc0 = c0->localPosition();
    Amg::Vector2D lc1 = c1->localPosition();
    
    std::pair<Amg::Vector3D, Amg::Vector3D> e0 =
      (d0->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(), lc0.x(), 0.)));
    std::pair<Amg::Vector3D, Amg::Vector3D> e1 =
      (d1->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(), lc1.x(), 0.)));
    
    auto stripCenter_1 = 0.5 * (e0.first + e0.second);
    auto stripDir_1 = e0.first - e0.second;
    
    auto stripCenter_2 = 0.5 * (e1.first + e1.second);
    auto stripDir_2 = e1.first - e1.second;
    
    float topHalfStripLength = 0.5 * stripDir_1.norm();
    Eigen::Matrix<double, 3, 1> topStripDirection = - stripDir_1 / (2. * topHalfStripLength);
    Eigen::Matrix<double, 3, 1> topStripCenter = stripCenter_1;
    float bottomHalfStripLength = 0.5 * stripDir_2.norm();
    Eigen::Matrix<double, 3, 1> bottomStripDirection = - stripDir_2 / (2. * bottomHalfStripLength);
    Eigen::Matrix<double, 3, 1> stripCenterDistance = stripCenter_1 - stripCenter_2;
    
    const Amg::MatrixX& v = trkSpacePoint.localCovariance();
    float f22 = static_cast<float>( v(1,1) );
    
    float covr = d0->isBarrel() ? .1 : 8.*f22;
    float covz = d0->isBarrel() ? 8.*f22 : .1;
    
    xaodSpacePoint.setSpacePoint({idHashes.first, idHashes.second},
				 globPos.cast<float>(),
				 covr,
				 covz,
				 {},
				 topHalfStripLength,
				 bottomHalfStripLength,
				 topStripDirection.cast<float>(),
				 bottomStripDirection.cast<float>(),
				 stripCenterDistance.cast<float>(),
				 topStripCenter.cast<float>());
    
    return StatusCode::SUCCESS;
  }
  
}
