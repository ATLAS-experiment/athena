/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "GeoPrimitives/GeoPrimitives.h"

template< typename T >
std::ostream& operator<< ( std::ostream& out,
			   const std::vector<T>& vec ) {
  out << "[";
  for( size_t i = 0; i < vec.size(); ++i ) {
    out << vec[ i ];
    if( i < vec.size() - 1 ) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

template< typename T, std::size_t N >
std::ostream& operator<< ( std::ostream& out,
                           const Eigen::Matrix<T,N,1>& arr) {
  std::vector<T> vec;
  vec.reserve(N);

  for( size_t i = 0; i < N; ++i ) {
    vec.push_back(arr(i,0));
  }

  out << vec;
  return out;
}

void fill(xAOD::SpacePoint& pixel, xAOD::SpacePoint& strip,
	  const xAOD::PixelClusterContainer& pixelClusters,
	  const xAOD::StripClusterContainer& stripClusters) {

  const xAOD::PixelCluster *pclus = pixelClusters.at(0);
  const xAOD::StripCluster *sclus0 = stripClusters.at(0);
  const xAOD::StripCluster *sclus1 = stripClusters.at(1);
  
  IdentifierHash::value_type idHashVal(123485);
  IdentifierHash idHash(idHashVal);

  Eigen::Matrix<float,3,1> globalPosition(1, 0, 32);
  Eigen::Matrix<float,2,1> globalVariance(0.21, 0.34);
  std::vector< const xAOD::UncalibratedMeasurement* > pixel_meas({pclus});

  pixel.setSpacePoint( idHash,
		       globalPosition,
		       globalVariance(0,0),
		       globalVariance(1,0),
		       std::move(pixel_meas));

  std::vector< const xAOD::UncalibratedMeasurement* > strip_meas({sclus0, sclus1});
  
  float topHalfStripLength = 1.20;
  float bottomHalfStripLength = 0.32;
  Eigen::Matrix<float,3,1> topStripDirection(2.3, 55.1, 0.3355);
  Eigen::Matrix<float,3,1> bottomStripDirection(1., 2., 5.);
  Eigen::Matrix<float,3,1> stripCenterDistance(0.012, 3.5, 2.11);
  Eigen::Matrix<float,3,1> topStripCenter(0., 0., 1.02);

  IdentifierHash::value_type idHashVals2(123486);
  IdentifierHash idHashs2(idHashVals2);

  strip.setSpacePoint( {idHash, idHashs2},
		       globalPosition,
                       globalVariance(0,0),
                       globalVariance(1,0),
		       std::move(strip_meas),
		       topHalfStripLength,
		       bottomHalfStripLength,
		       topStripDirection,
		       bottomStripDirection,
		       stripCenterDistance,
		       topStripCenter );
}

void print(xAOD::SpacePoint& pixel, xAOD::SpacePoint& strip) {
  std::cout << " --------- PIXEL SPACE POINT  ------------ " << std::endl;
  std::cout << "Identifier Hash = " << pixel.elementIdList()[0] << std::endl;
  std::cout << "Global Position = " << pixel.globalPosition() << std::endl;
  std::cout << "Global x = " << pixel.x() << std::endl; 
  std::cout << "Global y = " << pixel.y() << std::endl; 
  std::cout << "Global z = " << pixel.z() << std::endl; 
  std::cout << "Global Radius = " << pixel.radius() << std::endl;
  std::cout << "varianceR = " << pixel.varianceR() << std::endl;
  std::cout << "varianceZ = " << pixel.varianceZ() << std::endl;
  std::cout << "measurementIndices = " << pixel.measurements().at(0)->index() << std::endl;

  std::cout << " --------- STRIP SPACE POINT  ------------ " << std::endl;
  std::cout << "Identifier Hash = " << strip.elementIdList()[0] << " " << strip.elementIdList()[1] << std::endl;
  std::cout << "Global Position = " << strip.globalPosition() << std::endl;
  std::cout << "Global x = " << strip.x() << std::endl;
  std::cout << "Global y = " << strip.y() << std::endl;
  std::cout << "Global z = " << strip.z() << std::endl;
  std::cout << "Global Radius = " << strip.radius() << std::endl;
  std::cout << "varianceR = " << strip.varianceR() << std::endl;
  std::cout << "varianceZ = " << strip.varianceZ() << std::endl;
  std::cout << "measurementIndices = " << strip.measurements().at(0)->index() << ", " << strip.measurements().at(1)->index() <<std::endl;
  std::cout << "topHalfStripLength = " << strip.topHalfStripLength() << std::endl;
  std::cout << "bottomHalfStripLength = " << strip.bottomHalfStripLength() << std::endl;
  std::cout << "topStripDirection = " << strip.topStripDirection() << std::endl;
  std::cout << "bottomStripDirection = " << strip.bottomStripDirection() << std::endl;
  std::cout << "stripCenterDistance = " << strip.stripCenterDistance() << std::endl;
  std::cout << "topStripCenter = " << strip.topStripCenter() << std::endl;
}

int main() {
  // Make dummy cluster containers, sps need element links to these clusters
  xAOD::PixelClusterContainer tpc;
  xAOD::PixelClusterAuxContainer tpc_aux;
  tpc.setStore(&tpc_aux);
  // add one pixel cluster to the container
  tpc.push_back(new xAOD::PixelCluster());

  xAOD::StripClusterContainer spc;
  xAOD::StripClusterAuxContainer spc_aux;
  spc.setStore(&spc_aux);
  // add two strip clusters to the container
  spc.push_back(new xAOD::StripCluster());
  spc.push_back(new xAOD::StripCluster());
  
  // Create the main containers to test:
  // Pixel Space Point
  xAOD::SpacePointContainer pixel_tpc;
  xAOD::SpacePointAuxContainer pixel_aux;
  pixel_tpc.setStore(&pixel_aux);

  // Strip Space Point
  xAOD::SpacePointContainer strip_tpc;
  xAOD::SpacePointAuxContainer strip_aux;
  strip_tpc.setStore(&strip_aux);

  // Add one Space Point to the containers
  // Pixel
  xAOD::SpacePoint *pixel = new xAOD::SpacePoint();
  pixel_tpc.push_back(pixel);

  // Strip
  xAOD::SpacePoint *strip = new xAOD::SpacePoint();
  strip_tpc.push_back(strip);

  // Fill information
  fill(*pixel, *strip,
       tpc, spc);

  // Print information
  print(*pixel, *strip);
}
