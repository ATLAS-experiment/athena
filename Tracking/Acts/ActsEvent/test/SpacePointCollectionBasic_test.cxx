/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/SpacePointCollector.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h" 

void fill(xAOD::SpacePoint& spacePoint)
{  
  IdentifierHash::value_type idHashVal(123485);
  IdentifierHash idHash(idHashVal);

  Eigen::Matrix<float, 3, 1> globalPosition(10, 10, 10);
  Eigen::Matrix<float,2,1> globalCovariance(1.2, 0.35);

  float topHalfStripLength = 1.2;
  float bottomHalfStripLength = 1.5;

  Eigen::Matrix<float, 3, 1> topStripDirection(0,0,0);
  Eigen::Matrix<float, 3, 1> bottomStripDirection(0,0,0);
  Eigen::Matrix<float, 3, 1> stripCenterDistance(0,0,0);
  Eigen::Matrix<float, 3, 1> topStripCenter(0,0,0);

  std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer>> els;

  spacePoint.setSpacePoint({idHash},
			   globalPosition,
			   globalCovariance(0,0),
			   globalCovariance(1,0),
			   els,
			   topHalfStripLength,
			   bottomHalfStripLength,
			   topStripDirection,
			   bottomStripDirection,
			   stripCenterDistance,
			   topStripCenter);
}

int main() {
  // Make Collections
  std::unique_ptr<xAOD::SpacePointContainer> spacePoints =
    std::make_unique<xAOD::SpacePointContainer>();
  std::unique_ptr<xAOD::SpacePointAuxContainer> spacePointsAux =
    std::make_unique<xAOD::SpacePointAuxContainer>();
  spacePoints->setStore(spacePointsAux.get());

  // Fill Collections
  const std::size_t N = 10;
  for (std::size_t i(0); i<N; i++) {
    spacePoints->push_back(new xAOD::SpacePoint());
    fill(*spacePoints->back());
  }

  std::vector<const xAOD::SpacePoint*> sps;
  sps.reserve(spacePoints->size());
  for (const xAOD::SpacePoint* sp : *spacePoints.get())
    sps.push_back(sp);

  // Set The Collector for the types
  Acts::SpacePointContainerConfig spConfig;
  spConfig.useDetailedDoubleMeasurementInfo = false; /// PPP                                                                                                                                                                        
  // Options                                                                                                                                                                                                                        
  Acts::SpacePointContainerOptions spOptions;
  spOptions.beamPos = Acts::Vector2(0., 0.);

  ActsTrk::SpacePointCollector spacePointCollector(sps);
  Acts::SpacePointContainer<decltype(spacePointCollector), Acts::detail::RefHolder> spacePointWrapper(spConfig, spOptions, spacePointCollector);

  // Dump
  std::cout << "------ SPACE POINTS ------" << std::endl;
  for (const auto& spacePoint : spacePointWrapper) {
    std::cout << "* {" << spacePoint.x() << ", " << spacePoint.y() << ", " << spacePoint.z() << "} "
              << "[" << spacePoint.radius() << "] "
    	      << "{" << spacePoint.varianceR() << ", " << spacePoint.varianceZ() << "}"
              << std::endl;
  }

}
