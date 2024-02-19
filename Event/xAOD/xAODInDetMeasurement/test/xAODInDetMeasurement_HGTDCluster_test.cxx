/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "xAODInDetMeasurement/Utilities.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "xAODMeasurementBase/MeasurementDefs.h"

template< typename T >
std::ostream& operator<< ( std::ostream& out,
                           const std::vector< T >& vec ) {

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

/// Function fill one HGTD cluster with information

void fill( xAOD::HGTDCluster& HGTDCluster) {

    constexpr xAOD::DetectorIDHashType idHash(156237);

    constexpr xAOD::DetectorIdentType id(96851257);

    Eigen::Matrix<float,3,1> localPosition(0.1, 0.5, 3.2);

    Eigen::Matrix<float,3,3> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = 0.012;
    localCovariance(1, 1) = 0.012;
    localCovariance(2,2)  = 0.005;

    HGTDCluster.setMeasurement<3>(idHash, localPosition, localCovariance);
    HGTDCluster.setIdentifier(id);

    std::vector < Identifier > rdoList = { Identifier(Identifier::value_type(0x200921680c00000)),
                                           Identifier(Identifier::value_type(0x298094737200000)),
                                           Identifier(Identifier::value_type(0x24e105292800000)),
                                           Identifier(Identifier::value_type(0x298094737200000)),
                                           Identifier(Identifier::value_type(0xaa1cdd0d200000)),
                                           Identifier(Identifier::value_type(0x200921680c00000)) };

    HGTDCluster.setRDOlist(rdoList);

    std::vector < int > tots = {1, 2, 3, 4, 5, 6};

    HGTDCluster.setToTlist(tots);
    return;
}

void print ( const xAOD::HGTDCluster& HGTDCluster) {
    std::cout << " --------- MEASUREMENT BASE ------------ " << std::endl;
    std::cout << "Identifier Hash = " << HGTDCluster.identifierHash() << std::endl;
    std::cout << "Identifier = " << HGTDCluster.identifier() << std::endl;
    std::cout << "Local Position = " << HGTDCluster.localPosition<3>() << std::endl;
    std::cout << "Local Covariance = " << HGTDCluster.localCovariance<3>() << std::endl;
    std::cout << " --------- HGTD CLUSTER INFO ----------- " << std::endl;
    std::cout << "RDOs = " << HGTDCluster.rdoList() << std::endl;
    std::cout << "ToTs = " << HGTDCluster.totList() << std::endl;
    return;
}

int main() {

    // create the main containers to test:
    xAOD::HGTDClusterAuxContainer aux;
    xAOD::HGTDClusterContainer tpc;

    tpc.setStore(&aux);

    // add one HGTD cluster to the container
    xAOD::HGTDCluster * pix = new xAOD::HGTDCluster();
    tpc.push_back(pix);

    // fill information
    fill(*pix);

    //print information
    print(*pix);

    return 0;
}
