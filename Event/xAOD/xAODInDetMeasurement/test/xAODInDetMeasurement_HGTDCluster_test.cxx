/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "xAODInDetMeasurement/Utilities.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "xAODMeasurementBase/MeasurementDefs.h"

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

namespace {
template <typename T>
auto trans(T &&a) { return a; }

template <>
auto trans(const unsigned long long &a) { return Identifier(a); }

template <typename T>
std::ostream &operator<<(std::ostream &out, CxxUtils::range_with_conv<CxxUtils::span<T> > elements) {
   out << "[";
   for( size_t i = 0; i < elements.size(); ++i ) {
      out << trans(elements[ i ]);
      if( i < elements.size() - 1 ) {
         out << ", ";
      }
   }
   out << "]";
   return out;
}
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
